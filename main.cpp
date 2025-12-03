#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <iostream>

using namespace std;
using namespace cv;

void SIFT_FeaturePoints(const Mat &src);

int main(void)
{
    Mat src = imread("Lena.png");
    if (src.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }

    SIFT_FeaturePoints(src);

    waitKey(0);

    return 0;
}

void SIFT_FeaturePoints(const Mat &src)
{
    Mat gray;
    cvtColor(src, gray, COLOR_BGR2GRAY);

    Ptr<SIFT> sift = SIFT::create(
        0,
        3,
        0.01,
        10.0,
        1.6);

    vector<KeyPoint> kps;
    Mat desc;

    sift->detectAndCompute(gray, noArray(), kps, desc);

    cout << "keypoints = " << kps.size() << " , descriptor size=" << desc.rows << "x" << desc.cols << endl;

    Mat out;
    drawKeypoints(src, kps, out, Scalar(0, 255, 0));

    imshow("SIFT Keypoints", out);
}