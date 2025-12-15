#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/calib3d.hpp>

using namespace cv;
using namespace std;

Mat homography(Mat src, Mat dst)
{
    Ptr<SIFT> detector = SIFT::create();
    std::vector<KeyPoint> keypoints1, keypoints2;
    Mat descriptors1, descriptors2;
    detector->detectAndCompute(src, noArray(), keypoints1, descriptors1);
    detector->detectAndCompute(dst, noArray(), keypoints2, descriptors2);

    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);
    std::vector<std::vector<DMatch>> knn_matches;
    matcher->knnMatch(descriptors1, descriptors2, knn_matches, 2);

    const float ratio_thresh = 0.7f;
    vector<Point2f> src_point, dst_point;
    for (size_t i = 0; i < knn_matches.size(); i++)
    {
        if (knn_matches[i].size() < 2)
            continue;
        const DMatch &m1 = knn_matches[i][0];
        const DMatch &m2 = knn_matches[i][1];
        if (m1.distance < ratio_thresh * m2.distance)
        {
            src_point.push_back(keypoints1[m1.queryIdx].pt);
            dst_point.push_back(keypoints2[m1.trainIdx].pt);
        }
    }

    if (src_point.size() < 4)
        return Mat::eye(3, 3, CV_64F);
    Mat H = findHomography(src_point, dst_point, RANSAC);

    if (H.empty() || H.rows != 3 || H.cols != 3)
        return Mat::eye(3, 3, CV_64F);
    return H;
}

vector<double> smooth_path(const vector<double> &path, int kernel_size, double sigma)
{
    if (path.empty())
        return {};

    Mat path_mat(path, true);
    path_mat.convertTo(path_mat, CV_64F);

    if (kernel_size % 2 == 0)
        kernel_size++;
    if (kernel_size < 3)
        kernel_size = 3;

    Mat path_col = path_mat.reshape(1, path_mat.total()).t();
    Mat smoothed_col;

    GaussianBlur(path_col, smoothed_col, Size(1, kernel_size), sigma, 0, BORDER_REFLECT);

    Mat smoothed_mat = smoothed_col.t();
    vector<double> smoothed_path;
    smoothed_mat.reshape(1, 1).copyTo(smoothed_path);

    return smoothed_path;
}

int main(int argc, char *argv[])
{
    // 1. Motion Estimation (움직임 추정)
    VideoCapture cap = VideoCapture("0.avi");
    if (!cap.isOpened())
    {
        cerr << "Error: Could not open video file 0.avi" << endl;
        return -1;
    }

    vector<Mat> src;
    Mat frame;
    while (cap.read(frame))
    {
        Mat gray_frame;
        cvtColor(frame, gray_frame, COLOR_BGR2GRAY);
        src.push_back(gray_frame);
    }
    if (src.empty())
    {
        cerr << "Error: No frames read from video." << endl;
        return -1;
    }

    vector<Mat> H_list;
    Mat nujeok = Mat::eye(Size(3, 3), CV_64F);
    H_list.push_back(nujeok.clone());

    for (int i = 1; i < src.size(); i++)
    {
        Mat H = homography(src[i - 1], src[i]);
        nujeok = H * nujeok;
        H_list.push_back(nujeok.clone());
    }

    // 2. Path Planning & Smoothing (경로 계획 및 부드럽게 처리)

    vector<vector<double>> path_vectors(9);

    for (const Mat &H : H_list)
    {
        if (H.empty() || H.type() != CV_64F)
            continue;
        const double *ptr = H.ptr<double>(0);
        for (int k = 0; k < 9; ++k)
        {
            path_vectors[k].push_back(ptr[k]);
        }
    }

    int KERNEL_SIZE = 30;
    double SIGMA = 20.0;

    vector<vector<double>> smoothed_path_vectors(9);
    for (int k = 0; k < 9; ++k)
    {
        smoothed_path_vectors[k] = smooth_path(path_vectors[k], KERNEL_SIZE, SIGMA);
    }

    // 3. Stabilization (안정화)

    vector<Mat> H_stabilized_list;

    for (size_t i = 0; i < H_list.size(); ++i)
    {
        Mat H_smooth = Mat::eye(3, 3, CV_64F);
        for (int r = 0; r < 3; ++r)
        {
            for (int c = 0; c < 3; ++c)
            {
                H_smooth.at<double>(r, c) = smoothed_path_vectors[r * 3 + c][i];
            }
        }
        H_stabilized_list.push_back(H_smooth);
    }

    // 4. Motion Compensation 및 출력

    namedWindow("Original", WINDOW_AUTOSIZE);
    namedWindow("Stabilized", WINDOW_AUTOSIZE);

    for (size_t i = 0; i < src.size(); i++)
    {
        Mat H_comp = H_stabilized_list[i] * H_list[i].inv();

        Mat rst;
        warpPerspective(src[i], rst, H_comp, src[i].size(), INTER_LINEAR, BORDER_CONSTANT, Scalar(0));

        imshow("Original", src[i]);
        imshow("Stabilized", rst);

        char key = (char)waitKey(30);
        if (key == 'q' || key == 27)
            break;
    }

    waitKey(0);
    return 0;
}