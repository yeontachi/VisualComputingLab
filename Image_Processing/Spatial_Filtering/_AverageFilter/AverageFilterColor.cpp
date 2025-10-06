#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int wx[8] = {-1, 0, 1, 1, 1, 0, -1, -1}; // 좌상단부터 시계방향
int wy[8] = {-1, -1, -1, 0, 1, 1, 1, 0};

// 타입/채널을 보존하는 제로패딩
static Mat zeroPadding(const Mat &src)
{
    Mat padded(src.rows + 2, src.cols + 2, src.type(), Scalar::all(0));
    src.copyTo(padded(Rect(1, 1, src.cols, src.rows)));
    return padded;
}

// 단일 함수: 8비트 1채널/3채널 공용 평균 필터(3x3)
Mat AverageFilter(const Mat &src)
{
    CV_Assert(src.type() == CV_8UC1 || src.type() == CV_8UC3);

    Mat PaddedSrc = zeroPadding(src);

    const int channels = PaddedSrc.channels();          // 1 또는 3
    const int pStep = static_cast<int>(PaddedSrc.step); // row stride (bytes)
    const uchar *pData = PaddedSrc.ptr<uchar>(0);

    Mat output(src.rows, src.cols, src.type(), Scalar::all(0));
    const int oStep = static_cast<int>(output.step);
    uchar *outData = output.ptr<uchar>(0);

    for (int h = 1; h < PaddedSrc.rows - 1; ++h)
    {
        for (int w = 1; w < PaddedSrc.cols - 1; ++w)
        {
            for (int c = 0; c < channels; ++c)
            {
                int sum = pData[h * pStep + w * channels + c]; // 중심
                for (int i = 0; i < 8; ++i)                    // 8방향
                {
                    int yy = h + wy[i];
                    int xx = w + wx[i];
                    sum += pData[yy * pStep + xx * channels + c];
                }
                int Avg = sum / 9; // 3x3 평균
                outData[(h - 1) * oStep + (w - 1) * channels + c] = static_cast<uchar>(Avg);
            }
        }
    }
    return output;
}

int main(void)
{
    // 컬러/그레이스케일 어느 것이든 가능 (예: 컬러)
    Mat src = imread("Lena.png", IMREAD_COLOR);
    if (src.empty())
    {
        cerr << "Image Not Found\n";
        return -1;
    }

    Mat dstImg = AverageFilter(src);

    imshow("Original", src);
    imshow("AverageFilter", dstImg);
    waitKey(0);
    return 0;
}
