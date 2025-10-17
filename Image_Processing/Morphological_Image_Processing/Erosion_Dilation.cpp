#include <opencv2/opencv.hpp>
#include <iostream>
using namespace std;
using namespace cv;

int main()
{
    // 1) 흑백 이진 영상 읽기
    Mat src = imread("binary_squares.png", IMREAD_GRAYSCALE);
    if (src.empty())
    {
        cerr << "Image not found!" << endl;
        return -1;
    }

    // 2) 이진화 (혹시 회색값이 섞여 있을 때)
    Mat bin;
    threshold(src, bin, 128, 255, THRESH_BINARY);

    // 3) 커널(구조요소) 생성 (3x3 사각형)
    Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));

    // 4) Erosion (침식)
    Mat eroded;
    erode(bin, eroded, kernel, Point(-1, -1), 1);

    // 5) Dilation (팽창)
    Mat dilated;
    dilate(bin, dilated, kernel, Point(-1, -1), 1);

    // 6) 결과 출력
    imshow("Original", bin);
    imshow("Erosion", eroded);
    imshow("Dilation", dilated);

    waitKey(0);
    return 0;
}
