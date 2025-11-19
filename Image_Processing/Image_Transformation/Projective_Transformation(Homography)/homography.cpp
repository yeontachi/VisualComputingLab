#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int main(void)
{
    Mat src = imread("table.png");
    if (src.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }

    Mat dst;

    Point2f src_p[4], dst_p[4];

    src_p[0] = Point2f(434, 508);
    src_p[1] = Point2f(791, 508);
    src_p[2] = Point2f(412, 606);
    src_p[3] = Point2f(825, 606);

    dst_p[0] = Point2f(0, 0);
    dst_p[1] = Point2f(420 - 1, 0);
    dst_p[2] = Point2f(0, 100 - 1);
    dst_p[3] = Point2f(420 - 1, 100 - 1);

    Mat perspective_mat = getPerspectiveTransform(src_p, dst_p);

    warpPerspective(src, dst, perspective_mat, Size(420, 100));

    imshow("src", src);
    imshow("dst", dst);

    waitKey(0);
    return 0;
}