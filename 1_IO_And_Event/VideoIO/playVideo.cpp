#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int main(void)
{
    // 프로젝트 폴더에 있는 '0.avi' 비디오 파일을 불러옴
    VideoCapture capture("0.avi");

    Mat frame;

    if (!capture.isOpened())
    {
        cerr << "Error, opening video file" << "\n";
        return -1;
    }

    namedWindow("video");

    while (1)
    {
        // 캡쳐한 이미지를 프레임에 저장
        capture >> frame;

        if (frame.empty())
            break; // 비디오가 끝나면 break;

        imshow("video", frame);

        if (waitKey(33) > 0)
            break; // waitKey 안의 숫자 설정으로 fps 조정 가능
    }
}