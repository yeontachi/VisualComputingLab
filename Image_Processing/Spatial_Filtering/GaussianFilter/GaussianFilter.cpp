#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int wx[8] = {-1, 0, 1, 1, 1, 0, -1, -1}; // 좌상단부터 시게방향
int wy[8] = {-1, -1, -1, 0, 1, 1, 1, 0};

// 3x3 Gaussian kernel -> 배열 (0=center, 1~8 시계방향)
void createGaussian3x3_CenterFirst(double sigma, double outKernel[9])
{
    int ksize = 3;
    int half = ksize / 2;
    double mat[3][3];
    double sum = 0.0;

    // 먼저 3x3 Gaussian 값 계산
    for (int i = -half; i <= half; i++)
    {
        for (int j = -half; j <= half; j++)
        {
            double value = exp(-(i * i + j * j) / (2 * sigma * sigma));
            mat[i + half][j + half] = value;
            sum += value;
        }
    }

    // 정규화
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            mat[i][j] /= sum;

    // 원하는 순서로 outKernel에 넣기
    outKernel[0] = mat[1][1]; // center
    outKernel[1] = mat[0][0]; // 좌상단
    outKernel[2] = mat[0][1]; // 상단
    outKernel[3] = mat[0][2]; // 우상단
    outKernel[4] = mat[1][2]; // 오른쪽
    outKernel[5] = mat[2][2]; // 우하단
    outKernel[6] = mat[2][1]; // 아래
    outKernel[7] = mat[2][0]; // 좌하단
    outKernel[8] = mat[1][0]; // 왼쪽
}

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

Mat Gaussian_Filter(const Mat &src)
{
    Mat PaddedSrc = zeroPadding(src);

    // 원본 데이터 접근
    unsigned char *pData;
    pData = (unsigned char *)PaddedSrc.data;

    // OutputImage
    Mat output = Mat::zeros(src.rows, src.cols, CV_8UC1);

    unsigned char *outData;
    outData = (unsigned char *)output.data;

    double kernel[9];
    createGaussian3x3_CenterFirst(3.0, kernel);

    for (int h = 1; h < PaddedSrc.rows - 1; ++h)
    {
        for (int w = 1; w < PaddedSrc.cols - 1; ++w)
        {
            double Gaussian = pData[h * PaddedSrc.cols + w] * kernel[0];
            for (int i = 0; i < 8; i++) // 가운데 픽셀 기준으로 좌상단부터 시계방향으로 누적합 계산
            {
                Gaussian += pData[(h + wy[i]) * PaddedSrc.cols + (w + wx[i])] * kernel[i + 1];
            }

            if (Gaussian < 0.0)
                Gaussian = 0.0;
            else if (Gaussian > 255.0)
                Gaussian = 255.0;

            outData[(h - 1) * output.cols + (w - 1)] = static_cast<unsigned char>(std::lround(Gaussian));
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

    Mat dstImg = Gaussian_Filter(src); // Average Filter 적용

    imshow("Original", src);
    imshow("Gaussian Filter", dstImg);

    waitKey(0);

    return 0;
}