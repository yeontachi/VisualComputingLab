#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

using namespace std;
using namespace cv;

// ======================= 유틸 함수 =======================
static inline double pixelDist(const Vec3b &a, const Vec3b &b)
{
    double db = a[0] - b[0];
    double dg = a[1] - b[1];
    double dr = a[2] - b[2];
    return sqrt(db * db + dg * dg + dr * dr);
}

static inline unsigned char calCenter(unsigned char lo, unsigned char hi)
{
    return static_cast<unsigned char>(((int)lo + (int)hi) / 2);
}

// ======================= 피부색 판별 함수 =======================
static inline bool isFace(const Vec3b &pixel)
{
<<<<<<< HEAD
    // 네가 준 로직 그대로
=======
>>>>>>> main
    static const Vec3b LOW(94, 121, 171);   // B, G, R (하한)
    static const Vec3b HIGH(202, 202, 226); // B, G, R (상한)
    static const Vec3b center(
        calCenter(LOW[0], HIGH[0]),
        calCenter(LOW[1], HIGH[1]),
        calCenter(LOW[2], HIGH[2]));
    static const double radius = pixelDist(LOW, HIGH) / 2;

    return pixelDist(pixel, center) < radius;
}

// ======================= 평균 필터 (박스 필터) =======================
Mat meanFilter(const Mat &src, int ksize)
{
    CV_Assert(src.type() == CV_8UC3);
    Mat dst(src.size(), src.type());

    int radius = ksize / 2;
    unsigned char *pSrc = src.data;
    unsigned char *pDst = dst.data;

    int step = src.step;

    for (int y = 0; y < src.rows; ++y)
    {
        for (int x = 0; x < src.cols; ++x)
        {
            int sumB = 0, sumG = 0, sumR = 0;
            int count = 0;

            for (int dy = -radius; dy <= radius; ++dy)
            {
                for (int dx = -radius; dx <= radius; ++dx)
                {
                    int ny = y + dy;
                    int nx = x + dx;

                    if (ny >= 0 && ny < src.rows && nx >= 0 && nx < src.cols)
                    {
                        unsigned char *pN = pSrc + ny * step + nx * 3;
                        sumB += pN[0];
                        sumG += pN[1];
                        sumR += pN[2];
                        count++;
                    }
                }
            }

            unsigned char *pOut = pDst + y * step + x * 3;
            pOut[0] = static_cast<unsigned char>(sumB / count);
            pOut[1] = static_cast<unsigned char>(sumG / count);
            pOut[2] = static_cast<unsigned char>(sumR / count);
        }
    }

    return dst;
}

// ======================= 메인 =======================
int main()
{
    Mat src = imread("face.jpg"); // 입력 영상
    if (src.empty())
    {
        cerr << "Image not found!" << endl;
        return -1;
    }

    // 크면 축소 (가로 기준 1024)
    if (src.cols > 1024)
    {
        double scale = 1024.0 / src.cols;
        int newH = cvRound(src.rows * scale);
        resize(src, src, Size(1024, newH), 0, 0, INTER_AREA);
    }

    Mat dst = Mat::zeros(src.size(), src.type());  // Color Slicing 결과
    Mat faceMap = Mat::zeros(src.size(), CV_8UC1); // 얼굴 영역 Map (0 or 255)

    // =================== Step #1: Color Slicing ===================
    unsigned char *pSrc = src.data;
    unsigned char *pDst = dst.data;
    unsigned char *pMap = faceMap.data;

    int stepSrc = src.step;
    int stepDst = dst.step;
    int stepMap = faceMap.step;

    for (int y = 0; y < src.rows; ++y)
    {
        unsigned char *rowS = pSrc + y * stepSrc;
        unsigned char *rowD = pDst + y * stepDst;
        unsigned char *rowM = pMap + y * stepMap;

        for (int x = 0; x < src.cols; ++x)
        {
            unsigned char B = rowS[x * 3 + 0];
            unsigned char G = rowS[x * 3 + 1];
            unsigned char R = rowS[x * 3 + 2];
            Vec3b pixel(B, G, R);

            if (isFace(pixel))
            {
                // 얼굴 색일 경우 → 원본 복사
                rowD[x * 3 + 0] = B;
                rowD[x * 3 + 1] = G;
                rowD[x * 3 + 2] = R;
                rowM[x] = 255;
            }
            else
            {
                // 얼굴색이 아닐 경우 → 검정색
                rowD[x * 3 + 0] = 0;
                rowD[x * 3 + 1] = 0;
                rowD[x * 3 + 2] = 0;
                rowM[x] = 0;
            }
        }
    }

    imwrite("face_colorSlicing.png", dst);
    imwrite("face_map.png", faceMap);

    // =================== Step #2: Smoothing + 합성 ===================
    Mat smooth = meanFilter(src, 7); // 얼굴에 적용할 스무딩 필터 (7x7)
    Mat final = src.clone();

    unsigned char *pSmooth = smooth.data;
    unsigned char *pFinal = final.data;
    unsigned char *pMask = faceMap.data;

    int stepSmooth = smooth.step;
    int stepFinal = final.step;

    for (int y = 0; y < src.rows; ++y)
    {
        unsigned char *rowS = pSmooth + y * stepSmooth;
        unsigned char *rowF = pFinal + y * stepFinal;
        unsigned char *rowM = pMask + y * stepMap;

        for (int x = 0; x < src.cols; ++x)
        {
            if (rowM[x] == 255) // 얼굴 영역이면 스무딩 결과 사용
            {
                rowF[x * 3 + 0] = rowS[x * 3 + 0];
                rowF[x * 3 + 1] = rowS[x * 3 + 1];
                rowF[x * 3 + 2] = rowS[x * 3 + 2];
            }
        }
    }

    // =================== 결과 출력 ===================
    imshow("Original", src);
    imshow("Color Slicing", dst);
    imshow("Face Map", faceMap);
    imshow("Smoothed Face", final);

    imwrite("final_beautyShot.png", final);

    waitKey(0);
    return 0;
}
