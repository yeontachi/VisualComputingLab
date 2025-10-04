#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

using namespace std;
using namespace cv;

static double PSNR(const Mat &src, const Mat &dst)
{
    // 입력 영상 크기 동일성 확인
    if (src.size() != dst.size() || src.type() != dst.type())
        throw runtime_error("Input images must have same size and type");

    double sum = 0.0; // 오버플로우 방지

    for (int h = 0; h < src.rows; ++h)
    {
        const uchar *pSrc = src.ptr<uchar>(h);
        const uchar *pDst = dst.ptr<uchar>(h);
        for (int w = 0; w < src.cols; ++w)
        {
            double diff = (double)pSrc[w] - (double)pDst[w];
            sum += diff * diff;
        }
    }

    double MSE = sum / (double)(src.rows * src.cols);

    if (MSE == 0)
        return INFINITY; // 완전히 동일할 경우 PSNR = 무한대

    double psnr = 10.0 * log10((255.0 * 255.0) / MSE);
    return psnr;
}

int main(void)
{
    Mat src = imread("Lena.png", IMREAD_GRAYSCALE);
    Mat GN05 = imread("SPN005_Median.png", IMREAD_GRAYSCALE);
    Mat GN10 = imread("SPN010_Median.png", IMREAD_GRAYSCALE);
    Mat GN30 = imread("SPN025_Median.png", IMREAD_GRAYSCALE);
    Mat GN055 = imread("SPN005_AdaptiveMedian.png", IMREAD_GRAYSCALE);
    Mat GN105 = imread("SPN010_AdaptiveMedian.png", IMREAD_GRAYSCALE);
    Mat GN305 = imread("SPN025_AdaptiveMedian.png", IMREAD_GRAYSCALE);

    double psnr1 = PSNR(src, GN05);
    double psnr2 = PSNR(src, GN10);
    double psnr3 = PSNR(src, GN30);
    double psnr4 = PSNR(src, GN055);
    double psnr5 = PSNR(src, GN105);
    double psnr6 = PSNR(src, GN305);

    cout << "PSNR(median, 005): " << psnr1 << " dB\n";
    cout << "PSNR(median, 010): " << psnr2 << " dB\n";
    cout << "PSNR(median, 025): " << psnr3 << " dB\n";
    cout << "PSNR(AdaptiveMedian, 005): " << psnr4 << " dB\n";
    cout << "PSNR(AdaptiveMedian, 010): " << psnr5 << " dB\n";
    cout << "PSNR(AdaptiveMedian, 025): " << psnr6 << " dB\n";

    return 0;
}
