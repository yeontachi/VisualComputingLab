#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;

void detectSIFT(const Mat &img, vector<KeyPoint> &kps, Mat &desc)
{
    Mat gray;
    cvtColor(img, gray, COLOR_BGR2GRAY);
    Ptr<SIFT> sift = SIFT::create(0, 3, 0.04, 10.0, 1.6);
    sift->detectAndCompute(img, noArray(), kps, desc);
}

void SIFT_Matches(const Mat &img1, vector<KeyPoint> &k1, Mat &d1,
                  const Mat &img2, vector<KeyPoint> &k2, Mat &d2)
{
    BFMatcher matcher(NORM_L2, false);
    vector<vector<DMatch>> knn;
    matcher.knnMatch(d1, d2, knn, 2);

    const float ratio = 0.75f;
    vector<DMatch> good;

    for (auto &m : knn)
    {
        if (m.size() == 2 && m[0].distance < ratio * m[1].distance)
        {
            good.push_back(m[0]);
        }
    }

    Mat vis;
    drawMatches(img1, k1, img2, k2, good, vis, Scalar::all(-1), Scalar::all(-1), vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

    imshow("SIFT Matching", vis);
}

int main(void)
{
    Mat img1 = imread("Lena.png");
    Mat img2 = imread("matchLena.png");

    if (img1.empty() || img2.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }

    vector<KeyPoint> k1, k2;
    Mat d1, d2;

    detectSIFT(img1, k1, d1);
    detectSIFT(img2, k2, d2);

    SIFT_Matches(img1, k1, d1, img2, k2, d2);

    waitKey(0);

    return 0;
}