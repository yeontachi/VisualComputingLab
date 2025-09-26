#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

void onMouse(int event, int x, int y, int flags, void *userdata)
{
    if (event == EVENT_LBUTTONDOWN)
    {
        cout << "L button down!" << endl;
    }
    else if (event == EVENT_MOUSEMOVE)
    {
        cout << "Mouse Move!" << endl;
    }
    else if (event == EVENT_LBUTTONUP)
    {
        cout << "L button up!" << endl;
    }
}

int main(void)
{
    Mat screen;
    screen = Mat::zeros(512, 512, CV_8UC3);

    imshow("screen", screen);

    setMouseCallback("screen", onMouse, &screen);

    waitKey(0);
}