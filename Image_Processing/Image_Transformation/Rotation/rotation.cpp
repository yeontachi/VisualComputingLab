#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

using namespace std;
using namespace cv;

bool isInsideBoundary(int nHeight, int nWidth, double h, double w)
{
    if (h >= 0 && w >= 0 && h < nHeight && w < nWidth)
    {
        return true;
    }
    else
        return false;
}

unsigned char Bilinear_Interpolation(const Mat &src, int nHeight_Ori, int nWidth_Ori, double h_Cvt, double w_Cvt)
{
    int px = w_Cvt;
    int py = h_Cvt;

    if (px == nWidth_Ori - 1)
        px = px - 1;
    if (py == nHeight_Ori - 1)
        py = py - 1;

    double fx1 = w_Cvt - (int)w_Cvt;
    double fx2 = 1 - fx1;
    double fy1 = h_Cvt - (int)h_Cvt;
    double fy2 = 1 - fy1;

    double w1 = fx2 * fy2;
    double w2 = fx1 * fy2;
    double w3 = fx2 * fy1;
    double w4 = fx1 * fy1;

    unsigned char P1 = src.at<unsigned char>(py, px);
    unsigned char P2 = src.at<unsigned char>(py, px + 1);
    unsigned char P3 = src.at<unsigned char>(py + 1, px);
    unsigned char P4 = src.at<unsigned char>(py + 1, px + 1);

    int result = w1 * P1 + w2 * P2 + w3 * P3 + w4 * P4;

    if (result < 0)
        result = 0;
    if (result > 255)
        result = 255;

    return (unsigned char)result;
}

#define PI 3.141592265358979323846
Mat Image_Rotation(const Mat &src, int nAngle)
{
    Mat Out_Rotate = Mat::zeros(src.size(), CV_8UC1);

    int H_center = src.rows / 2;
    int W_center = src.cols / 2;

    double h_Rotate = 0;
    double w_Rotate = 0;

    double rad = PI * (nAngle / 180.);

    for (int h = 0; h < src.rows; ++h)
    {
        for (int w = 0; w < src.cols; ++w)
        {
            w_Rotate = cos(rad) * (w - W_center) - sin(rad) * (h - H_center) + W_center;
            h_Rotate = cos(rad) * (h - H_center) + sin(rad) * (w - W_center) + H_center;

            if (isInsideBoundary(src.rows, src.cols, h_Rotate, w_Rotate))
            {
                Out_Rotate.at<unsigned char>(h, w) = Bilinear_Interpolation(src, src.rows, src.cols, h_Rotate, w_Rotate);
            }
        }
    }

    return Out_Rotate;
}

Mat RGB_ImageRotation(const Mat &src, int nAngle)
{
    CV_Assert(src.type() == CV_8UC3);

    vector<Mat> bgr_channels;
    split(src, bgr_channels);

    bgr_channels[0] = Image_Rotation(bgr_channels[0], nAngle);
    bgr_channels[1] = Image_Rotation(bgr_channels[1], nAngle);
    bgr_channels[2] = Image_Rotation(bgr_channels[2], nAngle);

    Mat dst;
    merge(bgr_channels, dst);

    return dst;
}

int main(void)
{
    Mat src = imread("Lena.png", IMREAD_GRAYSCALE);
    if (src.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }

    int nAngle;

    cout << "Rotate Angle: ";
    cin >> nAngle;

    Mat Out_rotate = Image_Rotation(src, nAngle);

    imshow("Original", src);
    imshow("Rotate Image", Out_rotate);

    Mat colorsrc = imread("Lena.png");
    if (colorsrc.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }

    Mat Out_rotate_color = RGB_ImageRotation(colorsrc, nAngle);

    imshow("Color Image Rotation", Out_rotate_color);

    waitKey(0);

    return 0;
}