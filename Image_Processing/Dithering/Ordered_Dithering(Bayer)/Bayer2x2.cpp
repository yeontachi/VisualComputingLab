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
    int dither_Mat[2][2] = {
        {0, 2},
        {3, 1}};

    unsigned char *pData;
    pData = (unsigned char *)img.data;

    unsigned char *dData;
    dData = (unsigned char *)screen.data;

    for (int h = 0; h < img.rows; ++h)
    {
        for (int w = 0; w < img.cols; ++w)
        {
            int i = w % 2;
            int j = h % 2;

            if (pData[h * img.cols + w] > dither_Mat[j][i] * 64)
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