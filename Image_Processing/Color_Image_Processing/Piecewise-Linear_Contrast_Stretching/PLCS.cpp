#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

// 마우스 움직일 때 움직이는 것을 이벤트로 받을 수 있음
// 마우스를 움직여 Contrast Stretching 함수 그려서 해보기
// ================= 전역 상태 =====================
static Mat screen;                // Linear Contrast Stretch 그리기
static const int Stretch_w = 256; // 캔버스 폭(intput)
static const int Stretch_h = 256; // 캔버스 높이(output)
static Point pt1(1, -1);
static bool drawing = false;

// Constrast Stretching Array
static uchar outputIntensity[256];

static void addSegmentToConstrastStretching(const Point &A, const Point &B)
{
    Point p0 = A, p1 = B;
    if (p1.x < p0.x)
    {
        std::swap(p0, p1);
    }

    int dx = p1.x - p0.x;
    if (dx == 0)
    {
        cerr << "Divied with Zero" << "\n";
        return;
    }
    double y0 = 255.0 - p0.y;
    double y1 = 255.0 - p1.y;

    double gradient = (y1 - y0) / (double)dx; // 기울기

    int xStart = max(0, p0.x);
    int xEnd = min(255, p1.x);

    for (int i = xStart; i <= xEnd; ++i)
    {
        double y = y0 + gradient * (i - p0.x);
        int S = cvRound(y);
        if (S > 255)
            S = 255;
        else if (S < 0)
            S = 0;
        outputIntensity[i] = S;
    }
}

// 마우스 콜백
static void onMouse(int event, int x, int y, int, void *userdata)
{
    Mat *pSrcColor = (Mat *)userdata; // 원본 그레이스케일

    if (event == EVENT_LBUTTONDOWN)
    {
        pt1 = Point(x, y);
        drawing = true;
    }
    else if (event == EVENT_MOUSEMOVE && drawing)
    {
        Mat tmp = screen.clone();
        line(tmp, pt1, Point(x, y), Scalar(0, 0, 255), 2);
        imshow("screen", tmp);
    }
    else if (event == EVENT_LBUTTONUP)
    {
        Point pt2(x, y);
        line(screen, pt1, pt2, Scalar(0, 0, 255), 2);
        imshow("screen", screen);
        drawing = false;

        addSegmentToConstrastStretching(pt1, pt2);

        uchar lutData[256];
        for (int i = 0; i < 256; ++i)
        {
            int v = outputIntensity[i];
            if (v < 0)
                v = 0;
            if (v > 255)
                v = 255;
            lutData[i] = (uchar)v;
        }
        Mat lut(1, 256, CV_8UC1, lutData);

        Mat dst;
        LUT(*pSrcColor, lut, dst);
        imshow("Linear Constrast Stretching", dst);
    }
}

int main(void)
{
    Mat src = imread("Lena.png"); // Color Image(default)
    if (src.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }

    for (int i = 0; i < 256; ++i)
    {
        outputIntensity[i] = 0;
    }

    screen = Mat::zeros(Stretch_h, Stretch_w, CV_8UC3);

    line(screen, Point(0, 255), Point(255, 0), Scalar(0, 255, 0), 1);

    namedWindow("screen", WINDOW_AUTOSIZE);
    setMouseCallback("screen", onMouse, &src);
    imshow("screen", screen);

    imshow("Linear Constrast Stretching", src);

    waitKey(0);
    return 0;
}