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
    const float ratio = 0.75f; // Lowe ratio 임계값
    vector<DMatch> good;       // 최종 채택된 매칭을 담을 컨테이너

    for (auto &m : knn) // m: 한 쿼리 디스크립터에 대한 상위 k(=2)개의 후보 매칭 목록 , m[0]: 최단거리 후보, m[1]: 차선거리 후보
    // knn 각 원소에 대해 순회, m은 knn 내부 vecotr<DMatch> 자체를 가리킴
    {
        // knnMatch로 2개를 요구했지만,
        // 일부 항목은 후보가 1개만 나올 수도 있으므로 size 체크 필요
        if (m.size() == 2 && m[0].distance < ratio * m[1].distance) // 1등 거리 < 0.75*2등 거리 : 1등이 2등보다 상대적으로 충분히 가깝다면 진짜 대응일 가능성이 높다고 본다.(ratio는 경험적 파라미터 데이터/장면에 따라 조정)
        {
            good.push_back(m[0]); // 조건을 통과한 1등 후보만 최종 매칭으로 저장
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

/*
DMatch가 저장하는 것
OpenCV의 cv::DMatch는 한 쌍의 디스크립터 매칭을 표현하는 구조체이다.
 - queryIdx : 쿼리 디스크립터(d1)의 행 인덱스, 즉 k1[queryIdx]가 매칭된 왼쪽 이미지의 키 포인트
 - trainIdx : 타깃 디스크립터(d2)의 행 인덱스, 즉 k2[trainIdx]가 매칭된 오른쪽 이미지의 키 포인트
 - imgIdx : 다중이미지 매칭에서 타깃 이미지의 인덱스, 단일 타깃이면 보통 0
 - distance ; 두 디스크립터 사이의 거리, SIFT에서는 NORM_L2이므로 유클리드거리. 값이 작을수록 더 유사하다는 의미

===============================================================================================================================

good 벡터?
Ratio Test를 통과한 신뢰도 높은 매칭들만 DMatch 형태로 저장
따라서 good[i]는 queryIdx(왼쪽 키포인트), trainIdx(오른쪽 키포인트), distance 정보를 갖고 있다.

===============================================================================================================================
summary
 - knn는 “쿼리 디스크립터마다 상위 2개 후보 매칭”을 담는 2중 벡터 구조이다.
 - auto &m : knn는 그 2중 벡터를 참조로 순회하며, m은 한 쿼리에 대한 후보 리스트(vector<DMatch>)이다.
 - DMatch는 “쿼리/타깃 인덱스 + 거리”를 담는 한 쌍의 매칭이다.
 - Ratio Test는 “1등이 2등보다 충분히 가깝다”를 수학적으로 m[0].distance < ratio * m[1].distance로 검사하며, 통과한 매칭만 good에 모은다.
 - 최종적으로 good은 시각화·기하학적 정제(RANSAC)·포즈 추정 등 후속 단계에 바로 사용 가능한 정제된 매칭 목록이 된다.
*/