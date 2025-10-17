#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <stdio.h>
#include <iostream>

using namespace std;
using namespace cv;

int main(int argc, char **argv)
{
    Mat img_gray, img_edge1, img_edge2;
    Mat img = imread("Lena.png", IMREAD_COLOR);

    if (img.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }

    cvtColor(img, img_gray, COLOR_BGR2GRAY);

    GaussianBlur(img_gray, img_edge1, Size(5, 5), 0, 0);
    Canny(img_edge1, img_edge1, 100, 127, 3);

    GaussianBlur(img_gray, img_edge2, Size(5, 5), 0, 0);
    Canny(img_edge2, img_edge2, 70, 255, 3);

    imshow("thres 100-127", img_edge1);
    imshow("thres 100-200", img_edge2);

    waitKey(0);
    return 0;
}