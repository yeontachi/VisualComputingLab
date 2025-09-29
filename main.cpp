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

    unsigned char *sData;
    sData = (unsigned char *)img.data;

    Mat screen = Mat::zeros(img.rows, img.cols, CV_8UC1);

    unsigned char *pData;
    pData = (unsigned char *)screen.data;

    for (int h = 0; h < img.rows; ++h)
    {
        for (int w = 0; w < img.cols; ++w)
        {
            int pixelV = sData[h * img.cols + w];
            if (pixelV < 128)
                pData[h * img.cols + w] = 0;
            else
                pData[h * img.cols + w] = 255;
        }
    }

    imshow("1bit lena", screen);
    waitKey(0);
    return 0;
}
