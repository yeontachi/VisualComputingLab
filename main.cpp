#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

using namespace std;
using namespace cv;

Mat getGaussianFilter()
{
    int ksize = 5;
    float sigma = 1.0;
    Mat g = getGaussianKernel(ksize, sigma, CV_64F);
    return g * g.t();
}
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

// �Ǻλ�
static inline bool isFace(const Vec3b &pixel)
{
    static const Vec3b LOW(50, 130, 180);   // B, G, R (����)
    static const Vec3b HIGH(170, 190, 230); // B, G, R (����)
    static const Vec3b center(
        calCenter(LOW[0], HIGH[0]),
        calCenter(LOW[1], HIGH[1]),
        calCenter(LOW[2], HIGH[2]));
    static const double radius = pixelDist(LOW, HIGH) / 2;

    return pixelDist(pixel, center) < radius;
}

Mat GaussianFilter(const Mat &src, int ksize)
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

int main()
{
    Mat src = imread("image.jpg"); // �Է� ����
    if (src.empty())
    {
        cerr << "Image not found!" << endl;
        return -1;
    }

    // ũ�� ��� (���� ���� 1024)
    if (src.cols > 1024)
    {
        double scale = 1024.0 / src.cols;
        int newH = cvRound(src.rows * scale);
        resize(src, src, Size(1024, newH), 0, 0, INTER_AREA);
    }

    Mat dst = Mat::zeros(src.size(), src.type()); // Color Slicing ���
    Mat faceMap = Mat::zeros(src.size(), CV_8UC1);

    // Color Slicing
    unsigned char *pSrc = src.data;
    unsigned char *pDst = dst.data;
    unsigned char *pMap = faceMap.data;

    int stepSrc = src.step;
    int stepDst = dst.step;
    int stepMap = faceMap.step;

    Mat Bb(src.rows, src.cols, CV_8UC1);
    Mat Gg(src.rows, src.cols, CV_8UC1);
    Mat Rr(src.rows, src.cols, CV_8UC1);

    unsigned char *Bdata = Bb.data;
    unsigned char *Gdata = Gg.data;
    unsigned char *Rdata = Rr.data;

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
                // �� ���� ��� �� ���� ����
                rowD[x * 3 + 0] = B;
                rowD[x * 3 + 1] = G;
                rowD[x * 3 + 2] = R;
                rowM[x] = 255;
                Bdata[y * src.cols + x] = B;
                Gdata[y * src.cols + x] = G;
                Rdata[y * src.cols + x] = R;
            }
            else
            {
                // �󱼻��� �ƴ� ��� �� ������
                rowD[x * 3 + 0] = 0;
                rowD[x * 3 + 1] = 0;
                rowD[x * 3 + 2] = 0;
                rowM[x] = 0;
                Bdata[y * src.cols + x] = 0;
                Gdata[y * src.cols + x] = 0;
                Rdata[y * src.cols + x] = 0;
            }
        }
    }
    imshow("B", Bb);
    imshow("G", Gg);
    imshow("R", Rr);

    cv::imwrite("colorsliced.png", dst);
    cv::imwrite("mask.png", faceMap);

    // ������,�ռ�
    Mat smooth = GaussianFilter(src, 5);
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
            if (rowM[x] == 255) // �� �����̸� ������ ��� ���
            {
                rowF[x * 3 + 0] = rowS[x * 3 + 0];
                rowF[x * 3 + 1] = rowS[x * 3 + 1];
                rowF[x * 3 + 2] = rowS[x * 3 + 2];
            }
        }
    }

    imshow("Original", src);
    imshow("Color Slicing", dst);
    imshow("Face Map", faceMap);
    imshow("Smoothed Face", final);

    imwrite("mask.png", facemap);

    waitKey(0);
    return 0;
}