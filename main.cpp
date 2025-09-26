#include <opencv2/opencv.hpp>
using namespace cv;

static Mat screen;
static Point pt1;
static bool drawing = false;

void onMouse(int evt, int x, int y, int flags, void *userdata)
{
    if (evt == EVENT_LBUTTONDOWN)
    {
        pt1 = Point(x, y);
        drawing = true;
    }
    else if (evt == EVENT_MOUSEMOVE && drawing)
    {
        Mat temp = screen.clone();
        line(temp, pt1, Point(x, y), Scalar(255, 0, 0), 2);
        imshow("screen", temp);
    }
    else if (evt == EVENT_LBUTTONUP)
    {
        line(screen, pt1, Point(x, y), Scalar(255, 0, 0), 2);
        drawing = false;
        imshow("screen", screen);
    }
    else if (evt == EVENT_RBUTTONDOWN)
    {
        circle(screen, Point(x, y), 30, Scalar(0, 0, 255), 2);
        imshow("screen", screen);
    }
}

int main()
{
    screen = Mat::zeros(512, 512, CV_8UC3);
    namedWindow("screen");
    setMouseCallback("screen", onMouse, nullptr);
    imshow("screen", screen);
    waitKey(0);
    return 0;
}
