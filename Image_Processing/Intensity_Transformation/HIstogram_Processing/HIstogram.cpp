#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int main(void)
{
    Mat src = imread("Lena.png", IMREAD_GRAYSCALE);

    if (src.empty())
    {
        cerr << "Error, Image Not Found" << "\n";
        return -1;
    }

    // 히스토그램 배열
    int hist[256] = {
        0,
    };

    // 히스토그램 배열에 pixel value의 개수 저장(0-255 개수)
    for (int h = 0; h < src.rows; ++h)
    {
        for (int w = 0; w < src.cols; ++w)
        {
            hist[src.at<uchar>(h, w)]++;
        }
    }

    // hist[] > 정규화(0~hist_h 범위)
    int hist_w = 512, hist_h = 400;
    // bin은 막대 하나의 너비. (0-255, 총 256개의 bin)
    // 전체 너비 hist_w를 256으로 나누어 한 bin 당 픽셀 너비를 구함.
    int bin_w = cvRound((double)hist_w / 256); // 즉, 한 구간(bin)을 화면에 몇 픽셀로 그릴지 결정

    Mat histImage(hist_h, hist_w, CV_8UC1, Scalar(255)); // 히스토그램을 그릴 스크린(흰색 스크린)

    // 최대값 찾기
    int max_val = 0;
    for (int i = 0; i < 256; i++)
    {
        if (hist[i] > max_val)
            max_val = hist[i];
    }

    // 막대 그리기
    for (int i = 0; i < 256; i++)
    {
        int height = cvRound(((double)hist[i] / max_val) * hist_h); // 히스토그램 높이 정규화(0~hist_h)
        // 히스토그램 분포 막대 그리기
        rectangle(histImage,
                  Point(i * bin_w, hist_h - height), // 왼쪽 위 꼭짓점
                  Point((i + 1) * bin_w, hist_h),    // 오른쪽 아래 꼭짓점
                  Scalar(0),
                  FILLED);
    }

    imshow("Source", src);
    imshow("Histogram", histImage);
    waitKey();

    return 0;
}