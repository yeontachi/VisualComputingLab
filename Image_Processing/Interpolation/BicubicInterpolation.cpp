#include <opencv2\opencv.hpp>
#include <math.h>
#include <iostream>

using namespace std;
using namespace cv;

template <typename T>
inline T clamp(T v, T lo, T hi)
{ // clamp 함수 범위 안에서 값을 잘라내기 위해 정의
    if (v < lo)
        return lo;
    if (v > hi)
        return hi;
    return v;
}

// Bicubic, Cubic Convolution Weight, parameter a = -0.5(보통 이걸로한다함)
static inline double cubicWeight(double x, double a = -0.5)
{
    x = abs(x);
    if (x < 1.0)
        return (a + 2.0) * x * x * x - (a + 3.0) * x * x + 1;
    else if (x >= 1.0 && x < 2.0)
        return a * x * x * x - 5.0 * a * x * x + 8.0 * a * x - 4 * a;
    else if (x >= 2.0)
        return 0.0;
}

// Bicubic Interpolation 함수 정의
static inline unsigned char Bicubic_Interpolation(const Mat &src, int w, int h, int outSize)
{
    const int W = src.cols, H = src.rows; // 원본 이미지의 Width, Hegith

    const double rx = (outSize > 1) ? double(W - 1) / (outSize - 1) : 0.0;
    const double ry = (outSize > 1) ? double(H - 1) / (outSize - 1) : 0.0;

    const double sx = w * rx;
    const double sy = h * ry;

    const int x0 = (int)std::floor(sx);
    const int y0 = (int)std::floor(sy);
    const double dx = sx - x0;
    const double dy = sy - y0;

    // 4개x, 4개 y에 대한 가중치 계산
    double wx[4], wy[4];
    wx[0] = cubicWeight(1.0 + dx); // x0 - 1
    wx[1] = cubicWeight(dx);       // x0
    wx[2] = cubicWeight(1.0 - dx); // x0 + 1
    wx[3] = cubicWeight(2.0 - dx); // x0 + 2

    wy[0] = cubicWeight(1.0 + dy); // y0 - 1
    wy[1] = cubicWeight(dy);       // y0
    wy[2] = cubicWeight(1.0 - dy); // y0 + 1
    wy[3] = cubicWeight(2.0 - dy); // y0 + 2

    //  4x4 주변 픽셀 누적 경계는 clamp
    double sum = 0.0;
    double wsum = 0.0;

    for (int j = -1; j <= 2; ++j)
    {
        int yy = clamp(y0 + j, 0, H - 1);
        const uchar *row = src.ptr<uchar>(yy);
        double wyj = wy[j + 1];
        for (int i = -1; i <= 2; ++i)
        {
            int xx = clamp(x0 + i, 0, W - 1);
            double wij = wyj * wx[i + 1]; // w(i) x w(j)
            sum += wij * row[xx];
            wsum += wij;
        }
    }

    double val = (wsum != 0.0) ? (sum / wsum) : 0.0;

    val = std::round(val);
    if (val < 0.0)
        val = 0.0;
    else if (val > 255.0)
        val = 255.0;
    return static_cast<unsigned char>(val);
}

int main(void)
{
    Mat img_in = imread("Lenna.png", IMREAD_GRAYSCALE);

    if (img_in.empty())
    {
        cerr << "Error: No Image Found\n";
        return -1;
    }

    imshow("Source img", img_in);

    // 사이즈 또는 스케일 입력받기
    int size = 0; // 정사각형 가정
    cout << "Size: " << "\n";
    cin >> size;

    if (size <= 0)
    {
        cerr << "Invalid size\n";
        return -1;
    }

    // Interpolation 이미지 정의
    Mat img_out(size, size, CV_8UC1);
    if (!img_out.isContinuous())
        img_out = img_out.clone();

    // Bicubic Interpolation 적용
    unsigned char *pData;
    pData = (unsigned char *)img_out.data;

    for (int h = 0; h < size; ++h)
    {
        for (int w = 0; w < size; ++w)
        {
            pData[h * size + w] = Bicubic_Interpolation(img_in, w, h, size);
        }
    }

    imshow("Bicubic Interpolation Image", img_out);

    waitKey(0);

    return 0;
}