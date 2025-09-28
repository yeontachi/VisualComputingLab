#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

using namespace std;
using namespace cv;

// Histogram Equalization 함수
Mat Histogram_Equalization(const Mat &src);

int main(void)
{
    Mat src = imread("Lena.png", IMREAD_GRAYSCALE); // 원본 레나 영상
    Mat lena1 = src / 2;                            // 레나 영상 변환 1 : s = r / 2
    Mat lena2 = 128 + src / 2;                      // 레나 영상 변환 2 : s = 128 + r/2

    imshow("Original lena", src); // 원본 레나 영상
    imshow("lena1", lena1);       // 레나 영상 변환 1 : s = r / 2
    imshow("lena2", lena2);       // 레나 영상 변환 2 : s = 128 + r/2

    Mat HE_src = Histogram_Equalization(src);
    Mat HE_lena1 = Histogram_Equalization(lena1);
    Mat HE_lena2 = Histogram_Equalization(lena2);

    imshow("HE_Original", HE_src); // 원본 레나 영상의 Histogram Equalization 이후 영상
    imshow("HE_lena1", HE_lena1);  // lena1의 Histogram Equalization
    imshow("HE_lena2", HE_lena2);  // lena2의 Histogram Equalization

    waitKey(0);

    return 0;
}

Mat Histogram_Equalization(const Mat &src)
{
    // 입력 영상이 단일 채널 8비트가 아니면, exit한다.
    if (src.type() != CV_8UC1)
    {
        cerr << "Error, Input Image must be 8-bit GrayScale(CV_8UC1)" << "\n";
        exit(-1);
    }

    const int rows = src.rows;
    const int cols = src.cols;
    const size_t sstep = src.step; // 한 행의 바이트 수, CV_8UC1이면 보통 Step==cols이다.

    // 히스토그램 계산
    int hist[256] = {
        // 히스토그램 초기화
        0,
    };

    unsigned char *sdata;
    sdata = (unsigned char *)src.data;

    if (src.isContinuous())
    {
        const size_t Nbytes = rows * sstep;
        for (size_t i = 0; i < Nbytes; ++i)
        {
            hist[sdata[i]]++; // 픽셀값(0-255)의 빈도 수 계산
        }
    }

    // cdf 계산 (표준 누적 분포)
    int cdf[256];
    cdf[0] = hist[0];
    for (int i = 1; i < 256; ++i) // 0부터 현재 밝기까지 픽셀이 몇 개 잇는지 누적한 값
    {
        cdf[i] = cdf[i - 1] + hist[i];
    }

    // cdf 최소값 찾기, 영상에서 실제로 등장하는 가장 낮은 밝기의 첫 누적 값, 실제 최소 밝기를 0으로 매핑하가 위해 구한다.
    int cdf_min = 0;
    for (int i = 0; i < 256; ++i)
    {
        if (cdf[i] > 0)
        {
            cdf_min = cdf[i];
            break;
        }
    }

    // Look Up Table 생성 s_k = round( (cdf[k] - cdf_min) / (N - cdf_min) * 255 )
    const int N = rows * cols;
    unsigned char lut[256];
    if (N == cdf_min)
    {
        // 모든 픽셀이 같은 값이 경우 동일 매핑
        for (int i = 0; i < 256; ++i)
        {
            lut[i] = static_cast<unsigned char>(i);
        }
    }
    else
    {
        for (int k = 0; k < 256; ++k)
        {
            double sk = (static_cast<double>(cdf[k] - cdf_min) / (N - cdf_min)) * 255.0;
            if (sk < 0.0)
                sk = 0.0;
            if (sk > 255.0)
                sk = 255.0;
            lut[k] = static_cast<unsigned char>(std::lround(sk));
        }
    }

    // lut 적용하여 출력 생성
    Mat dst(rows, cols, CV_8UC1);
    unsigned char *dData;
    dData = (unsigned char *)dst.data;

    const size_t dstep = dst.step;

    for (int h = 0; h < rows; ++h)
    {
        const unsigned char *srow = sdata + h * sstep;
        unsigned char *drow = dData + h * dstep;
        for (int w = 0; w < cols; ++w)
        {
            drow[w] = lut[srow[w]];
        }
    }

    return dst;
}