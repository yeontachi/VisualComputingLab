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

// Histogram Equalization 함수
Mat Histogram_Equalization(const Mat &src)
{
    // 입력 영상이 단일 채널 8비트가 아니면, exit한다.
    if (src.type() != CV_8UC1)
    {
        cerr << "Error, Input Image must be 8-bit GrayScale(CV_8UC1)" << "\n";
        exit(-1);
    }

    const int rows = src.rows;     // 원본 데이터의 y축
    const int cols = src.cols;     // 원본 데이터의 x축
    const size_t sstep = src.step; // 한 행의 바이트 수, CV_8UC1이면 보통 Step==cols이다.

    // 히스토그램 계산
    int hist[256] = {
        // 히스토그램 초기화
        0,
    };

    // 원본 데이터 픽셀 접근
    unsigned char *sdata;
    sdata = (unsigned char *)src.data;

    if (src.isContinuous()) // 메모리 연속성 확인
    {
        const size_t Nbytes = rows * sstep;
        for (size_t i = 0; i < Nbytes; ++i)
        {
            hist[sdata[i]]++; // 픽셀값(0-255)의 빈도 수 계산
        }
    }

    // cdf 계산 (표준 누적 분포)
    int cdf[256];
    cdf[0] = hist[0];             // 처음 값은 그대로 적용
    for (int i = 1; i < 256; ++i) // 0부터 현재 밝기까지 픽셀이 몇 개 잇는지 누적한 값
    {
        cdf[i] = cdf[i - 1] + hist[i]; // 현재 픽셀 값의 누적합 = 이전 픽셀 값의 누적합 + 현재 픽셀 개수
    }

    // cdf 최소값 찾기, 영상에서 실제로 등장하는 가장 낮은 밝기의 첫 누적 값, 실제 최소 밝기를 0으로 매핑하가 위해 구한다.
    int cdf_min = 0;
    for (int i = 0; i < 256; ++i)
    {
        if (cdf[i] > 0) // 픽셀 개수가 1이상인 픽셀 값들 중, 픽셀 값이 가장 작은(가장 낮은 밝기 값)값을 min으로 설정
        {
            cdf_min = cdf[i];
            break;
        }
    }

    // Look Up Table 생성 s_k = round( (cdf[k] - cdf_min) / (N - cdf_min) * 255 )
    const int N = rows * cols;
    unsigned char lut[256]; // Look up table, 출력 생성에 사용하기 위함
    if (N == cdf_min)       // 픽셀 총 개수와 cdf최소값이 같다면, 모든 픽셀이 같은 값이라는 의미
    {
        // 모든 픽셀이 같은 값이 경우 동일 매핑
        for (int i = 0; i < 256; ++i)
        {
            lut[i] = static_cast<unsigned char>(i);
        }
    }
    else // 모든 픽셀 값이 같지 않은 경우
    {
        for (int k = 0; k < 256; ++k)
        {
            // cdf[k] - cdf_min : 누적 분포를 0부터 시작
            // N - cdf_min : 전체 픽셀 수에서 최소값을 뺀 범위로 나눠서 [0,1] 구간으로 정규화
            // sk는 intensity k에 대응하는 새로운 픽셀 값
            double sk = (static_cast<double>(cdf[k] - cdf_min) / (N - cdf_min)) * 255.0; // 정규화한 cdf에 255를 곱해 8비트 픽셀 값으로 변경(스케일링)
            if (sk < 0.0)                                                                //  0보다 작을 경우 0으로 설정
                sk = 0.0;
            if (sk > 255.0) // 255보다 클 경우 255로 설정
                sk = 255.0;
            lut[k] = static_cast<unsigned char>(std::lround(sk)); // intesity k에 대한 새로운 픽셀 값 sk를 반올림 후 unsigned char로 변환하여 LUT에 저장
        }
    }

    // lut 적용하여 출력 생성
    Mat dst(rows, cols, CV_8UC1); // 결과 이미지를 저장할 Mat 객체 생성(8비트 단일 채널)
    unsigned char *dData;
    dData = (unsigned char *)dst.data; // 결과 이미지 데이터 포인터

    // 한 행(row)마다 실제 메모리 상에서 차지하는 바이트 수(stride)
    const size_t dstep = dst.step;

    for (int h = 0; h < rows; ++h)
    {
        // 원본 이미지의 h번째 행 시작 주소(sstep: 원본 stride)
        const unsigned char *srow = sdata + h * sstep;

        // 결과 이미지의 h번째 행 시작 주소
        unsigned char *drow = dData + h * dstep;
        for (int w = 0; w < cols; ++w)
        {
            // 원본 픽셀 값(srow[w])를 LUT로 매핑하여 새로운 값으로 변환 후 결과에 저장
            drow[w] = lut[srow[w]];
        }
    }

    // LUT 매핑이 적용된 결과 이미지 반환
    return dst;
}