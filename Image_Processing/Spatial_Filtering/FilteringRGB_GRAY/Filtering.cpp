#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;
using namespace cv;

// Zero Padding 함수
Mat ZeroPadding(const Mat &src, int n)
{
    CV_Assert(src.type() == CV_8UC1);
    int pad = n;

    Mat padded = Mat::zeros(src.rows + 2 * pad, src.cols + 2 * pad, CV_8UC1);

    for (int h = 0; h < src.rows; ++h)
    {
        for (int w = 0; w < src.cols; ++w)
        {
            padded.at<uchar>(h + pad, w + pad) = src.at<uchar>(h, w);
        }
    }
    return padded;
}

// 단일 채널 필터링 함수
Mat FilteringGray(const Mat &src, const vector<float> &kernel, int n)
{
    CV_Assert(src.type() == CV_8UC1);
    int kSize = 2 * n + 1;
    Mat P = ZeroPadding(src, n);
    Mat dst = Mat::zeros(src.size(), CV_8UC1);

    const size_t pstep = P.step;
    const size_t dstep = dst.step;
    const uchar *pData = P.data;
    uchar *dData = dst.data;

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

// 컬러 영상 필터링 함수 (BGR 각 채널에 적용)
Mat FilteringColor(const Mat &src, const vector<float> &kernel, int n)
{
    CV_Assert(src.type() == CV_8UC3);

    vector<Mat> ch(3);
    split(src, ch);

    for (int i = 0; i < 3; ++i)
    {
        ch[i] = FilteringGray(ch[i], kernel, n);
    }

    Mat dst;
    merge(ch, dst);
    return dst;
}

// 평균 필터 커널 생성
vector<float> MakeAverageKernel(int n)
{
    int size = (2 * n + 1) * (2 * n + 1);
    vector<float> kernel(size, 1.0f / size);
    return kernel;
}

// 라플라시안 (3x3) 커널
vector<float> MakeLaplacianKernel()
{
    return {
        0, -1, 0,
        -1, 4, -1,
        0, -1, 0};
}

// 샤프닝 (3x3, 라플라시안 기반)
vector<float> MakeSharpenKernel()
{
    return {
        0, -1, 0,
        -1, 5, -1,
        0, -1, 0};
}

int main()
{
    Mat src = imread("Lena.png", IMREAD_COLOR);
    if (src.empty())
    {
        cerr << "Image Not Found\n";
        return -1;
    }

    cout << "Filter Type (avg/laplacian/sharpen): ";
    string filterType;
    cin >> filterType;

    vector<float> kernel;
    int n = 1; // 기본 필터 크기 n=1 → 3x3

    if (filterType == "avg")
    {
        cout << "Enter n (e.g., 1→3x3, 2→5x5, 3→7x7): ";
        cin >> n;
        kernel = MakeAverageKernel(n);
    }
    else if (filterType == "laplacian")
    {
        n = 1;
        kernel = MakeLaplacianKernel();
    }
    else if (filterType == "sharpen")
    {
        n = 1;
        kernel = MakeSharpenKernel();
    }
    else
    {
        cerr << "Unknown filter type.\n";
        return -1;
    }

    // Gray 변환
    Mat gray;
    cvtColor(src, gray, COLOR_BGR2GRAY);

    // Filtering
    Mat dstGray = FilteringGray(gray, kernel, n);
    Mat dstColor = FilteringColor(src, kernel, n);

    // 결과 출력
    imshow("Original Gray", gray);
    imshow("Filtered Gray", dstGray);
    imshow("Original Color", src);
    imshow("Filtered Color", dstColor);

    waitKey(0);
    return 0;
}
