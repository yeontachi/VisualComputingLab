#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

static inline unsigned char clamp8(int v)
{
    return (unsigned char)std::max(0, std::min(255, v));
}

// Zero Padding
Mat ZeroPadding(const Mat &src, int pad)
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

// Median Filter
Mat MedianFilter(const Mat &src, int ksize)
{
    if (src.empty())
        throw runtime_error("empty Image");
    if (src.type() != CV_8UC1)
        throw runtime_error("use 8-bit grayscale");
    if (ksize <= 0 || (ksize % 2) == 0)
        throw runtime_error("ksize must be odd and >0");

    const int pad = ksize / 2;
    Mat padded = ZeroPadding(src, pad);
    Mat dst(src.rows, src.cols, CV_8UC1, Scalar(0));

    const int W = padded.cols;
    const unsigned char *pData = padded.ptr<unsigned char>(0);
    unsigned char *outData = dst.ptr<unsigned char>(0);

    for (int h = pad; h < padded.rows - pad; ++h)
    {
        for (int w = pad; w < padded.cols - pad; ++w)
        {
            vector<uchar> window;
            window.reserve(ksize * ksize);

            for (int y = -pad; y <= pad; ++y)
            {
                const uchar *row = padded.ptr<uchar>(h + y);
                for (int x = -pad; x <= pad; ++x)
                {
                    window.push_back(row[w + x]);
                }
            }

            // 오름차순 정렬
            sort(window.begin(), window.end());

            // median
            int mid = window.size() / 2;
            outData[(h - pad) * src.cols + (w - pad)] = window[mid];
        }
    }
    return dst;
}

int main(void)
{
    Mat GN10 = imread("GaussianNoise10.png", IMREAD_GRAYSCALE); // 가우시안 노이즈 이미지
    if (GN10.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }
    Mat GN25 = imread("GaussianNoise25.png", IMREAD_GRAYSCALE); // 가우시안 노이즈 이미지
    if (GN25.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }

    Mat SPN005 = imread("lena512SPN005.png", IMREAD_GRAYSCALE); // 소금후추 노이즈 이미지
    if (SPN005.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }
    Mat SPN010 = imread("lena512SPN010.png", IMREAD_GRAYSCALE); // 소금후추 노이즈 이미지
    if (SPN010.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }

    int ksize = 3; // filter 사이즈
    cout << "커널 사이즈: ";
    cin >> ksize;

    Mat GN10out = MedianFilter(GN10, ksize);
    Mat GN25out = MedianFilter(GN25, ksize);

    Mat SPN005out = MedianFilter(SPN005, ksize);
    Mat SPN010out = MedianFilter(SPN010, ksize);

    imshow("Gaussian Noise10", GN10);
    imshow("Gaussian Noise25", GN25);
    imshow("Salt and Pepper Noise 0.05", SPN005);
    imshow("Salt and Pepper Noise 0.10", SPN010);

    imshow("GN10_MedianFilter", GN10out);
    imshow("GN25_MedianFilter", GN25out);

    imshow("SPN005_MedianFilter", SPN005out);
    imshow("SPN010_MedianFilter", SPN010out);

    waitKey(0);

    return 0;
}