#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;
using namespace cv;

// ------------------------------
// Sobel kernels
// ------------------------------
vector<float> SobelEdgeKernel_gx()
{
    return {-1, 0, 1,
            -2, 0, 2,
            -1, 0, 1};
}
vector<float> SobelEdgeKernel_gy()
{
    return {-1, -2, -1,
            0, 0, 0,
            1, 2, 1};
}

// ------------------------------
// Filtering (Sobel conv). n=1 => 3x3
// dst는 0~255 범위의 8U (시각화용)
// ------------------------------
Mat Filtering_gray(const Mat &src, const vector<float> &kernel, int n)
{
    CV_Assert(src.type() == CV_8UC1);
    const int kSize = 2 * n + 1;
    CV_Assert((int)kernel.size() == kSize * kSize);

    Mat P;
    copyMakeBorder(src, P, n, n, n, n, BORDER_REPLICATE);

    Mat dst32 = Mat::zeros(src.size(), CV_32F);

    for (int h = n; h < P.rows - n; ++h)
    {
        for (int w = n; w < P.cols - n; ++w)
        {
            float sum = 0.0f;
            int k = 0;
            for (int y = -n; y <= n; ++y)
            {
                for (int x = -n; x <= n; ++x)
                {
                    sum += static_cast<float>(P.at<uchar>(h + y, w + x)) * kernel[k++];
                }
            }
            dst32.at<float>(h - n, w - n) = sum;
        }
    }

    // 절댓값 후 0~255 스케일로 변환
    Mat dstAbs = cv::abs(dst32);
    double minv, maxv;
    minMaxLoc(dstAbs, &minv, &maxv);
    double scale = (maxv > 0) ? 255.0 / maxv : 1.0;

    Mat dst8u;
    dstAbs.convertTo(dst8u, CV_8U, scale);
    return dst8u;
}

// ------------------------------
// |gx|, |gy| -> edge magnitude (0~255, 8U)
//   magnitude = sqrt(gx^2 + gy^2) 를 0~255로 정규화
// ------------------------------
Mat make_edge_magnitude8u(const Mat &gx8u, const Mat &gy8u)
{
    CV_Assert(gx8u.type() == CV_8UC1 && gy8u.type() == CV_8UC1);
    CV_Assert(gx8u.size() == gy8u.size());

    Mat gx32, gy32;
    gx8u.convertTo(gx32, CV_32F);
    gy8u.convertTo(gy32, CV_32F);

    Mat gx2, gy2, sum, mag;
    multiply(gx32, gx32, gx2);
    multiply(gy32, gy32, gy2);
    sum = gx2 + gy2;
    sqrt(sum, mag); // 0 ~ 약 360 범위

    double minv, maxv;
    minMaxLoc(mag, &minv, &maxv);
    double scale = (maxv > 0) ? 255.0 / maxv : 1.0;

    Mat edge8u;
    mag.convertTo(edge8u, CV_8U, scale);
    return edge8u;
}

// ------------------------------
// thresholding_gray:
//   edge(8U, 0~255)와 임계값을 받아 이진화 결과(8U, 0/255) 반환
// ------------------------------
Mat thresholding_gray(const Mat &edge8u, int thresh)
{
    CV_Assert(edge8u.type() == CV_8UC1);
    Mat bin;
    // thresh 값보다 큰 픽셀을 255로
    threshold(edge8u, bin, thresh, 255, THRESH_BINARY);
    return bin;
}

// ==============================
// 전역(트랙바 콜백용)
// ==============================
static Mat g_src, g_gx, g_gy, g_gxgy, g_edge8u, g_bin;
static int g_thresh = 100;

static void on_trackbar(int, void *)
{
    g_bin = thresholding_gray(g_gxgy, g_thresh);
    imshow("threshold", g_bin);
}

// ==============================
// 가우시안 노이즈 추가
// ==============================
static inline unsigned char clamp8(int v)
{
    return (unsigned char)std::max(0, std::min(255, v));
}

