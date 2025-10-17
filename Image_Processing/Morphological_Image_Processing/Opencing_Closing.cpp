#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

/*
이진 영상(Binary Image)에 대해서
Erosion(침식)과 Dilation(팽창) 적용 코드.

Erosion > 흰색 객체(정사각형)가 작아짐
Dilation > 흰색 객체가 커짐
*/
int main(void)
{
    Mat src = imread("img2.png", IMREAD_GRAYSCALE);

    if (src.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }

    Mat bin;
    threshold(src, bin, 128, 255, THRESH_BINARY);

    // 커널(구조 요소) 생성 (3x3 사각형)
    Mat kernel = getStructuringElement(MORPH_RECT, Size(17, 17));

    // Opening
    Mat eroded;
    erode(bin, eroded, kernel, Point(-1, -1));
    dilate(bin, eroded, kernel, Point(-1, -1));

    // Closing
    Mat dilated;
    dilate(bin, dilated, kernel, Point(-1, -1));
    erode(bin, dilated, kernel, Point(-1, -1));

    imshow("Original", bin);
    imshow("Opening", eroded);
    imshow("Closing", dilated);

    waitKey(0);

    return 0;
}