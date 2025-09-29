#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int wx[4] = {1, -1, 0, 1}; // 오른쪽, 좌하, 하, 우하
int wy[4] = {0, 1, 1, 1};
int Floyd[4] = {7, 3, 5, 1}; // 가중치 분자 (분모는 16)

int main(void)
{
    Mat src = imread("Lena.png", IMREAD_GRAYSCALE);
    if (src.empty())
    {
        cerr << "Error, Image Not Found" << "\n";
        return -1;
    }

    Mat screen = Mat::zeros(src.rows, src.cols, CV_8UC1);

    // 원본을 부동소수 버퍼로 복사 (오차 누적/확산용)
    Mat buf(src.size(), CV_32F);
    for (int h = 0; h < src.rows; ++h)
        for (int w = 0; w < src.cols; ++w)
            buf.at<float>(h, w) = static_cast<float>(src.at<uchar>(h, w));

    unsigned char *sData;
    sData = (unsigned char *)src.data; // (원본 포인터: 그대로 두지만, 실제 연산은 buf 사용)

    unsigned char *dData;
    dData = (unsigned char *)screen.data; // 출력(양자화 결과)

    for (int h = 0; h < src.rows; ++h)
    {
        for (int w = 0; w < src.cols; ++w)
        {
            // 현재 픽셀의 (오차 누적 후) 값
            float oldp = buf.at<float>(h, w);

            // 이진 양자화 (threshold 128 근사)
            float newp = (oldp > 127.5f) ? 255.0f : 0.0f;
            dData[h * src.cols + w] = static_cast<unsigned char>(newp);

            // 양자화 오차 = 원래값 - 양자화값
            float quant_error = oldp - newp; // ★ 정의 완료

            // 오차 확산 (오른쪽/아래 이웃들)
            for (int i = 0; i < 4; ++i)
            {
                int nx = w + wx[i];
                int ny = h + wy[i];
                if (0 <= nx && nx < src.cols && 0 <= ny && ny < src.rows)
                {
                    // 분모 16을 포함한 가중치 적용 (float 연산)
                    buf.at<float>(ny, nx) += (quant_error * (float)Floyd[i] / 16.0f);
                }
            }
        }
    }

    imshow("Floyd Steinberg Dithering", screen);
    waitKey(0);
    return 0;
}
