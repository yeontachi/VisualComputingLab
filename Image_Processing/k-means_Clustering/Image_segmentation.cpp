#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int main() {
    Mat img = imread("Lenna.png", IMREAD_COLOR);
    if (img.empty()) {
        cerr << "Image Not Found!" << endl;
        return -1;
    }

    int width = img.cols;
    int height = img.rows;
    int nPoints = width * height;
    const int K = 8;  // 클러스터 개수

    Mat points(nPoints, 3, CV_32F);

    int idx = 0;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x, ++idx) {
            Vec3b bgr = img.at<Vec3b>(y, x);
            points.at<float>(idx, 0) = static_cast<float>(bgr[0]); // B
            points.at<float>(idx, 1) = static_cast<float>(bgr[1]); // G
            points.at<float>(idx, 2) = static_cast<float>(bgr[2]); // R
        }
    }

    // k-means Clustering
    Mat labels;   // 각 포인트의 클러스터 인덱스 (nPoints x 1, int)
    Mat centers;  // 클러스터 중심 (K x 3, float)

    TermCriteria criteria(
        TermCriteria::EPS + TermCriteria::MAX_ITER,
        10,   // 최대 반복 횟수
        1.0   // 수렴 기준
    );

    kmeans(points,          // N x 3, CV_32F
        K,               // 클러스터 개수
        labels,          // 출력: 각 포인트의 클러스터 번호
        criteria,        // 종료 조건
        3,               // 랜덤 초기화 시도 횟수
        KMEANS_RANDOM_CENTERS,
        centers);        // 출력: K x 3, CV_32F

    // 4. 결과 영상 생성
    Mat res(height, width, img.type());
    idx = 0;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x, ++idx) {
            int clusterIdx = labels.at<int>(idx, 0);  // 이 픽셀이 속한 클러스터

            Vec3b& dstPix = res.at<Vec3b>(y, x);
            for (int c = 0; c < 3; ++c) {
                float val = centers.at<float>(clusterIdx, c);  // 중심 B/G/R 값
                dstPix[c] = saturate_cast<uchar>(cvRound(val)); // 0~255로 자동 클램핑
            }
        }
    }

    // 결과 출력
    imshow("Input", img);
    imshow("Result (k-means color quantization)", res);
    waitKey(0);
    return 0;
}
