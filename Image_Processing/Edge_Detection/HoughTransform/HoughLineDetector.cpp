#include <opencv2/opencv.hpp>
#include <iostream>
using namespace std;
using namespace cv;

static Mat src, grayImg, edges, vis;
static int cannyLow = 50, cannyHigh = 150;
static int rhoTimes10 = 10;  // rho = rhoTimes10/10 (픽셀)
static int thetaDeg = 1;     // theta = thetaDeg * CV_PI / 180
static int houghThresh = 50; // 누적 임계값
static int minLineLen = 50;  // 최소 선 길이(px)
static int maxLineGap = 10;  // 같은 선으로 간주할 최대 간격(px)

static void run(int = 0, void * = nullptr)
{
    // 1) Canny
    Canny(grayImg, edges, cannyLow, cannyHigh);

    // 2) HoughLinesP
    double rho = std::max(1, rhoTimes10) / 10.0; // 0 방지
    double theta = thetaDeg * CV_PI / 180.0;
    vector<Vec4i> lines;
    HoughLinesP(edges, lines, rho, theta, houghThresh, (double)minLineLen, (double)maxLineGap);

    // 3) 시각화
    cvtColor(edges, vis, COLOR_GRAY2BGR);
    for (const auto &l : lines)
        line(vis, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 255, 0), 2, LINE_AA);

    // 4) 정보 표시
    putText(vis, format("Canny=(%d,%d)  rho=%.1f  theta=%ddeg  thr=%d  minLen=%d  maxGap=%d  #lines=%zu", cannyLow, cannyHigh, rho, thetaDeg, houghThresh, minLineLen, maxLineGap, lines.size()),
            Point(10, 25), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 200, 255), 2);

    imshow("edges", edges);
    imshow("HoughLinesP result", vis);
}

int main(int argc, char **argv)
{
    string path = (argc > 1) ? argv[1] : "test.png"; // 이미지 경로
    src = imread(path);
    if (src.empty())
    {
        cerr << "Image Not Found: " << path << "\n";
        return -1;
    }
    cvtColor(src, grayImg, COLOR_BGR2GRAY);
    GaussianBlur(grayImg, grayImg, Size(3, 3), 0.8); // 약간의 블러로 잡음 억제

    namedWindow("HoughLinesP result", WINDOW_AUTOSIZE);
    namedWindow("edges", WINDOW_AUTOSIZE);

    createTrackbar("CannyLow", "HoughLinesP result", &cannyLow, 300, run);
    createTrackbar("CannyHigh", "HoughLinesP result", &cannyHigh, 400, run);
    createTrackbar("rho x0.1", "HoughLinesP result", &rhoTimes10, 50, run); // 0.1~5.0
    createTrackbar("theta deg", "HoughLinesP result", &thetaDeg, 5, run);   // 1~5 deg
    createTrackbar("threshold", "HoughLinesP result", &houghThresh, 300, run);
    createTrackbar("minLineLen", "HoughLinesP result", &minLineLen, 500, run);
    createTrackbar("maxLineGap", "HoughLinesP result", &maxLineGap, 200, run);

    run();
    waitKey(0);
    return 0;
}
