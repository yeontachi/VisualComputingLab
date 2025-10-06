#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>

using namespace std;
using namespace cv;

// 가우시안 커널 생성 (ksize x ksize, 홀수 크기)
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
    Mat src = imread("Lena.png", IMREAD_GRAYSCALE);

    if (src.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }

    Mat dst3_01 = Gaussian_Filter(src, 7, 0.1);
    Mat dst3_1 = Gaussian_Filter(src, 7, 1.0);
    Mat dst3_5 = Gaussian_Filter(src, 7, 5.0);

    imshow("Original", src);
    imshow("Gaussian Filter 7x7 sigma=0.1", dst3_01);
    imshow("Gaussian Filter 7x7 sigma=1.0", dst3_1);
    imshow("Gaussian Filter 7x7 sigma=5.0", dst3_5);

    waitKey(0);
    return 0;
}
