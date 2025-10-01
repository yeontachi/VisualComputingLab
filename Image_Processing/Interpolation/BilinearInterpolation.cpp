#include <math.h>
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

template <typename T>
inline T clamp(T v, T lo, T hi)
{ // Clamp 함수 범위 안에서 값을 잘라내기 위해 정의
    if (v < lo)
        return lo;
    if (v > hi)
        return hi;
    return v;
}

static inline unsigned char Bilinear_Interpolation(const Mat &src, int w, int h, int outSize)
{
    const int W = src.cols, H = src.rows; // 원본 이미지의 전체 width, Height

    const double rx = (outSize > 1) ? double(W - 1) / (outSize - 1) : 0.0; // 1은 입력하지 않겠지만, Outsize가 1일 때 예외 처리를 위한 삼항연산
    const double ry = (outSize > 1) ? double(H - 1) / (outSize - 1) : 0.0;

    double sx = w * rx; // 원본 사이즈 기준 추가된 픽셀의 비율적 실제 위치
    double sy = h * ry;

    sx = clamp(sx, 0.0, double(W - 1)); // 경계 확인(안전장치)
    sy = clamp(sy, 0.0, double(H - 1));

    int x0 = int(floor(sx)), y0 = int(floor(sy));
    int x1 = min(x0 + 1, W - 1), y1 = min(y0 + 1, H - 1);

    double a = sx - x0, b = sy - y0; // 1기준 비율 길이 계산 실제 픽셀 위치 - 원본 픽셀 위치

    const uchar *r0 = src.ptr<uchar>(y0);
    const uchar *r1 = src.ptr<uchar>(y1);

    double p1 = r0[x0], p2 = r0[x1]; // 주변 4개 점의 픽셀 값들
    double p3 = r1[x0], p4 = r1[x1];

    double q1 = (1.0 - a) * p1 + a * p2; // Bilinear Interpolation 수식
    double q2 = (1.0 - a) * p3 + a * p4;
    double q = (1.0 - b) * q1 + b * q2;

    int v = (int)std::lround(q);
    return (uchar)clamp(v, 0, 255);
}

int main()
{
    Mat img_in;

    // image 읽고 gray로 바꾸기
    img_in = imread("Lena_256x256.png");

    if (img_in.empty())
    {
        cerr << "Error: No Image Found" << "\n";
        return -1;
    }

    cvtColor(img_in, img_in, cv::COLOR_RGB2GRAY);
    imshow("source img", img_in);

    double scale = 0;

    cout << "Scale: ";
    cin >> scale;

    if (scale < 0)
    {
        cerr << "Invalid scale\n";
        return -1;
    }

    int size = img_in.cols * scale;

    Mat img_out(size, size, CV_8UC1);
    if (!img_out.isContinuous())
        img_out = img_out.clone();

    unsigned char *pData;
    pData = (unsigned char *)img_out.data;

    for (int h = 0; h < size; h++)
    {
        for (int w = 0; w < size; w++)
        {
            pData[h * size + w] = Bilinear_Interpolation(img_in, w, h, size);
        }
    }

    imshow("Bilinear Interpolation Image", img_out);

    waitKey(0);

    return 0;
}