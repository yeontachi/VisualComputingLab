#include <opencv2/opencv.hpp>
#include <iostream>
using namespace std;
using namespace cv;
int SharpenFilter[9] = {0, -1, 0, -1, 5, -1, 0, -1, 0};
int main(void)
{
    Mat img = imread("Lenna.png", IMREAD_GRAYSCALE);
    unsigned char *pData;
    pData = (unsigned char *)img.data;
    int newRows = img.rows + 2;
    int newCols = img.cols + 2;
    unsigned char *padded = new unsigned char[newRows * newCols]();
    for (int h = 0; h < img.rows; ++h)
    {
        for (int w = 0; w < img.cols; ++w)
        {
            int srcIdx = h * img.cols + w;
            int dstIdx = (h + 1) * newCols + (w + 1);
            padded[dstIdx] = pData[srcIdx];
        }
    }
    Mat paddedMat(newRows, newCols, CV_8UC1, padded);
    imshow("Original", img);
    imshow("padded", paddedMat);
    waitKey(0);
}