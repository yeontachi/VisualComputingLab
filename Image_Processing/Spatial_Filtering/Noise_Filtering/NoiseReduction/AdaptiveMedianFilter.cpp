#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cstring>

using namespace std;
using namespace cv;

static inline unsigned char clamp8(int v)
{
    return (unsigned char)std::max(0, std::min(255, v));
}

// ===== 경계 복제 패딩(Replicate) =====
static Mat padReplicateGray(const Mat &src, int pad)
{
    CV_Assert(src.type() == CV_8UC1);
    const int H = src.rows, W = src.cols;
    Mat dst(H + 2 * pad, W + 2 * pad, CV_8UC1);

    // 가운데 원본 복사
    for (int y = 0; y < H; ++y)
    {
        const uchar *s = src.ptr<uchar>(y);
        uchar *d = dst.ptr<uchar>(y + pad) + pad;
        std::memcpy(d, s, W * sizeof(uchar));
    }
    // 좌우 복제
    for (int y = 0; y < H; ++y)
    {
        const uchar *srow = src.ptr<uchar>(y);
        uchar *drow = dst.ptr<uchar>(y + pad);
        for (int x = 0; x < pad; ++x)
            drow[x] = srow[0];
        for (int x = 0; x < pad; ++x)
            drow[pad + W + x] = srow[W - 1];
    }
    // 위/아래 복제
    const int fullW = W + 2 * pad;
    const uchar *first = dst.ptr<uchar>(pad);
    const uchar *last = dst.ptr<uchar>(pad + H - 1);
    for (int y = 0; y < pad; ++y)
    {
        std::memcpy(dst.ptr<uchar>(y), first, fullW);
        std::memcpy(dst.ptr<uchar>(pad + H + y), last, fullW);
    }
    return dst;
}

// ===== 윈도우의 min/max/median 계산(8비트) =====
static inline void window_stats(const Mat &pad, int cy, int cx, int k,
                                uchar &zmin, uchar &zmax, uchar &zmed)
{
    const int r = k / 2;
    vector<uchar> win;
    win.reserve(k * k);
    for (int j = -r; j <= r; ++j)
    {
        const uchar *row = pad.ptr<uchar>(cy + j);
        for (int i = -r; i <= r; ++i)
            win.push_back(row[cx + i]);
    }
    // min/max
    auto mm = minmax_element(win.begin(), win.end());
    zmin = *mm.first;
    zmax = *mm.second;

    // median (정렬 또는 nth_element)
    const size_t mid = win.size() / 2;
    nth_element(win.begin(), win.begin() + mid, win.end());
    zmed = win[mid];
}

// ===== Adaptive Median Filter =====
// Smax: 최대 커널 크기(예: 7, 9, 11 등 홀수)
Mat AdaptiveMedianFilter(const Mat &src, int Smax, int Sinit = 3)
{
    if (src.empty())
        throw runtime_error("empty image");
    if (src.type() != CV_8UC1)
        throw runtime_error("use CV_8UC1 grayscale");
    if ((Smax % 2) == 0 || Smax < 3)
        throw runtime_error("Smax must be odd >=3");
    if ((Sinit % 2) == 0 || Sinit < 3 || Sinit > Smax)
        throw runtime_error("Sinit odd and 3<=Sinit<=Smax");

    // 패딩은 최대 반경으로 한 번만
    const int pad = Smax / 2;
    Mat padImg = padReplicateGray(src, pad);
    Mat dst(src.rows, src.cols, CV_8UC1);

    for (int y = 0; y < src.rows; ++y)
    {
        uchar *drow = dst.ptr<uchar>(y);
        for (int x = 0; x < src.cols; ++x)
        {
            const int py = y + pad; // 패딩 좌표
            const int px = x + pad;

            int S = Sinit;
            uchar out = 0;

            while (true)
            {
                uchar Zmin, Zmax, Zmed;
                window_stats(padImg, py, px, S, Zmin, Zmax, Zmed);

                // Level A
                int A1 = int(Zmed) - int(Zmin);
                int A2 = int(Zmed) - int(Zmax);
                if (A1 > 0 && A2 < 0)
                {
                    // Level B
                    uchar Zxy = src.at<uchar>(y, x);
                    int B1 = int(Zxy) - int(Zmin);
                    int B2 = int(Zxy) - int(Zmax);
                    out = (B1 > 0 && B2 < 0) ? Zxy : Zmed;
                    break;
                }
                else
                {
                    S += 2; // 윈도우 크기 증가(홀수 유지)
                    if (S <= Smax)
                    {
                        continue; // Level A 반복
                    }
                    else
                    {
                        // 더 키울 수 없으면 원본 반환
                        out = src.at<uchar>(y, x);
                        break;
                    }
                }
            }
            drow[x] = out;
        }
    }
    return dst;
}

int main(void)
{
    Mat GN = imread("lena256_GN.png", IMREAD_GRAYSCALE); // 가우시안 노이즈 이미지
    if (GN.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }

    Mat SPN = imread("lena256SPN.png", IMREAD_GRAYSCALE); // 소금후추 노이즈 이미지
    if (SPN.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }

    int Smax = 7;  // 최대 커널
    int Sinit = 3; // 초기 커널
    cout << "최대 커널 사이즈: ";
    cin >> Smax;
    cout << "초기 커널 사이즈: ";
    cin >> Sinit;

    Mat GNout = AdaptiveMedianFilter(GN, Smax, Sinit);
    Mat SPNout = AdaptiveMedianFilter(SPN, Smax, Sinit);

    imshow("Gaussian Noise", GN);
    imshow("Salt and Pepper Noise", SPN);

    imshow("GN_AdaptiveMedianFilter", GNout);
    imshow("SPN_AdaptiveMedianFilter", SPNout);

    waitKey(0);

    return 0;
}