#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include <algorithm>

using namespace std;
using namespace cv;

// =========== 전역 상태 ==========
static Mat screen;             // 사용자가 PDF 그리는 캔버스
static const int hist_w = 512; // 그리기 캔버스 폭(밝기 축)
static const int hist_h = 400; // 그리기 캔버스 높이(빈도 축)
static Point pt1(1, -1);
static bool drawing = false;

// 256-bin 목표 PDF를 만들기 위한 임시 높이(정규화 전) - 벡터 대신 배열
static double g_dstHeight[256];

// 유틸리티: 구간 [a, b] clamp
template <typename T>
inline T clampv(T v, T lo, T hi) { return std::max(lo, std::min(v, hi)); }

// 선분을 256-bin으로 rasterization하여 g_dstHeight 업데이트
static void addSegmentToTargetPDF(const Point &A, const Point &B)
{
    Point p0 = A, p1 = B;
    if (p1.x < p0.x)
    {
        std::swap(p0, p1);
    }

    int dx = (p1.x - p0.x);

    if (dx == 0)
    {
        int bx = clampv(int(std::lround((p0.x * 255.0) / (hist_w - 1))), 0, 255);
        double h = clampv(double(hist_h - p0.y), 0.0, double(hist_h));
        g_dstHeight[bx] = std::max(g_dstHeight[bx], h);
        return;
    }

    for (int x = p0.x; x <= p1.x; ++x)
    {
        double t = double(x - p0.x) / double(dx);
        double y = (1.0 - t) * p0.y + t * p1.y;
        int bx = clampv(int(std::lround((x * 255.0) / (hist_w - 1))), 0, 255);
        double h = clampv(double(hist_h - y), 0.0, double(hist_h));
        g_dstHeight[bx] = std::max(g_dstHeight[bx], h);
    }
}

// 마우스 콜백
static void onMouse(int event, int x, int y, int, void *)
{
    if (event == EVENT_LBUTTONDOWN)
    {
        pt1 = Point(x, y);
        drawing = true;
    }
    else if (event == EVENT_MOUSEMOVE && drawing)
    {
        Mat tmp = screen.clone();
        line(tmp, pt1, Point(x, y), Scalar(0, 0, 255), 2);
        imshow("screen", tmp);
    }
    else if (event == EVENT_LBUTTONUP)
    {
        Point pt2(x, y);
        line(screen, pt1, pt2, Scalar(0, 0, 255), 2);
        imshow("screen", screen);
        drawing = false;
        addSegmentToTargetPDF(pt1, pt2); // 좌표 넘겨주기
    }
}

static void computeHist256(const Mat &gray, int hist[256])
{
    // 초기화
    for (int i = 0; i < 256; ++i)
    {
        hist[i] = 0;
    }

    const int rows = gray.rows;
    const int cols = gray.cols;
    const size_t step = gray.step;
    const unsigned char *sdata = gray.data;

    if (gray.isContinuous())
    {
        const size_t Nbytes = rows * step;
        for (size_t i = 0; i < Nbytes; ++i)
        {
            hist[sdata[i]]++;
        }
    }
    else
    {
        for (int r = 0; r < rows; ++r)
        {
            const unsigned char *rowp = sdata + r * step;
            for (int c = 0; c < cols; ++c)
            {
                hist[rowp[c]]++;
            }
        }
    }
}

// 정규화 PDF 및 CDF 계산
static void pdfAndCdf(const int hist[256], double pdf[256], double cdf[256])
{
    double total = 0.0;
    for (int i = 0; i < 256; ++i)
    {
        total += hist[i];
    }
    if (total <= 0.0)
    {
        total = 1.0;
    }

    for (int i = 0; i < 256; ++i)
    {
        pdf[i] = hist[i] / total;
    }
    cdf[0] = pdf[0];
    for (int i = 1; i < 256; ++i)
    {
        cdf[i] = cdf[i - 1] + pdf[i];
    }
}

