// This is for the test
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main()
{
    // 현재 프로젝트 폴더에 있는 "Lenna.png" 이미지 파일을 불러온 후, Image 변수에 저장
    Mat img = imread("Lenna.png", IMREAD_GRAYSCALE);
    if (img.empty())
    {
        cerr << "이미지를 불러올 수 없습니다." << endl;
        return -1;
    }

    // 윈도우에 이름 설정
    namedWindow("Display", WINDOW_AUTOSIZE);

    // imshow ("윈도우 이름", 윈도우에 디스플레이 하고자 하는 이미지)
    imshow("Display", img);

    // imwrite("저장하고자 하는 이미지 파일 이름.파일확장자", 저장하고자 하는 이미지)
    imwrite("Lenna_copy.png", img);

    waitKey(0);

    return 0;
}
