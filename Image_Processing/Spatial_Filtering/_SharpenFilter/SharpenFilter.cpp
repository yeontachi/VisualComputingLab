#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int wx[8] = {-1, 0, 1, 1, 1, 0, -1, -1}; // 좌상단부터 시게방향
int wy[8] = {-1, -1, -1, 0, 1, 1, 1, 0};

int SharpenFilter[9] = {5, 0, -1, 0, -1, 0, -1, 0, -1}; // 가운데 > 좌상단부터 시계방향

// Zero Padding
Mat zeroPadding(const Mat &src)
{
    int newRows = src.rows + 2;
    int newCols = src.cols + 2;

    Mat paddedMat = Mat::zeros(newRows, newCols, CV_8UC1);

    for (int h = 0; h < src.rows; ++h)
    {
        for (int w = 0; w < src.cols; ++w)
        {
            paddedMat.at<uchar>(h + 1, w + 1) = src.at<uchar>(h, w); // at 접근법
        }
    }

    return paddedMat;
}

Mat Sharpen_Filter(const Mat &src)
{
    Mat PaddedSrc = zeroPadding(src);

    // 원본 데이터 접근
    unsigned char *pData;
    pData = (unsigned char *)PaddedSrc.data;

    // OutputImage
    Mat output = Mat::zeros(src.rows, src.cols, CV_8UC1);

    unsigned char *outData;
    outData = (unsigned char *)output.data;

    for (int h = 1; h < PaddedSrc.rows - 1; ++h)
    {
        for (int w = 1; w < PaddedSrc.cols - 1; ++w)
        {
            int Sharpen = pData[h * PaddedSrc.cols + w] * SharpenFilter[0];
            for (int i = 0; i < 8; i++) // 가운데 픽셀 기준으로 좌상단부터 시계방향으로 누적합 계산
            {
                Sharpen += pData[(h + wy[i]) * PaddedSrc.cols + (w + wx[i])] * SharpenFilter[i + 1];
            }

            outData[(h - 1) * output.cols + (w - 1)] = static_cast<unsigned char>(Sharpen);
        }
    }

    return output;
}

int main(void)
{
    Mat src = imread("Lena.png", IMREAD_GRAYSCALE);

    if (src.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }

    Mat dstImg = Sharpen_Filter(src); // Average Filter 적용

    imshow("Original", src);
    imshow("Sharpen Filter", dstImg);

    waitKey(0);

    return 0;
}