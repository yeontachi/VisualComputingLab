#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#define PI 3.141592265358979323846

using namespace std;
using namespace cv;

int main(void)
{
    Mat src = imread("Lena.png");
    if (src.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }

    Point2f srcTri[3];
    srcTri[0] = Point2f(0.f, 0.f);
    srcTri[1] = Point2f(src.cols - 1.f, 0.f);
    srcTri[2] = Point2f(0.f, src.rows - 1.f);

    Point2f dstTri[3];
    dstTri[0] = Point2f(0.f, 0.f);
    dstTri[1] = Point2f(0.8f * (src.cols - 1), 0);
    dstTri[2] = Point2f(0.f, 0.8f * (src.rows - 1));

    Mat scale_mat = getAffineTransform(srcTri, dstTri);

    Mat scale_dst = Mat::zeros(src.size(), src.type());

    warpAffine(src, scale_dst, scale_mat, scale_dst.size());

    imshow("Affine, sclae", scale_dst);
    imshow("Original", src);

    waitKey(0);
}