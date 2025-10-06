#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int wx[8] = {-1, 0, 1, 1, 1, 0, -1, -1}; // 좌상단부터 시게방향
int wy[8] = {-1, -1, -1, 0, 1, 1, 1, 0};

// Zero Padding with Variable border
static Mat zeroPadding(const Mat &src, int pad)
{
    Mat padded = Mat::zeros(src.rows + 2 * pad, src.cols + 2 * pad, CV_8UC1);
    for (int h = 0; h < src.rows; ++h)
    {
        for (int w = 0; w < src.cols; ++w)
        {
            padded.at<uchar>(h + pad, w + pad) = src.at<uchar>(h, w);
        }
    }
    return padded;
}

// ksize x ksize Average Filter(ksize는 홀수)
Mat AverageFilter(const Mat &src, int ksize)
{
    if (src.empty())
        throw runtime_error("empty Image");
    if (src.type() != CV_8UC1)
        throw runtime_error("use 8-bit grayscale");
    if (ksize <= 0 || (ksize % 2) == 0)
        throw runtime_error("ksize must be odd and >0");

    const int pad = ksize / 2;
    Mat padded = zeroPadding(src, pad);
    Mat dst(src.rows, src.cols, CV_8UC1, Scalar(0));

    const int W = padded.cols;
    const unsigned char *pData = padded.ptr<unsigned char>(0);
    unsigned char *outData = dst.ptr<unsigned char>(0);

    // (r, c) in Original corresponds to (r+pad, c+pad) in padded
    for (int h = pad; h < padded.rows - pad; ++h)
    {
        for (int w = pad; w < padded.cols - pad; ++w)
        {
            int sum = 0;
            // ksize x ksize 누적합
            for (int dy = -pad; dy <= pad; ++dy)
            {
                const int base = (h + dy) * W + (w - pad);
                for (int dx = 0; dx < ksize; ++dx)
                {
                    sum += pData[base + dx];
                }
            }
            const int avg = sum / (ksize * ksize);
            outData[(h - pad) * dst.cols + (w - pad)] = static_cast<uchar>(avg);
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

    Mat k3 = AverageFilter(src, 3);
    Mat k5 = AverageFilter(src, 5);
    Mat k11 = AverageFilter(src, 11);
    Mat k25 = AverageFilter(src, 25);

    imshow("Original", src);
    imshow("AverageFilter 3x3", k3);
    imshow("AverageFilter 5x5", k5);
    imshow("AverageFilter 11x11", k11);
    imshow("AverageFilter 25x25", k25);

    waitKey(0);

    return 0;
}