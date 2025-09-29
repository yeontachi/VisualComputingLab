#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstdlib> // rand, srand
#include <ctime>   // time

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

    unsigned char *pData;
    pData = (unsigned char *)img.data;

    unsigned char *dData;
    dData = (unsigned char *)screen.data;

    // 랜덤 시드 초기화
    srand((unsigned int)time(NULL));

    for (int h = 0; h < img.rows; ++h)
    {
        for (int w = 0; w < img.cols; ++w)
        {
            // 픽셀당 랜덤 threshold 생성 (0~255)
            int rand_th = rand() % 256;

            if (pData[h * img.cols + w] > rand_th)
            {
                dData[h * screen.cols + w] = 255;
            }
            else
            {
                dData[h * screen.cols + w] = 0;
            }
        }
    }

    imshow("Dithered Image (Random)", screen);

    waitKey(0);

    return 0;
}
