#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

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

Mat Image_Shearing(const Mat &src, double shx, double shy)
{
    CV_Assert(src.type() == CV_8UC1);

    Mat dst = Mat::zeros(src.size(), CV_8UC1);

    double det = 1.0 - shx * shy;
    if (fabs(det) < 1e-8)
    {
        cerr << "Determinant is too small. Choose different shear values." << endl;
        return dst;
    }

    for (int h = 0; h < dst.rows; ++h)
    {
        for (int w = 0; w < dst.cols; ++w)
        {
            double ws = (w - shx * h) / det;
            double hs = (h - shy * w) / det;

            if (isInsideBoundary(src.rows, src.cols, hs, ws))
            {
                dst.at<unsigned char>(h, w) = src.at<unsigned char>((int)hs, (int)ws);
            }
        }
    }

    return dst;
}

Mat BGR_ImageShearing(const Mat &src, double shx, double shy)
{
    CV_Assert(src.type() == CV_8UC3);

    vector<Mat> bgr_channels;
    split(src, bgr_channels);

    bgr_channels[0] = Image_Shearing(bgr_channels[0], shx, shy);
    bgr_channels[1] = Image_Shearing(bgr_channels[1], shx, shy);
    bgr_channels[2] = Image_Shearing(bgr_channels[2], shx, shy);

    Mat dst;
    merge(bgr_channels, dst);

    return dst;
}

int main(void)
{
    Mat src_gray = imread("Lena.png", IMREAD_GRAYSCALE);
    Mat src_color = imread("Lena.png");

    if (src_gray.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }

    double shx, shy;
    cout << "Shear factors (shx, shy): ";
    cin >> shx >> shy;

    Mat dst_gray = Image_Shearing(src_gray, shx, shy);
    Mat dst_color = BGR_ImageShearing(src_color, shx, shy);

    imshow("Original", src_color);
    imshow("Shearing Gray", dst_gray);
    imshow("Shearing Color", dst_color);

    waitKey(0);

    return 0;
}