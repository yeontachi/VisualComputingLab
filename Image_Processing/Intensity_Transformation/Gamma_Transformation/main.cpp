#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

using namespace std;
using namespace cv;

static inline unsigned char gamma_transformation(const Mat &src, int h, int w, double gamma)
{
    // 입력 픽셀값
    unsigned char pixel = src.at<uchar>(h, w);

    // [0, 1]로 정규화
    double r = pixel / 255.0;

    // 감마 보정
    double s = pow(r, gamma);

    // 다시 [0, 255] 변환 후 리턴
    return static_cast<unsigned char>(s * 255.0);
}

int main(void)
{
    Mat img = imread("Lena.png", IMREAD_GRAYSCALE);

    if (img.empty())
    {
        cerr << "Error, Image Not Found" << "\n";
        return -1;
    }

    // 감마값 입력 받기
    double gamma;

    cout << "Gamma: ";
    cin >> gamma;

    Mat img_out(img.rows, img.cols, CV_8UC1);
    if (!img_out.isContinuous())
        img_out = img_out.clone();

    // 픽셀 값 접근
    unsigned char *pData;
    pData = (unsigned char *)img_out.data;

    for (int h = 0; h < img.rows; ++h)
    {
        for (int w = 0; w < img.cols; ++w)
        {
            pData[h * img.cols + w] = gamma_transformation(img, h, w, gamma);
        }
    }

    imshow("Gamma Transformation Image", img_out);

    waitKey(0);

    return 0;
}