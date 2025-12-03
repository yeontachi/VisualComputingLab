#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;

// 기존 함수 (전체 이미지)
void detectSIFT(const Mat &img, vector<KeyPoint> &kps, Mat &desc)
{
    Mat gray;
    cvtColor(img, gray, COLOR_BGR2GRAY);
    Ptr<SIFT> sift = SIFT::create(0, 3, 0.04, 10.0, 1.6);
    sift->detectAndCompute(gray, noArray(), kps, desc);
}

// 새로운 함수 (블록별)
/**
 * @brief 이미지를 M x N 그리드로 나누어 각 블록에서 SIFT 특징점을 독립적으로 추출합니다.
 * * @param img 입력 이미지 (BGR)
 * @param M 수직 방향 (행) 분할 개수
 * @param N 수평 방향 (열) 분할 개수
 * @param all_kps 모든 블록에서 추출된 특징점들을 저장할 벡터
 * @param all_desc 모든 블록에서 추출된 특징 디스크립터를 저장할 Mat (행 단위로 합쳐짐)
 */
void detectSIFT_BlockWise(const Mat &img, int M, int N,
                          vector<KeyPoint> &all_kps, Mat &all_desc)
{
    if (M <= 0 || N <= 0)
    {
        cerr << "Error: M and N must be positive integers." << "\n";
        return;
    }

    Mat gray;
    cvtColor(img, gray, COLOR_BGR2GRAY);

    int rows = img.rows;
    int cols = img.cols;
    int block_height = rows / M;
    int block_width = cols / N;

    Ptr<SIFT> sift = SIFT::create(0, 3, 0.04, 10.0, 1.6);

    // 특징점과 디스크립터를 저장할 임시 변수 초기화
    all_kps.clear();
    vector<Mat> desc_list;

    for (int i = 0; i < M; ++i) // 행 (수직) 블록
    {
        for (int j = 0; j < N; ++j) // 열 (수평) 블록
        {
            // 현재 블록의 경계 설정
            int y_start = i * block_height;
            int x_start = j * block_width;

            // 마지막 블록은 남은 영역을 모두 포함하도록 크기 조정
            int current_height = (i == M - 1) ? (rows - y_start) : block_height;
            int current_width = (j == N - 1) ? (cols - x_start) : block_width;

            Rect block_roi(x_start, y_start, current_width, current_height);
            Mat block = gray(block_roi);

            vector<KeyPoint> kps_block;
            Mat desc_block;

            // SIFT 특징점 검출 및 디스크립터 계산
            sift->detectAndCompute(block, noArray(), kps_block, desc_block);

            // 특징점 좌표를 전체 이미지 좌표계로 변환
            for (auto &kp : kps_block)
            {
                kp.pt.x += x_start;
                kp.pt.y += y_start;
            }

            // 결과 합치기
            all_kps.insert(all_kps.end(), kps_block.begin(), kps_block.end());
            if (!desc_block.empty())
            {
                desc_list.push_back(desc_block);
            }
        }
    }

    // 모든 디스크립터를 하나의 Mat으로 병합 (수직으로 합치기)
    if (!desc_list.empty())
    {
        vconcat(desc_list, all_desc);
    }
    else
    {
        all_desc = Mat(); // 디스크립터가 하나도 없으면 빈 Mat 반환
    }
}

// 기존 함수 (매칭)
void SIFT_Matcher(const Mat &img1, vector<KeyPoint> &k1, Mat &d1,
                  const Mat &img2, vector<KeyPoint> &k2, Mat &d2)
{
    if (d1.empty() || d2.empty())
    {
        cout << "One or both descriptor matrices are empty. Cannot perform matching." << "\n";
        return;
    }

    BFMatcher matcher(NORM_L2, false);
    vector<vector<DMatch>> knn;
    matcher.knnMatch(d1, d2, knn, 2);

    const float ratio = 0.75f;
    vector<DMatch> good;

    for (const auto &m : knn)
    {
        // knnMatch 결과가 최소 2개이고, D. Lowe의 Ratio Test를 통과하는 경우에만 좋은 매치로 간주
        if (m.size() == 2 && m[0].distance < ratio * m[1].distance)
        {
            good.push_back(m[0]);
        }
    }

    Mat vis;
    drawMatches(img1, k1, img2, k2, good, vis, Scalar::all(-1), Scalar::all(-1), vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

    imshow("SIFT Matches (Block-Wise)", vis);
    waitKey(0);
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

    // 특징점을 저장할 벡터 및 Mat
    vector<KeyPoint> k1_block, k2_block;
    Mat d1_block, d2_block;

    // 이미지를 4 x 4 (16 블록)로 나누어 특징점 추출
    int M = 4; // 행 (수직) 분할 개수
    int N = 4; // 열 (수평) 분할 개수

    cout << "Detecting SIFT features in " << M << "x" << N << " blocks for Image 1..." << "\n";
    detectSIFT_BlockWise(img1, M, N, k1_block, d1_block);

    cout << "Detecting SIFT features in " << M << "x" << N << " blocks for Image 2..." << "\n";
    detectSIFT_BlockWise(img2, M, N, k2_block, d2_block);

    cout << "Image 1: " << k1_block.size() << " keypoints detected." << "\n";
    cout << "Image 2: " << k2_block.size() << " keypoints detected." << "\n";

    // 매칭 수행 및 결과 시각화
    SIFT_Matcher(img1, k1_block, d1_block, img2, k2_block, d2_block);

    return 0;
}