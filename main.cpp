/*
Intensity Level Slicing 함수를 생성하고,
각 채널 (RGB)에 대해 따로 적용하여,
각 채널 별 Low, High 값을 임계값으로 두고 Slicing을 적용한다.
*/
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;
using namespace cv;

// 단일 채널 Intensity Level Slicing
static inline Mat IntensityLevelSlicing(const Mat &ch, int Low, int High)
{
    Low = std::clamp(Low, 0, 255);
    High = std::clamp(High, 0, 255);
    if (Low > High)
        std::swap(Low, High);

    Mat out = ch.clone();
    const int rows = ch.rows, cols = ch.cols;

    for (int h = 0; h < rows; ++h)
    {
        const uchar *srcp = ch.ptr<uchar>(h);
        uchar *dstp = out.ptr<uchar>(h);
        for (int w = 0; w < cols; ++w)
        {
            uchar v = srcp[w];
            if (v < Low || v > High)
                dstp[w] = 0;
        }
    }
    return out;
}

// 3채널 Color 영상 Slicing
static inline Mat ColorSlicing(Mat &src)
{
    int R_low, R_high, G_low, G_high, B_low, B_high;

    R_low = 70;
    R_high = 205;
    G_low = 15;
    G_high = 109;
    B_low = 7;
    B_high = 131;

    vector<Mat> ch(3);
    split(src, ch);

    Mat b, g, r;

    b = IntensityLevelSlicing(ch[0], B_low, B_high);
    g = IntensityLevelSlicing(ch[1], G_low, G_high);
    r = IntensityLevelSlicing(ch[2], R_low, R_high);

    for (int h = 0; h < src.rows; ++h)
    {
        for (int w = 0; w < src.cols; ++w)
        {
            uchar *bData = b.ptr<uchar>(h);
            uchar *gData = g.ptr<uchar>(h);
            uchar *rData = r.ptr<uchar>(h);

            for (int w = 0; w < src.cols; ++w)
            {
                if (bData[w] == 0 || gData[w] == 0 || rData[w] == 0)
                {
                    bData[w] = 0;
                    gData[w] = 0;
                    rData[w] = 0;
                }
            }
        }
    }

    // 채널 다시 합침
    Mat merged;
    merge(vector<Mat>{b, g, r}, merged);

    return merged;
}

int main(void)
{
    Mat src = imread("Strawberries.jpg");
    if (src.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }

    Mat dst(src.rows, src.cols, CV_8UC3);

    dst = ColorSlicing(src);

    imshow("Original", src);
    imshow("Color Sliced", dst);

    waitKey(0);

    return 0;
}