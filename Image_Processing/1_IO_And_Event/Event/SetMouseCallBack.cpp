#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

void onMouse(int event, int x, int y, int flags, void *userdata)
{
    cout << "position" << x << "," << y << endl;
}

int main(void)
{
    Mat screen;
    screen = Mat::zeros(512, 512, CV_8UC3);
    imshow("screen", screen);

    setMouseCallback("screen", onMouse, &screen);

    waitKey();
    return 0;
}