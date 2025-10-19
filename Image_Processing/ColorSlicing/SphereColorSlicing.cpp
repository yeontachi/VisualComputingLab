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
    // BGR 기준 (OpenCV는 BGR)
    static const Vec3b LOW(7, 15, 41);
    static const Vec3b HIGH(146, 119, 205);
    static const Vec3b center(
        calCenter(LOW[0], HIGH[0]),
        calCenter(LOW[1], HIGH[1]),
        calCenter(LOW[2], HIGH[2]));

    static const double radius = pixelDist(LOW, HIGH) / 2.0;

    return pixelDist(pixel, center) < radius;
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
            const Vec3b &p = src.at<Vec3b>(y, x);

            if (isBerry(p))
            {
                // 딸기(선택 영역)는 원본 색 유지
                dst.at<Vec3b>(y, x) = p;
            }
            else
            {
                // 배경은 그레이스케일로 변환
                // OpenCV 관례의 BGR → Y(회색) 가중치 사용
                // Y = 0.114B + 0.587G + 0.299R
                double grayd = 0.114 * p[0] + 0.587 * p[1] + 0.299 * p[2];
                unsigned char gray = static_cast<unsigned char>(std::min(255.0, std::max(0.0, grayd)));
                dst.at<Vec3b>(y, x) = Vec3b(gray, gray, gray);
            }
        }
    }

    imshow("Original", src);
    imshow("Color Sliced (Strawberries Only)", dst);
    waitKey(0);
    return 0;
}
