#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;
using namespace cv;

static inline Mat IntensityLevelSlicing(const Mat &ch, int Low, int High)
{
    Low = std::clamp(Low, 0, 255);
    High = std::clamp(High, 0, 255);

    if (Low > High)
    {
        std::swap(Low, High);
    }

    Mat out = ch.clone();

    for (int h = 0; h < ch.rows; ++h)
    {
        const unsigned char *cData = ch.ptr<uchar>(h);
        unsigned char *oData = out.ptr<uchar>(h);
        for (int w = 0; w < ch.cols; ++w)
        {
            uchar v = cData[w];
            if (v < Low || v > High)
            {
                oData[w] = 0;
            }
        }
    }
    return out;
}

static inline Mat ColorSlicing(const Mat &src)
{
    int R_low = 165, R_high = 223;
    int G_low = 118, G_high = 223;
    int B_low = 100, B_high = 223;

    vector<Mat> ch(3);
    split(src, ch);

    Mat b, g, r;

    b = IntensityLevelSlicing(ch[0], B_low, B_high);
    g = IntensityLevelSlicing(ch[1], G_low, G_high);
    r = IntensityLevelSlicing(ch[2], R_low, R_high);

    for (int h = 0; h < src.rows; ++h)
    {
        unsigned char *bData = b.ptr<uchar>(h);
        unsigned char *gData = g.ptr<uchar>(h);
        unsigned char *rData = r.ptr<uchar>(h);
        for (int w = 0; w < src.cols; ++w)
        {
            if (bData[w] == 0 || gData[w] == 0 || rData[w] == 0)
            {
                bData[w] = 0;
                gData[w] = 0;
                rData[w] = 0;
            }
        }
    }

    Mat merged;
    merge(vector<Mat>{b, g, r}, merged);
    return merged;
}

// 1) 얼굴 여부 판별: ColorSlicing에 사용한 범위를 그대로 사용 (간단/일관성 유지)
static inline bool isFace(const Vec3b &pix)
{
    // BGR 순서 주의
    const int B = pix[0], G = pix[1], R = pix[2];
    // ColorSlicing에서 사용한 동일 범위
    return (R >= 165 && R <= 223) &&
           (G >= 118 && G <= 223) &&
           (B >= 100 && B <= 223);
}

// 2) 얼굴 마스크 생성 (0/255)
static inline Mat BuildFaceMask(const Mat &src)
{
    CV_Assert(src.type() == CV_8UC3);
    Mat mask(src.rows, src.cols, CV_8U);

    for (int h = 0; h < src.rows; ++h)
    {
        const Vec3b *sData = src.ptr<Vec3b>(h);
        uchar *mData = mask.ptr<uchar>(h);
        for (int w = 0; w < src.cols; ++w)
        {
            mData[w] = isFace(sData[w]) ? 255 : 0;
        }
    }
    return mask;
}

// 3) 단순 스무딩 함수 (요청: 정규화합성/모폴로지/페더링 없이)
//    기존 main의 호출부를 바꾸지 않기 위해 이름을 ColorAverageMasked로 둠.
static inline Mat ColorAverageMasked(const Mat &img, int ksize)
{
    CV_Assert(img.type() == CV_8UC3);
    if (ksize % 2 == 0)
        ksize += 1; // 홀수 보정
    Mat out;
    // 간단히 GaussianBlur 사용 (원하면 blur로 교체 가능)
    GaussianBlur(img, out, Size(ksize, ksize), 0.0);
    return out;
}

int main(void)
{
    Mat src = imread("FaceImage.png");

    if (src.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }

    Mat dst(src.rows, src.cols, CV_8UC3);

    dst = ColorSlicing(src);

    imshow("Original", src);
    imshow("ColorSliced Image", dst);

    // 전체 영상에 스무딩 실행
    Mat smoothed(src.rows, src.cols, CV_8UC3);

    smoothed = ColorAverageMasked(dst, 9);

    Mat faceMask = BuildFaceMask(src);

    // 합성: 얼굴(마스크=255)에는 smoothed, 그외에는 원본 src
    Mat result = src.clone();
    smoothed.copyTo(result, faceMask); // mask=255 영역만 복사

    imshow("Smooth", smoothed);
    imshow("facemask", faceMask);
    imshow("Beauty Shot", result);

    waitKey(0);
}