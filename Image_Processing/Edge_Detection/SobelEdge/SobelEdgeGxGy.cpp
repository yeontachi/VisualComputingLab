#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

// x방향 기울기 변화 감지
vector<float> SobelEdgeKernel_gx()
{
    return {
        -1, 0, 1,
        -2, 0, 2,
        -1, 0, 1};
}

// y방향 기울기 변화 감지
vector<float> SobelEdgeKernel_gy()
{
    return {
        -1, -2, -1,
        0, 0, 0,
        1, 2, 1};
}

Mat Filtering_gray(const Mat &src, const vector<float> &kernel, int n)
{
    CV_Assert(src.type() == CV_8UC1);

    // int kSize = 2 * n + 1;

    Mat P; // 패딩
    copyMakeBorder(src, P, 1, 1, 1, 1, BORDER_REPLICATE);

    Mat dst = Mat::zeros(src.size(), CV_8UC1);

    unsigned char *pData;
    pData = (unsigned char *)P.data;

    const size_t pstep = P.step;

    unsigned char *dData;
    dData = (unsigned char *)dst.data;

    const size_t dstep = dst.step;

    for (int h = n; h < P.rows - n; ++h)
    {
        for (int w = n; w < P.cols - n; ++w)
        {
            float sum = 0.0f;
            int k = 0;
            for (int y = -n; y <= n; ++y)
            {
                for (int x = -n; x <= n; ++x)
                {
                    sum += pData[(h + y) * pstep + (w + x)] * kernel[k++];
                }
            }
            uchar result;
            if (sum < 0)
                result = 0;
            else if (sum > 255)
                result = 255;
            else
                result = static_cast<uchar>(sum);

            dData[(h - n) * dstep + (w - n)] = result;
        }
    }
    return dst;
}

int main(void)
{
    Mat src = imread("Lena.png", IMREAD_GRAYSCALE);
    if (src.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }

    vector<float> gxKernel;

    gxKernel = SobelEdgeKernel_gx();

    // |gx|
    Mat dstGX = Filtering_gray(src, gxKernel, 1);

    vector<float> gyKernel;

    gyKernel = SobelEdgeKernel_gy();

    // |gy|
    Mat dstGY = Filtering_gray(src, gyKernel, 1);

    imshow("Original", src);
    imshow("|gx| display", dstGX);
    imshow("|gy| display", dstGY);

    imwrite("gx.png", dstGX);
    imwrite("gy.png", dstGY);

    waitKey(0);

    return 0;
}