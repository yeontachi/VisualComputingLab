#include <opencv2/opencv.hpp>
#include <iostream>
#include <ctime>

using namespace std;
using namespace cv;

int main(void)
{
    const int MAX_CLUSTERS = 5;
    Scalar colorTab[] = {
        Scalar(0, 0, 255),
        Scalar(0, 255, 0),
        Scalar(255, 100, 100),
        Scalar(255, 0, 255),
        Scalar(0, 255, 255)};

    Mat img(500, 500, CV_8UC3);
    RNG rng(time(NULL));
    for (;;)
    {
        int k, clusterCount = 5;
        int i, sampleCount = 1000;

        Mat points(sampleCount, 1, CV_32FC2), labels;
        clusterCount = MIN(clusterCount, sampleCount);
        Mat centers;

        for (k = 0; k < clusterCount; k++)
        {
            Point center;
            center.x = rng.uniform(0, img.cols);
            center.y = rng.uniform(0, img.rows);
            Mat pointChunk = points.rowRange(k * sampleCount / clusterCount,
                                             k == clusterCount - 1 ? sampleCount : (k + 1) * sampleCount / clusterCount);
            rng.fill(pointChunk, RNG::NORMAL, Scalar(center.x, center.y), Scalar(img.cols * 0.05, img.rows * 0.05));
        }
        randShuffle(points, 1, &rng);
        kmeans(points, clusterCount, labels,
               TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 10, 1.0), 3, KMEANS_RANDOM_CENTERS, centers);
        img = Scalar::all(0);

        for (i = 0; i < sampleCount; ++i)
        {
            int clusterIdx = labels.at<int>(i);
            Point ipt = points.at<Point2f>(i);
            circle(img, ipt, 2, colorTab[clusterIdx], FILLED);
        }

        imshow("clusters", img);
        char key = (char)waitKey();
        if (key == 27 || key == 'q' || key == '0')
            break;
    }

    return 0;
}