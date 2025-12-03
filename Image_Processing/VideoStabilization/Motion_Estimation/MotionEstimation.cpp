#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

Mat homography(Mat src, Mat dst)
{
    //-- Step 1: Detect the keypoints using SURF Detector, compute the descriptors
    int minHessian = 400;
    Ptr<SIFT> detector = SIFT::create(minHessian);
    std::vector<KeyPoint> keypoints1, keypoints2;
    Mat descriptors1, descriptors2;
    detector->detectAndCompute(src, noArray(), keypoints1, descriptors1);
    detector->detectAndCompute(dst, noArray(), keypoints2, descriptors2);

    //-- Step 2: Matching descriptor vectors with a FLANN based matcher
    // Since SURF is a floating-point descriptor NORM_L2 is used
    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);
    std::vector<std::vector<DMatch>> knn_matches;
    matcher->knnMatch(descriptors1, descriptors2, knn_matches, 2);

    //-- Filter matches using the Lowe's ratio test
    const float ratio_thresh = 0.7f;
    std::vector<DMatch> good_matches;
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
            // good_matches.push_back(knn_matches[i][0]);
        }
    }

    Mat H = findHomography(src_point, dst_point, RANSAC);
    if (H.empty())
        H = Mat::eye(3, 3, CV_64F);
    return H;
}

int main(int argc, char *argv[])
{
    VideoCapture cap = VideoCapture("0.avi");
    Mat frame;
    vector<Mat> src = {};

    while (cap.read(frame))
    {
        cvtColor(frame, frame, COLOR_BGR2GRAY);
        src.push_back(frame);
    }

    vector<Mat> H_list;
    Mat accum = Mat::eye(Size(3, 3), CV_64F);
    H_list.push_back(accum.clone());
    // cout << H_list[0] << endl;

    for (int i = 1; i < src.size(); i++)
    {
        Mat prev = src[i - 1];
        Mat now = src[i];
        // cout << H_list[0] << endl;

        Mat H = homography(prev, now);
        accum = H * accum;
        H_list.push_back(accum.clone());
        cout << H_list[i - 1] << endl;
    }

    for (int i = 0; i < src.size(); i++)
    {
        Mat rst;
        warpPerspective(src[i], rst, H_list[i].inv(), src[i].size());
        print(H_list[i]);
        imshow("rst", rst);
        waitKey(0);
    }

    return 0;
}