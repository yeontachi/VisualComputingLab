#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

using namespace std;
using namespace cv;

static double PSNR(const Mat &src, const Mat &dst)
{
    // MSE 계산
    int sum = 0; // 합

    for (int h = 0; h < src.rows; ++h)
    {
        for (int w = 0; w < src.cols; ++w)
        {
            int p = src.at<uchar>(h, w) - dst.at<uchar>(h, w);

            sum += (p * p);
        }
    }

    double MSE = sum / (src.rows * src.cols);

    // PSNR

    double psnr = 10 * log(255 * 255 / MSE);

    return psnr;
}

int main(void)
{
    Mat src = imread("Lena.png", IMREAD_GRAYSCALE); // 원본 이미지(Noise 없음)

    if (src.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }

    Mat NoiseModel = imread("Lena.png", IMREAD_GRAYSCALE); // Noise Model

    if (NoiseModel.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }

    cout << "PSNR: " << PSNR(src, NoiseModel) << "\n";

    return 0;
}