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
    Mat src = imread("Lena.png", IMREAD_GRAYSCALE);         // 원본
    Mat noise = imread("Lena_Noise.png", IMREAD_GRAYSCALE); // 노이즈 영상

    if (src.empty() || noise.empty())
    {
        cerr << "Image Not Found\n";
        return -1;
    }

    double psnr = PSNR(src, noise);
    cout << "PSNR: " << psnr << " dB\n";

    return 0;
}