Mat addGaussianNoiseGray(const Mat &src, double mean, double stddev)
{
    Mat dst = src.clone();

    RNG rng((uint64)-1); // OpenCV의 난수 생성기 객체

    for (int h = 0; h < src.rows; ++h)
    {
        const uchar *srow = src.ptr<uchar>(h); // 원본 픽셀 값
        uchar *drow = dst.ptr<uchar>(h);
        for (int w = 0; w < src.cols; ++w)
        {
            // 평균 mean, 표준편차 stddev의 가우시안 샘플
            double n = mean + rng.gaussian(stddev);               // rng.gaussian(stddev) : 평균이 0이고, 표준편차가 stddev인 정규분포 난수 생성 mean + 로 원하는 평균을 갖도록 시프트
            drow[w] = clamp8((int)std::lround((int)srow[w] + n)); // 원본 픽셀값에 가우시안 난수 추가(+n)
        }
    }
    return dst;
}

// ==============================
// 가우시안 필터 추가
// ==============================
vector<double> createGaussianKernel(int ksize, double sigma)
{
    if (ksize % 2 == 0 || ksize <= 0)
        throw runtime_error("ksize must be odd and > 0");

    int half = ksize / 2;
    vector<double> kernel(ksize * ksize);
    double sum = 0.0;

    // 가우시안 값 계산
    for (int i = -half; i <= half; i++)
    {
        for (int j = -half; j <= half; j++)
        {
            double value = exp(-(i * i + j * j) / (2 * sigma * sigma));
            kernel[(i + half) * ksize + (j + half)] = value;
            sum += value;
        }
    }

    // 정규화
    for (int i = 0; i < ksize * ksize; i++)
        kernel[i] /= sum;

    return kernel;
}

// Zero Padding (ksize 맞춰서 패딩)
Mat zeroPadding(const Mat &src, int pad)
{
    Mat paddedMat = Mat::zeros(src.rows + 2 * pad, src.cols + 2 * pad, CV_8UC1);
    for (int h = 0; h < src.rows; ++h)
    {
        for (int w = 0; w < src.cols; ++w)
        {
            paddedMat.at<uchar>(h + pad, w + pad) = src.at<uchar>(h, w);
        }
    }
    return paddedMat;
}

// Gaussian Filter (ksize, sigma 인자로 받음)
Mat Gaussian_Filter(const Mat &src, int ksize, double sigma)
{
    int pad = ksize / 2;
    Mat PaddedSrc = zeroPadding(src, pad);

    Mat output = Mat::zeros(src.rows, src.cols, CV_8UC1);

    const unsigned char *pData = PaddedSrc.data;
    unsigned char *outData = output.data;

    vector<double> kernel = createGaussianKernel(ksize, sigma);

    for (int h = pad; h < PaddedSrc.rows - pad; ++h)
    {
        for (int w = pad; w < PaddedSrc.cols - pad; ++w)
        {
            double Gaussian = 0.0;

            // ksize x ksize convolution
            for (int i = -pad; i <= pad; i++)
            {
                for (int j = -pad; j <= pad; j++)
                {
                    int idx = (i + pad) * ksize + (j + pad);
                    Gaussian += pData[(h + i) * PaddedSrc.cols + (w + j)] * kernel[idx];
                }
            }

            // 값 범위 조정
            Gaussian = std::clamp(Gaussian, 0.0, 255.0);
            outData[(h - pad) * output.cols + (w - pad)] = static_cast<unsigned char>(std::lround(Gaussian));
        }
    }

    return output;
}