// 높이 벡터(0~hist_h) -> 정규화 PDF & CDF
static void targetPdfFromHeight(const double height[256], double pdf[256], double cdf[256])
{
    double sumh = 0.0;
    for (int i = 0; i < 256; ++i)
    {
        sumh += std::max(0.0, height[i]);
    }
    if (sumh <= 0.0)
    {
        for (int i = 0; i < 256; ++i)
        {
            pdf[i] = 1.0 / 256.0; // fallback: uniform
        }
    }
    else
    {
        for (int i = 0; i < 256; ++i)
        {
            pdf[i] = std::max(0.0, height[i]) / sumh;
        }
    }

    cdf[0] = pdf[0];
    for (int i = 1; i < 256; ++i)
    {
        cdf[i] = cdf[i - 1] + pdf[i];
    }
}

static void buildLUTfromCDFs(const double cdfSrc[256], const double cdfDst[256], unsigned char lut[256])
{
    for (int g = 0; g < 256; ++g)
    {
        double s = cdfSrc[g];
        int z = 0;
        while (z < 255 && cdfDst[z] < s)
            ++z;

        lut[g] = static_cast<unsigned char>(z);
    }
}

static Mat applyLUT(const Mat &gray, const unsigned char lut[256])
{
    Mat out(gray.size(), CV_8UC1);
    const int rows = gray.rows;
    const int cols = gray.cols;
    const size_t sstep = gray.step;
    const size_t dstep = out.step;

    const unsigned char *sdata = gray.data;
    unsigned char *ddata = out.data;

    for (int r = 0; r < rows; ++r)
    {
        const unsigned char *srow = sdata + r * sstep;
        unsigned char *drow = ddata + r * dstep;
        for (int c = 0; c < cols; ++c)
        {
            drow[c] = lut[srow[c]];
        }
    }

    return out;
}

int main(void)
{
    // g_dstHeight 초기화
    for (int i = 0; i < 256; ++i)
    {
        g_dstHeight[i] = 0.0;
    }

    // 1) 목표 PDF 그리기 창
    screen = Mat::zeros(hist_h, hist_w, CV_8UC3);
    putText(screen, "Draw target Histogram", Point(10, 20), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(200, 200, 200), 1);

    namedWindow("screen");
    setMouseCallback("screen", onMouse);
    imshow("screen", screen);

    // 2) 원본 영상 로드
    Mat Origin = imread("Lena.png", IMREAD_GRAYSCALE);
    if (Origin.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }

    imshow("Origin", Origin);

    waitKey(0);

    // 3) 원본 히스토그램 / PDF / CDF
    int histSrc[256];
    double pdfSrc[256], cdfSrc[256];
    computeHist256(Origin, histSrc);    // 원본 히스토그램 계산 (256-bin)
    pdfAndCdf(histSrc, pdfSrc, cdfSrc); // 정규화 PDF, CDF 계산

    // 4) 사용자 곡선 -> 목표 PDF/CDF
    double pdfDst[256], cdfDst[256];
    targetPdfFromHeight(g_dstHeight, pdfDst, cdfDst); // 높이 벡터 > 정규화 PDF & CDF

    // 5) CDF 매칭 LUT
    unsigned char lut[256];
    buildLUTfromCDFs(cdfSrc, cdfDst, lut); // CDF 매칭 LUT 생성

    // 6) 적용
    Mat Result = applyLUT(Origin, lut); // LUT 적용, 포인터 순회

    // 7) 결과 및 히스토그램 표시
    int histDst[256];
    computeHist256(Result, histDst);

    imshow("Result", Result);
    // imshow("Hist_Origin", drawHistBars(histSrc)); // 원본 영상 히스토그램 출력
    // imshow("Hist_Result", drawHistBars(histDst)); // Histogram Matching 영상 히스토그램 출력

    // 사용자 입력 대기
    waitKey(0);
    return 0;
}
