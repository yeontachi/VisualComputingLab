#include <opencv2/opencv.hpp>
#include <iostream>
#include <algorithm>
#include <cmath>

using namespace std;
using namespace cv;

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

int main(void)
{
    Mat src = imread("Lena.png", IMREAD_GRAYSCALE);

    if (src.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }

    double mean = 0.0, stddev = 20.0;
    cout << "Gaussian mean: ";
    cin >> mean;
    cout << "Gaussian stddev: ";
    cin >> stddev;

    Mat GN_img = addGaussianNoiseGray(src, mean, stddev);

    imshow("Original", src);
    imshow("Gaussian Noise", GN_img);

    waitKey(0);

    return 0;
}