int main(void)
{
    g_src = imread("Lena.png", IMREAD_GRAYSCALE);
    if (g_src.empty())
    {
        cerr << "Image Not Found\n";
        return -1;
    }

    // Original, Sobel filtering (시각화용 0~255 스케일)
    /*
    vector<float> gxKernel = SobelEdgeKernel_gx();
    vector<float> gyKernel = SobelEdgeKernel_gy();
    g_gx = Filtering_gray(g_src, gxKernel, 1);
    g_gy = Filtering_gray(g_src, gyKernel, 1);

    g_gxgy = g_gx + g_gy;

    // 에지 크기 영상(0~255) 미리 만들어두기
    g_edge8u = make_edge_magnitude8u(g_gx, g_gy);

    // 초기 thresholding
    g_bin = thresholding_gray(g_gxgy, g_thresh);

    // 창/표시
    imshow("Original", g_src);
    imshow("|gx| display", g_gx);
    imshow("|gy| display", g_gy);
    imshow("|gx| + |gy| display", g_gxgy);
    // imshow("Edge magnitude(0~255)", g_edge8u);
    imshow("threshold", g_bin);

    // 트랙바: 0~255
    namedWindow("threshold", WINDOW_AUTOSIZE);
    createTrackbar("Thresh", "threshold", &g_thresh, 255, on_trackbar);

    // ---- 저장 로직 ----
    // 's' 를 누르면 현재 결과 저장
    // 'q'나 ESC로 종료. 종료 시 마지막 결과도 저장
    cout << "[Hint] Press 's' to save current results, 'q' or ESC to quit.\n";
    while (true)
    {
        int key = waitKey(30);
        if (key == 's' || key == 'S')
        {
            imwrite("gx.png", g_gx);
            imwrite("gy.png", g_gy);
            // imwrite("edge_mag.png", g_edge8u);
            imwrite("gxgy.png", g_gxgy);
            imwrite("gxgy_thresh.png", g_bin);
            cout << "Saved: gx.png, gy.png, edge_mag.png, gxgy_thresh.png\n";
        }
        else if (key == 27 || key == 'q' || key == 'Q')
        {
            break;
        }
    }*/

    // 가우시안 노이즈 추가 mean = 0.0, stddev = 20.0
    /*
    Mat GaussianNoise_src = addGaussianNoiseGray(g_src, 0.0, 20.0);

    vector<float> gxKernel = SobelEdgeKernel_gx();
    vector<float> gyKernel = SobelEdgeKernel_gy();
    g_gx = Filtering_gray(GaussianNoise_src, gxKernel, 1);
    g_gy = Filtering_gray(GaussianNoise_src, gyKernel, 1);

    g_gxgy = g_gx + g_gy;

    g_bin = thresholding_gray(g_gxgy, g_thresh);

    imshow("GaussianNoise", GaussianNoise_src);
    imshow("GN, |gx| display", g_gx);
    imshow("GN, |gy| display", g_gy);
    imshow("|gx|+|gy| display", g_gxgy);
    imshow("threshold", g_bin);

    namedWindow("threshold", WINDOW_AUTOSIZE);
    createTrackbar("Thresh", "threshold", &g_thresh, 255, on_trackbar);

    cout << "[Hint] Press 's' to save current results, 'q' or ESC to quit.\n";
    while (true)
    {
        int key = waitKey(30);
        if (key == 's' || key == 'S')
        {
            imwrite("GNgx.png", g_gx);
            imwrite("GNgy.png", g_gy);
            // imwrite("edge_mag.png", g_edge8u);
            imwrite("GNgxgy.png", g_gxgy);
            imwrite("GNgxgy_thresh.png", g_bin);
            cout << "Saved: gx.png, gy.png, gxgy_thresh.png\n";
        }
        else if (key == 27 || key == 'q' || key == 'Q')
        {
            break;
        }
    }*/

    // 가우시안 노이즈에 가우시안 필터 적용 후 edge detection

    Mat GaussianNoise_src = addGaussianNoiseGray(g_src, 0.0, 20.0);

    Mat GF_1 = Gaussian_Filter(GaussianNoise_src, 7, 1.0);
    Mat GF_2 = Gaussian_Filter(GaussianNoise_src, 7, 5.0);

    vector<float> gxKernel = SobelEdgeKernel_gx();
    vector<float> gyKernel = SobelEdgeKernel_gy();
    g_gx = Filtering_gray(GF_1, gxKernel, 1);
    g_gy = Filtering_gray(GF_1, gyKernel, 1);

    g_gxgy = g_gx + g_gy;

    g_bin = thresholding_gray(g_gxgy, g_thresh);

    imshow("GaussianNoise", GF_1);
    imshow("GN, |gx| display", g_gx);
    imshow("GN, |gy| display", g_gy);
    imshow("|gx|+|gy| display", g_gxgy);
    imshow("threshold", g_bin);

    namedWindow("threshold", WINDOW_AUTOSIZE);
    createTrackbar("Thresh", "threshold", &g_thresh, 255, on_trackbar);

    cout << "[Hint] Press 's' to save current results, 'q' or ESC to quit.\n";
    while (true)
    {
        int key = waitKey(30);
        if (key == 's' || key == 'S')
        {
            imwrite("GNgx.png", g_gx);
            imwrite("GNgy.png", g_gy);
            // imwrite("edge_mag.png", g_edge8u);
            imwrite("GNgxgy.png", g_gxgy);
            imwrite("GNgxgy_thresh.png", g_bin);
            cout << "Saved: gx.png, gy.png, gxgy_thresh.png\n";
        }
        else if (key == 27 || key == 'q' || key == 'Q')
        {
            break;
        }
    }

    return 0;
}
