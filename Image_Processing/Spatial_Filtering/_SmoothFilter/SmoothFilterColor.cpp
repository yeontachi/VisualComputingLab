#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

// 3x3 기준
int wx[8] = {-1, 0, 1, 1, 1, 0, -1, -1}; // 좌상단부터 시계방향
int wy[8] = {-1, -1, -1, 0, 1, 1, 1, 0};

double SmoothFilter[9] = {4, 1, 2, 1, 2, 1, 2, 1, 2}; // 가운데 > 좌상단부터 시계방향, 총합 16

// ---- 채널 수와 타입을 보존하는 제로패딩 (ROI 복사) ----
static Mat zeroPadding(const Mat &src)
{
    Mat padded(src.rows + 2, src.cols + 2, src.type(), Scalar::all(0));
    // (1,1)에 원본 복사
    src.copyTo(padded(Rect(1, 1, src.cols, src.rows)));
    return padded;
}

// ---- 단일 함수: 그레이스케일/컬러(BGR) 공용 ----
Mat Smooth_Filter(const Mat &src)
{
    // 입력은 8비트 1채널 또는 3채널 가정
    CV_Assert(src.type() == CV_8UC1 || src.type() == CV_8UC3);

    Mat PaddedSrc = zeroPadding(src);

    const int channels = PaddedSrc.channels();          // 1 또는 3
    const int pStep = static_cast<int>(PaddedSrc.step); // 바이트 단위 row stride

    const uchar *pData = PaddedSrc.data;

    Mat output(src.rows, src.cols, src.type(), Scalar::all(0));
    const int oStep = static_cast<int>(output.step);
    uchar *outData = output.data;

    // 유효 영역만 순회 (패딩 제외)
    for (int h = 1; h < PaddedSrc.rows - 1; ++h)
    {
        for (int w = 1; w < PaddedSrc.cols - 1; ++w)
        {
            // 채널별 계산 (그레이스케일이면 1번만 돈다)
            for (int c = 0; c < channels; ++c)
            {
                // 중심 픽셀
                double Smooth = pData[h * pStep + w * channels + c] * (SmoothFilter[0] / 16.0);

                // 8방향 누적
                for (int i = 0; i < 8; ++i)
                {
                    int yy = h + wy[i];
                    int xx = w + wx[i];
                    Smooth += pData[yy * pStep + xx * channels + c] * (SmoothFilter[i + 1] / 16.0);
                }

                // 패딩 보정 좌표로 기록
                outData[(h - 1) * oStep + (w - 1) * channels + c] = static_cast<uchar>(Smooth);
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
        cerr << "Image Not Found" << "\n";
        return -1;
    }

    Mat dstImg = Smooth_Filter(src); // 컬러/그레이 공용 스무딩

    imshow("Original", src);
    imshow("Smooth Filter", dstImg);
    waitKey(0);
    return 0;
}
