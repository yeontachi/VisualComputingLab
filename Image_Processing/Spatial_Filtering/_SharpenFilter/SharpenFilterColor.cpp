#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

// 3x3 기준
int wx[8] = {-1, 0, 1, 1, 1, 0, -1, -1}; // 좌상단부터 시계방향
int wy[8] = {-1, -1, -1, 0, 1, 1, 1, 0};

int SharpenFilter[9] = {5, 0, -1, 0, -1, 0, -1, 0, -1}; // 중심 > 좌상단부터 시계방향

// 타입/채널을 보존하는 제로 패딩
static Mat zeroPadding(const Mat &src)
{
    Mat padded(src.rows + 2, src.cols + 2, src.type(), Scalar::all(0));
    src.copyTo(padded(Rect(1, 1, src.cols, src.rows)));
    return padded;
}

// 단일 함수: 8비트 1채널/3채널 공용 샤픈 필터
Mat Sharpen_Filter(const Mat &src)
{
    CV_Assert(src.type() == CV_8UC1 || src.type() == CV_8UC3);

    Mat PaddedSrc = zeroPadding(src);

    const int channels = PaddedSrc.channels();          // 1 또는 3
    const int pStep = static_cast<int>(PaddedSrc.step); // 바이트 단위 stride
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
                int acc = pData[h * pStep + w * channels + c] * SharpenFilter[0];

                for (int i = 0; i < 8; ++i)
                {
                    int yy = h + wy[i];
                    int xx = w + wx[i];
                    acc += pData[yy * pStep + xx * channels + c] * SharpenFilter[i + 1];
                }

                outData[(h - 1) * oStep + (w - 1) * channels + c] = saturate_cast<uchar>(acc);
            }
        }
    }
    return output;
}

int main(void)
{
    // 컬러로 읽어도 되고, 그레이로 읽어도 자동 처리됨
    Mat src = imread("Lena.png", IMREAD_COLOR);
    if (src.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }

    Mat dstImg = Sharpen_Filter(src);

    imshow("Original", src);
    imshow("Sharpen Filter", dstImg);
    waitKey(0);
    return 0;
}
