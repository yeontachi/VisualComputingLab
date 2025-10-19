#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;
using namespace cv;

// 단일 채널 Intensity Level Slicing: 입력 보존, 출력 분리
static inline Mat IntensityLevelSlicing(const Mat &ch, int Low, int High)
{
    CV_Assert(ch.type() == CV_8UC1);

    // 파라미터 정규화
    Low = std::clamp(Low, 0, 255);
    High = std::clamp(High, 0, 255);
    if (Low > High)
        std::swap(Low, High);

    Mat out = ch.clone(); // 입력 보존 (원본 유지 + 범위 밖만 0)
    const int rows = ch.rows, cols = ch.cols;

    for (int y = 0; y < rows; ++y)
    {
        const uchar *srcp = ch.ptr<uchar>(y);
        uchar *dstp = out.ptr<uchar>(y);
        for (int x = 0; x < cols; ++x)
        {
            uchar v = srcp[x];
            if (v < Low || v > High)
                dstp[x] = 0; // 구간 밖은 0
            // 구간 안은 원본 유지
        }
    }
    return out;
}

// 3채널 Color 영상 Slicing (BGR 기준)
static inline Mat ColorSlicing(const Mat &src)
{
    CV_Assert(src.type() == CV_8UC3);

    // (슬라이싱 임계값: 채널별)
    int R_low = 70, R_high = 205;
    int G_low = 15, G_high = 109;
    int B_low = 7, B_high = 131;

    vector<Mat> ch(3);
    split(src, ch); // ch[0]=B, ch[1]=G, ch[2]=R

    Mat b = IntensityLevelSlicing(ch[0], B_low, B_high);
    Mat g = IntensityLevelSlicing(ch[1], G_low, G_high);
    Mat r = IntensityLevelSlicing(ch[2], R_low, R_high);

    Mat merged;
    merge(vector<Mat>{b, g, r}, merged);
    return merged;
}

int main()
{
    Mat src = imread("Strawberries.jpg", IMREAD_COLOR);
    if (src.empty())
    {
        cerr << "Image Not Found\n";
        return -1;
    }

    Mat dst = ColorSlicing(src);

    imshow("Original", src);
    imshow("Color Sliced", dst);
    waitKey(0);
    return 0;
}
