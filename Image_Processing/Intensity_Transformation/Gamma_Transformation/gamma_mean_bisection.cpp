// gamma_mean_bisection.cpp
// - OpenCV는 imread/imshow/waitKey만 사용
// - 감마 추정(이분법)과 적용(LUT)은 직접 구현
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include <vector>
#include <numeric>
#include <algorithm>

using namespace std;
using namespace cv;

static inline double mean01(const Mat &img8u_gray)
{
    CV_Assert(img8u_gray.type() == CV_8UC1);
    const int N = img8u_gray.rows * img8u_gray.cols;
    const uchar *p = img8u_gray.ptr<uchar>(0);
    long long sumv = 0;
    for (int i = 0; i < N; ++i)
        sumv += p[i];
    return (double)sumv / (255.0 * (double)N);
}

static inline double meanAfterGamma(const Mat &img8u_gray, double gamma)
{
    CV_Assert(img8u_gray.type() == CV_8UC1);
    const int N = img8u_gray.rows * img8u_gray.cols;
    const uchar *p = img8u_gray.ptr<uchar>(0);
    double acc = 0.0;
    for (int i = 0; i < N; ++i)
    {
        double x = (double)p[i] / 255.0; // [0,1]
        // x==0이면 pow(0,gamma)=0 그대로
        acc += std::pow(x, gamma);
    }
    return acc / (double)N;
}

// 평균 일치(mean match) 이분법
static double findGammaByMeanMatch(const Mat &tgt_gray, const Mat &ref_gray,
                                   double gL = 0.10, double gR = 5.00,
                                   double tol = 1e-7, int maxit = 60)
{
    const double targetMean = std::clamp(mean01(ref_gray), 1e-9, 1.0 - 1e-9);

    auto f = [&](double g)
    { return meanAfterGamma(tgt_gray, g) - targetMean; };

    double left = gL, right = gR;
    double fL = f(left), fR = f(right);

    // 범위 밖이면 확장 시도
    if (fL < 0.0)
    {
        for (int k = 0; k < 10 && fL < 0.0; ++k)
        {
            right = left;
            left = max(1e-4, left * 0.5);
            fL = f(left);
        }
    }
    if (fR > 0.0)
    {
        for (int k = 0; k < 10 && fR > 0.0; ++k)
        {
            left = right;
            right = min(50.0, right * 2.0);
            fR = f(right);
        }
    }
    // 여전히 부호 교차가 없으면 중간값 반환(근사)
    if (!(fL > 0.0 && fR < 0.0))
        return 0.5 * (left + right);

    for (int it = 0; it < maxit; ++it)
    {
        double mid = 0.5 * (left + right);
        double fm = f(mid);
        if (std::abs(fm) < tol)
            return mid;
        // f는 gamma에 대해 감소 → fL>0, fR<0 유지되도록 갱신
        if (fm > 0.0)
            left = mid; // 아직 밝음 → gamma ↑
        else
            right = mid; // 아직 어두움 → gamma ↓
    }
    return 0.5 * (left + right);
}

static void buildGammaLUT(array<uchar, 256> &LUT, double gamma)
{
    for (int v = 0; v < 256; ++v)
    {
        double x = (double)v / 255.0;
        int out = (int)std::lround(255.0 * std::pow(x, gamma));
        LUT[v] = (uchar)std::clamp(out, 0, 255);
    }
}

static Mat applyGammaLUT(const Mat &src8u_any, const array<uchar, 256> &LUT)
{
    CV_Assert(src8u_any.type() == CV_8UC1 || src8u_any.type() == CV_8UC3);
    Mat dst = src8u_any.clone();
    const int N = dst.rows * dst.cols * dst.channels();
    uchar *p = dst.ptr<uchar>(0);
    for (int i = 0; i < N; ++i)
        p[i] = LUT[p[i]];
    return dst;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cerr << "Usage: " << argv[0] << " <target_image> <reference_image>\n";
        return 1;
    }

    // I/O만 OpenCV 사용 (요구사항)
    Mat target_gray = imread(argv[1], IMREAD_GRAYSCALE);
    Mat refer_gray = imread(argv[2], IMREAD_GRAYSCALE);
    if (target_gray.empty() || refer_gray.empty())
    {
        cerr << "Failed to read images.\n";
        return 1;
    }

    // 감마 추정(평균 일치, 이분법)
    double gamma = findGammaByMeanMatch(target_gray, refer_gray, 0.10, 5.00, 1e-7, 60);

    // 적용용 LUT 만들고, 원본(컬러 보기 원하면 별도 로드)에도 적용 가능
    array<uchar, 256> LUT;
    buildGammaLUT(LUT, gamma);

    // 결과 영상(그레이스케일) 생성
    Mat corrected_gray = applyGammaLUT(target_gray, LUT);

    // 평균 확인 로그
    cout.setf(ios::fixed);
    cout << setprecision(6);
    cout << "[INFO] mean(target)    = " << mean01(target_gray) << "\n";
    cout << "[INFO] mean(reference) = " << mean01(refer_gray) << "\n";
    cout << "[INFO] gamma(found)    = " << gamma << "\n";
    cout << "[INFO] mean(corrected) = " << mean01(corrected_gray) << "\n";

    // 표시
    imshow("Target (Gray)", target_gray);
    imshow("Reference (Gray)", refer_gray);
    imshow("Corrected (Gray)", corrected_gray);
    waitKey(0);
    return 0;
}
