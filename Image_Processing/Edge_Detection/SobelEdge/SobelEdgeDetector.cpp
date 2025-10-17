#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int wx[8] = {-1, 0, 1, 1, 1, 0, -1, -1};
int wy[8] = {-1, -1, -1, 0, 1, 1, 1, 0};

int Horizontal[9] = {0, -1, 0, 1, 2, 1, 0, -1, -2};
int Vertical[9] = {0, -1, -2, -1, 0, 1, 2, 1, 0};

static void HorizontalEdgeD(Mat &src) {}

static void VericalEdgeD(Mat &src) {}

static Mat SobeledgeD(Mat &src) {}

int main(void)
{
    Mat src = imread("Lena.png", IMREAD_COLOR);
    if (src.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }
    imshow("Original", src);

    Mat Sobeled;
    Sobeled = SobeledgeD(src);

    imshow("Sobel Edge Detector", Sobeled);

    waitKey(0);

    return 0;
}