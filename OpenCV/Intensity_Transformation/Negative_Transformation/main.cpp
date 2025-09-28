#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

using namespace std;
using namespace cv;

static inline unsigned char Negative_transformation(const Mat &src, int h, int w)
{
    unsigned char pixel = src.at<uchar>(h, w);

    return static_cast<unsigned char>(255 - pixel);
}

int main(void)
{
    Mat img_in = imread("Lena.png", IMREAD_GRAYSCALE);

    if (img_in.empty())
    {
        cerr << "Error, Image Not Found" << "\n";
        return -1;
    }

    Mat img_out(img_in.rows, img_in.cols, CV_8UC1);
    if (!img_out.isContinuous())
    {
        img_out = img_out.clone();
    }

    unsigned char *pData;
    pData = (unsigned char *)img_out.data;

    for (int h = 0; h < img_in.rows; ++h)
    {
        for (int w = 0; w < img_in.cols; ++w)
        {
            pData[h * img_in.cols + w] = Negative_transformation(img_in, h, w);
        }
    }

    imshow("Original Image", img_in);
    imshow("Negative Transformation Image", img_out);

    waitKey(0);

    return 0;
}