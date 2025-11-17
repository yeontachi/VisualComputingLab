#include <opencv2/opencv.hpp>
#include <iostream>

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

Mat Image_translation(const Mat &src, double x_trans, double y_trans)
{
    Mat Out_trans = Mat::zeros(src.size(), CV_8UC1);

    double h_Trans = 0;
    double w_Trans = 0;

    for (int h = 0; h < src.rows; ++h)
    {
        for (int w = 0; w < src.cols; ++w)
        {
            h_Trans = h - y_trans;
            w_Trans = w - x_trans;

            if (isInsideBoundary(src.rows, src.cols, h_Trans, w_Trans))
            {
                Out_trans.at<unsigned char>(h, w) = src.at<unsigned char>(h_Trans, w_Trans);
            }
        }
    }

    return Out_trans;
}

Mat BGR_ImageTranslation(const Mat &src, double x_trans, double y_trans)
{
    CV_Assert(src.type() == CV_8UC3);

    vector<Mat> bgr_channels;
    split(src, bgr_channels);

    bgr_channels[0] = Image_translation(bgr_channels[0], x_trans, y_trans);
    bgr_channels[1] = Image_translation(bgr_channels[1], x_trans, y_trans);
    bgr_channels[2] = Image_translation(bgr_channels[2], x_trans, y_trans);

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

    double x_trans, y_trans;

    cout << "Translation(x, y): ";
    cin >> x_trans >> y_trans;

    Mat dst_gray = Image_translation(src_gray, x_trans, y_trans);
    Mat dst_color = BGR_ImageTranslation(src_color, x_trans, y_trans);

    imshow("Original", src_color);

    imshow("translation Gray", dst_gray);
    imshow("translation Color", dst_color);

    waitKey(0);

    return 0;
}