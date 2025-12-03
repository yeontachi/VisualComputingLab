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
    int blockSize = 2;    // 코너를 계산할 때 고려하는 이웃 픽셀의 크기
    int apertureSize = 3; // Sobel 미분 연산자를 적용할 때 사용되는 Kernel 크기, 이미지 미분(기울기 계산)의 정밀도와 스무딩 정도를 결정하는 파라미터
    double k = 0.04;      // Harris 자유 파라미터(일반적으로 0.04가 사용됨)

    Mat dst = Mat::zeros(src.size(), CV_32FC1);
    // 계산 결과인 코너 응답 값이 실수이기 때문에 CV_32FC1을 사용한다. 정수를 이용할 경우 정보가 손실될 수 있다.(음수 표현 불가, 소수점 등)

    cornerHarris(src_gray, dst, blockSize, apertureSize, k);
    // dst : 코너 응답 함수의 값들 저장(코너일 가능성을 수치적으로 나타냄)
    /*
    R>0 corner
    R<0 Edge
    R=0 Flat region
    */

    // imshow("cornerHarris dst", dst);

    Mat dst_norm, dst_norm_scaled;

    normalize(dst, dst_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat());
    // 코너 응답 함수 값4들을 normalize 함수를 통해 R 값을 0~255 사이로 스케일링 후 thresh값과 비교
    /*
    normarlize > zero-> 127.5 정도로 정규화
    */
    // convertScaleAbs(dst_norm, dst_norm_scaled);
    dst_norm_scaled = src.clone();

    for (int i = 0; i < dst_norm.rows; ++i)
    {
        for (int j = 0; j < dst_norm.cols; ++j)
        {
            if ((int)dst_norm.at<float>(i, j) > thresh) // Corner로 인식된 값 중에, thresh보다 크다면 원을 그려 표현
            {
                circle(dst_norm_scaled, Point(j, i), 1, Scalar(0, 0, 255), 2, 8, 0);
            } // 이미지에 그리는 함수(circle, line, rectangle)에 전달하는 좌표는 Opencv Mat 객체 행렬 인덱스와 인자의 순서를 반대로 적용해서 보낸다.
        }
    }
    namedWindow(corners_window);
    imshow(corners_window, dst_norm_scaled);
}