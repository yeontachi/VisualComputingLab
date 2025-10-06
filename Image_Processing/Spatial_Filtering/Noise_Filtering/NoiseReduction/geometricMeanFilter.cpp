#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

static inline unsigned char clamp8(int v)
{
    return (unsigned char)std::max(0, std::min(255, v));
}

// Zero Padding
Mat ZeroPadding(const Mat &src)
{
    int newRows = src.rows + 2;
    int newCols = src.cols + 2;

    Mat paddedMat = Mat::zeros(newRows, newCols, CV_8UC1);

    for (int h = 0; h < src.rows; ++h)
    {
        for (int w = 0; w < src.cols; ++w)
        {
            paddedMat.at<uchar>(h + 1, w + 1) = src.at<uchar>(h, w);
        }
    }

    return paddedMat;
}

// Geometric Mean Filter
Mat geometricMeanFilter(const Mat &src, int ksize, double eps = 1.0)
{
    const int H = src.rows;
    const int W = src.cols;
    const int psize = ksize / 2;
    const int N = ksize * ksize;

    Mat pad = ZeroPadding(src);

    Mat dst(H, W, CV_8UC1);

    // loop : 윈도우의 log 합 > 평균 > exp
    for (int h = 0; h < H; ++h)
    {
        uchar *drow = dst.ptr<uchar>(h);
        for (int w = 0; w < W; ++w)
        {
            //(h, w)에 대응하는 패딩 좌표 시작점
            int sy = h;
            int sx = w;

            double sumLog = 0.0;
            for (int j = 0; j < ksize; ++j)
            {
                const uchar *prow = pad.ptr<uchar>(sy + j);
                for (int i = 0; i < ksize; ++i)
                {
                    double val = (double)prow[sx + i] + eps; // log 안정화
                    sumLog += std::log(val);
                }
            }
            double meanLog = sumLog / (double)N;
            double gmean = std::exp(meanLog) - eps; // 역변환
            int outv = (int)std::lround(gmean);
            drow[w] = clamp8(outv);
        }
    }
    return dst;
}

int main(void)
{
    Mat GN = imread("lena256_GN.png", IMREAD_GRAYSCALE); // 가우시안 노이즈 이미지
    if (GN.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }

    Mat SPN = imread("lena256SPN.png", IMREAD_GRAYSCALE); // 소금후추 노이즈 이미지
    if (SPN.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }

    int ksize = 3;    // filter 사이즈
    double eps = 1.0; // 0의 log 방지
    cout << "커널 사이즈: ";
    cin >> ksize;
    cout << "epsilon: ";
    cin >> eps;

    Mat GNout = geometricMeanFilter(GN, ksize, eps);
    Mat SPNout = geometricMeanFilter(SPN, ksize, eps);

    imshow("Gaussian Noise", GN);
    imshow("Salt and Pepper Noise", SPN);

    imshow("GN_geometricMeanFilter", GNout);
    imshow("SPN_geometricMeanFilter", SPNout);
    imwrite("GN_gmf.png", GNout);
    imwrite("SPN_gmf.png", SPNout);

    waitKey(0);

    return 0;
}