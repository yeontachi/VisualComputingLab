#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

Mat src, src_gray;
int thresh = 150;
int max_thresh = 255;
const char *source_window = "Source Image";
const char *corners_window = "Corners detected";
void cornerHarris_demo(int, void *);

int main(void)
{
    src = imread("Lena.png");
    cvtColor(src, src_gray, COLOR_BGR2GRAY);
    namedWindow(source_window);
    createTrackbar("Threshold: ", source_window, &thresh, max_thresh, cornerHarris_demo);

    imshow(source_window, src);
    cornerHarris_demo(0, 0);

    waitKey();

    return 0;
}

void cornerHarris_demo(int, void *)
{
    int blockSize = 2;
    int apertureSize = 3;
    double k = 0.04;

    Mat dst = Mat::zeros(src.size(), CV_32FC1);

    cornerHarris(src_gray, dst, blockSize, apertureSize, k);

    imshow("cornerHarris dst", dst);

    Mat dst_norm, dst_norm_scaled;

    normalize(dst, dst_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat());
    // convertScaleAbs(dst_norm, dst_norm_scaled);
    dst_norm_scaled = src.clone();

    for (int i = 0; i < dst_norm.rows; ++i)
    {
        for (int j = 0; j < dst_norm.cols; ++j)
        {
            if ((int)dst_norm.at<float>(i, j) > thresh) // Corner로 인식된 값 중에, thresh보다 크다면 원을 그려 표현
            {
                circle(dst_norm_scaled, Point(j, i), 1, Scalar(0, 0, 255), 2, 8, 0);
            }
        }
    }
    namedWindow(corners_window);
    imshow(corners_window, dst_norm_scaled);
}