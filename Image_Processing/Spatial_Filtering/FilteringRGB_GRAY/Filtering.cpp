#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;
using namespace cv;

// 채널 Split(컬러 영상을 Split해 단일 채널 함수에 적용)
void SplitChannel(const Mat &src, Mat &B, Mat &G, Mat &R)
{
    CV_Assert(src.type() == CV_8UC3);

    int rows = src.rows;
    int cols = src.cols;

    B.create(rows, cols, CV_8UC1);
    G.create(rows, cols, CV_8UC1);
    R.create(rows, cols, CV_8UC1);

    const uchar *srcData = src.data;
    uchar *bData = B.data;
    uchar *gData = G.data;
    uchar *rData = R.data;

    size_t sstep = src.step; // 한 행의 바이트 수 (3 * cols)
    size_t bstep = B.step;   // 단일채널 행 바이트 수

    for (int h = 0; h < rows; ++h)
    {
        const uchar *srcRow = srcData + h * sstep;
        uchar *bRow = bData + h * bstep;
        uchar *gRow = gData + h * bstep;
        uchar *rRow = rData + h * bstep;

        for (int w = 0; w < cols; ++w)
        {
            bRow[w] = srcRow[w * 3 + 0]; // Blue
            gRow[w] = srcRow[w * 3 + 1]; // Green
            rRow[w] = srcRow[w * 3 + 2]; // Red
        }
    }
}

// 채널 merge(Split된 B,G,R 채널을 다시 Merge)
Mat MergeChannel(const Mat &B, const Mat &G, const Mat &R)
{
    CV_Assert(B.type() == CV_8UC1 && G.type() == CV_8UC1 && R.type() == CV_8UC1);
    CV_Assert(B.size() == G.size() && B.size() == R.size());

    int rows = B.rows;
    int cols = B.cols;

    Mat dst(rows, cols, CV_8UC3);
    uchar *dstData = dst.data;

    const uchar *bData = B.data;
    const uchar *gData = G.data;
    const uchar *rData = R.data;

    size_t dstep = dst.step;
    size_t bstep = B.step;

    for (int h = 0; h < rows; ++h)
    {
        const uchar *bRow = bData + h * bstep;
        const uchar *gRow = gData + h * bstep;
        const uchar *rRow = rData + h * bstep;

        uchar *dstRow = dstData + h * dstep;

        for (int w = 0; w < cols; ++w)
        {
            dstRow[w * 3 + 0] = bRow[w]; // Blue
            dstRow[w * 3 + 1] = gRow[w]; // Green
            dstRow[w * 3 + 2] = rRow[w]; // Red
        }
    }

    return dst;
}

// Zero Padding 함수
// 단일 채널에 대해 적용하며, 이후 Color Image Filtering에서 각 채널별로 적용
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
// 컬러 영상의 경우 B,G,R 채널 순으로 각각 필터링 적용
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
// 컬러영상을 Split한 후, 채널 별로 필터링 적용 후 다시 Merge
// 이전에 구현한 Split, Merge 함수를 그대로 이용
Mat FilteringColor(const Mat &src, const vector<float> &kernel, int n)
{
    CV_Assert(src.type() == CV_8UC3);

    Mat B, G, R;
    SplitChannel(src, B, G, R); // 포인터 기반 Split

    B = FilteringGray(B, kernel, n);
    G = FilteringGray(G, kernel, n);
    R = FilteringGray(R, kernel, n);

    Mat dst = MergeChannel(B, G, R); // 포인터 기반 Merge5
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

    cout << "Filter Type (avg/laplacian/sharpen): "; // 필터 타입 입력받기
    string filterType;
    cin >> filterType;

    vector<float> kernel;
    int n = 1; // 기본 필터 크기 n=1 -> 3x3

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
