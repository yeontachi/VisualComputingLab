#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

// RGB 공간상 구의 반지름
static double pixelDist(const Vec3b &a, const Vec3b &b)
{
    double db = a[0] - b[0];
    double dg = a[1] - b[1];
    double dr = a[2] - b[2];

    return sqrt(db * db + dg * dg + dr * dr);
}

// 각 좌표별 중앙값 구하기(3채널 중심점 찾기 위함)
static unsigned char calCenter(unsigned char lo, unsigned char hi)
{
    return static_cast<unsigned char>(((int)lo + (int)hi) / 2);
}

static bool isBerry(const Vec3b &pixel)
{
    static const Vec3b LOW(7, 15, 141);
    static const Vec3b HIGH(146, 119, 205);
    static const Vec3b center(calCenter(LOW[0], HIGH[0]), calCenter(LOW[1], HIGH[1]), calCenter(LOW[2], HIGH[2]));

    static const double radius = pixelDist(LOW, HIGH) / 2;

    return pixelDist(pixel, center) < radius;
}
Mat meanFilter(const Mat &src, int ksize)
{
    CV_Assert(src.type() == CV_8UC3);
    Mat dst(src.size(), src.type());

    int radius = ksize / 2;
    unsigned char *pSrc = src.data;
    unsigned char *pDst = dst.data;

    int step = src.step;

    for (int y = 0; y < src.rows; ++y)
    {
        for (int x = 0; x < src.cols; ++x)
        {
            int sumB = 0, sumG = 0, sumR = 0;
            int count = 0;

            for (int dy = -radius; dy <= radius; ++dy)
            {
                for (int dx = -radius; dx <= radius; ++dx)
                {
                    int ny = y + dy;
                    int nx = x + dx;

                    if (ny >= 0 && ny < src.rows && nx >= 0 && nx < src.cols)
                    {
                        unsigned char *pN = pSrc + ny * step + nx * 3;
                        sumB += pN[0];
                        sumG += pN[1];
                        sumR += pN[2];
                        count++;
                    }
                }
            }

            unsigned char *pOut = pDst + y * step + x * 3;
            pOut[0] = static_cast<unsigned char>(sumB / count);
            pOut[1] = static_cast<unsigned char>(sumG / count);
            pOut[2] = static_cast<unsigned char>(sumR / count);
        }
    }

    return dst;
}
int main(void)
{
    Mat src = imread("Strawberries.jpg");

    if (src.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }

    // Color Slicing
    Mat dst(src.rows, src.cols, CV_8UC3);

    for (int y = 0; y < src.rows; ++y)
    {
        for (int x = 0; x < src.cols; ++x)
        {
            const Vec3b &p = src.at<Vec3b>(y, x); // 원본 영상에서 현재 픽셀값 접근

            if (isBerry(p))
            {
                // 딸기(선택 영역)는 원본 색 유지
                dst.at<Vec3b>(y, x) = p;
            }
            else
            {
                // 배경 검정색
                dst.at<Vec3b>(y, x) = Vec3b(0, 0, 0);
            }
        }
    }

    Mat smooth = meanFilter(src, 13);
    Mat final = src.clone();

    unsigned char *pSmooth = smooth.data;
    unsigned char *pFinal = final.data;

    int stepSmooth = smooth.step;
    int stepFinal = final.step;

    for (int h = 0; h < src.rows; ++h)
    {
        unsigned char *rowS = pSmooth + h * stepSmooth;
        unsigned char *rowF = pFinal + h * stepFinal;
        for (int w = 0; w < src.cols; ++w)
        {
        }
    }

    imshow("Original", src);
    imshow("Color Sliced", dst);

    waitKey(0);
    return 0;
}