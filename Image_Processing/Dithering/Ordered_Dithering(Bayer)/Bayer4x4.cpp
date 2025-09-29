#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int main(void)
{
    Mat img = imread("Lena.png", IMREAD_GRAYSCALE);

    if (img.empty())
    {
        cerr << "Error, Image Not Found" << "\n";
        return -1;
    }

    Mat screen = Mat::zeros(img.size(), CV_8UC1);

    // Dither Matrix 선언
    int dither_Mat[4][4] = {
        {0, 8, 2, 10},
        {12, 4, 14, 6},
        {3, 11, 1, 9},
        {15, 7, 13, 5}};

    unsigned char *pData;
    pData = (unsigned char *)img.data;

    unsigned char *dData;
    dData = (unsigned char *)screen.data;

    for (int h = 0; h < img.rows; ++h)
    {
        for (int w = 0; w < img.cols; ++w)
        {
            int i = w % 4;
            int j = h % 4;

            if (pData[h * img.cols + w] > dither_Mat[j][i] * 16)
            {
                dData[h * screen.cols + w] = 255;
            }
            else
            {
                dData[h * screen.cols + w] = 0;
            }
        }
    }

    imshow("Dithered Image", screen);

    waitKey(0);

    return 0;
}