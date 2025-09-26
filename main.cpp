// This is for the test
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main()
{
    Mat img = imread("Lenna.png"); // 현재 경로의 이미지 불러오기
    if (img.empty())
    {
        cerr << "이미지를 불러올 수 없습니다." << endl;
        return -1;
    }

    imshow("Display window", img);
    waitKey(0); // 키 입력 대기
    return 0;
}
