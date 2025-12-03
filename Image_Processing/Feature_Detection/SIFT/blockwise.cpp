#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;

void detectSIFT_BlockWise(const Mat &img, int M, int N, vector<KeyPoint> &kps, Mat &desc)
{
    Mat gray;
    cvtColor(img, gray, COLOR_BGR2GRAY);

    int rows = img.rows;
    int cols = img.cols;
    int block_height = rows / M;
    int block_width = cols / N;

    Ptr<SIFT> sift = SIFT::create(0, 3, 0.04, 10.0, 1.6);

    kps.clear();
    vector<Mat> desc_list;

    for (int h = 0; h < M; ++h)
    {
        for (int w = 0; w < N; ++w)
        {
            int y_start = h * block_height;
            int x_start = w * block_width;

            int current_height = (h == M - 1) ? (rows - y_start) : block_height;
            int current_width = (w == N - 1) ? (rows - x_start) : block_width;

            Rect block_roi(x_start, y_start, current_width, current_height);
            Mat block = gray(block_roi);

            vector<KeyPoint> kps_block;
            Mat desc_block;

            sift->detectAndCompute(block, noArray(), kps_block, desc_block);

            for (auto &kp : kps_block)
            {
                kp.pt.x += x_start;
                kp.pt.y += y_start;
            }

            kps.insert(kps.end(), kps_block.begin(), kps_block.end());
            if (!desc_block.empty())
                desc_list.push_back(desc_block);
        }
    }

    if (!desc_list.empty())
        vconcat(desc_list, desc);
    else
        desc = Mat();

    Mat vis;

    drawKeypoints(img, kps, vis, Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

    imshow("Blockwise SIFT Feature Detectioin", vis);
}

int main(void)
{
    Mat img1 = imread("Lena.png");
    // Mat img2 = imread("matchLena.png");

    if (img1.empty() /*|| img2.empty()*/)
    {
        cerr << "Image Not FOund" << "\n";
        return -1;
    }

    vector<KeyPoint> k1_block, k2_block;
    Mat d1_block, d2_block;

    int M = 50;
    int N = 50;

    detectSIFT_BlockWise(img1, M, N, k1_block, d1_block);
    // detectSIFT_BlockWise(img2, M, N, k2_block, d2_block);

    waitKey(0);

    return 0;
}