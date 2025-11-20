#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

Mat homography(Mat src, Mat dst)
{
    int minHessian = 400;
    Ptr<SIFT> detector = SIFT::create(minHessian);

    vector<KeyPoint> keypoints1, keypoints2;
    Mat descriptors1, descriptors2;
    detector->detectAndCompute(src, noArray(), keypoints1, descriptors1);
    detector->detectAndCompute(dst, noArray(), keypoints2, descriptors2);

    if (descriptors1.empty() || descriptors2.empty())
        return Mat::eye(3, 3, CV_64F);

    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);
    vector<vector<DMatch>> knn_matches;
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
    if (H.empty())
        return Mat::eye(3, 3, CV_64F);

    // 스케일 정규화 (선택)
    double s = H.at<double>(2, 2);
    if (fabs(s) > 1e-12)
        H /= s;

    return H;
}

int main(int argc, char *argv[])
{
    VideoCapture cap("0.avi");
    Mat frame;
    vector<Mat> src;

    while (cap.read(frame))
    {
        cvtColor(frame, frame, COLOR_BGR2GRAY);
        src.push_back(frame.clone()); // ★ clone
    }

    if (src.empty())
        return -1;

    vector<Mat> H_list;
    Mat base = src[0];
    H_list.push_back(Mat::eye(3, 3, CV_64F));

    for (int i = 1; i < (int)src.size(); i++)
    {
        Mat now = src[i];
        Mat H = homography(now, base); // now → base
        H_list.push_back(H.clone());
    }

    for (int i = 0; i < (int)src.size(); i++)
    {
        Mat rst;
        warpPerspective(src[i], rst, H_list[i], src[0].size());
        imshow("rst", rst);
        waitKey(0);
    }

    return 0;
}
