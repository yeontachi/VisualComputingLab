#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int main(void)
{
    Mat screen;

    screen = Mat::zeros(512, 512, CV_8UC3);

    line(screen, Point(10, 10), Point(200, 50), Scalar(255, 0, 0), 5);

    circle(screen, Point(300, 300), 50, Scalar(0, 0, 255), 5);

    imshow("screen", screen);

    waitKey();

    return 0;
}