#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int main(void)
{
    Mat img1 = imread("Lena.png");
    Mat img2 = imread("Lena.png");

    if (img1.empty() || img2.empty())
    {
        cerr << "image Not Found" << endl;
        return -1;
    }

    Mat gray;
    cvtColor(img1, gray, COLOR_BGR2GRAY);

    // 1) SIFT 생성
    Ptr<SIFT> sift = SIFT::create(
        0,    // nfeatures: 상한(0=자동)
        3,    // nOctaveLayers : 옥타브당 계층 수 (기본 3)
        0.04, // contrastThreshold : 낮출수록 더 많이 검출
        10.0, // edgeThreshold : 높일수록 에지 억제 완화
        1.6   // sigma: 초기 블러(가우시안 표준편차)
    );

    // 2) 검출 + descriptor 계산
    vector<KeyPoint> kps;
    Mat desc;
    sift->detectAndCompute(gray, noArray(), kps, desc);

    cout << "#keypoits=" << kps.size()
         << " , descriptor size=" << desc.rows << "x" << desc.cols << endl;

    // 3) 시각화
    Mat out;
    drawKeypoints(img1, kps, out, Scalar(0, 255, 0));
    imshow("SIFT Keypoints", out);

    waitKey(0);

    return 0;
}