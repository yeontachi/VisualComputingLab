#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <iostream>

using namespace std;
using namespace cv;

static void detectSIFT(const Mat &img, vector<KeyPoint> &kps, Mat &desc)
{
    Mat gray;
    cvtColor(img, gray, COLOR_BGR2GRAY);
    Ptr<SIFT> sift = SIFT::create(0, 3, 0.04, 10.0, 1.6);
    sift->detectAndCompute(gray, noArray(), kps, desc);
}

int main(void)
{
    Mat img1 = imread("Lena.png");
    Mat img2 = imread("Lena.png");

    if (img1.empty() || img2.empty())
    {
        cerr << "Image Not Found" << endl;
        return -1;
    }

    vector<KeyPoint> k1, k2;
    Mat d1, d2;
    detectSIFT(img1, k1, d1);
    detectSIFT(img2, k2, d2);

    // -----------------------------------------
    // 2) KNN 매칭 (k=2)
    //    - BFMatcher: L2 거리(유클리드)로 완전탐색 매칭
    //    - crossCheck=false: 교차 일치 강제하지 않음(대신 Ratio Test 사용)
    // -----------------------------------------
    BFMatcher matcher(NORM_L2, false);
    vector<vector<DMatch>> knn;
    matcher.knnMatch(d1, d2, knn, 2);

    // -----------------------------------------
    // 3) Lowe Ratio Test
    //    - 가장 가까운 후보 m[0]과 차선 m[1]의 거리 비율을 비교
    //    - m[0].distance < ratio * m[1].distance 이면 좋은 매칭으로 채택
    //    - ratio=0.75가 보편적인 시작점(0.7~0.8 사이 튜닝)
    // -----------------------------------------
    const float ratio = 0.75f;
    vector<DMatch> good;

    for (auto &m : knn)
    {
        // knnMatch로 2개를 요구했지만,
        // 일부 항목은 후보가 1개만 나올 수도 있으므로 size 체크 필요
        if (m.size() == 2 && m[0].distance < ratio * m[1].distance)
        {
            good.push_back(m[0]);
        }
    }

    cout << "matches: " << knn.size() << " , good" << good.size() << endl;

    // -----------------------------------------
    // 4) 매칭 시각화
    //    - NOT_DRAW_SINGLE_POINTS: 매칭되지 않은 키포인트는 생략
    // -----------------------------------------
    Mat vis;
    drawMatches(img1, k1, img2, k2, good, vis, Scalar::all(-1), Scalar::all(-1),
                vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

    imshow("SIFT matches", vis);
    waitKey(0);
    return 0;
}