#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int main(void)
{
    Mat img = imread("Lena.png", IMREAD_GRAYSCALE);

    if (img.empty())
    {
        cerr << "Error, Image Not Found" << "\n";
        return -1;
    }

    imshow("Original", img);

    waitKey(0);
    return 0;
}
