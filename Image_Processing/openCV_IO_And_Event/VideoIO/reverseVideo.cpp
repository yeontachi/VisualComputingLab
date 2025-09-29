#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;

int main(void)
{
    VideoCapture capture("0.avi");

    if (!capture.isOpened())
    {
        cerr << "Error opening video file" << "\n";
        return -1;
    }

    // 비디오 저장 객체
    VideoWriter writer;

    // 비디오 프레임을 저장할 Mat 벡터 선언
    vector<Mat> video;

    // VideoWriter(파일 이름, 코덱, 프레임 속도, 비디오 크기, 컬러);
    writer.open("NewVideo.avi", VideoWriter::fourcc('D', 'I', 'V', 'X'), 30, Size(640, 360), true);

    while (1)
    {
        // 캡쳐한 프레임을 Mat에 저장
        Mat frame;
        capture >> frame;

        if (frame.empty())
            break;

        video.push_back(frame); // Mat 타입 벡터에 프레임 넣기
    }

    for (size_t i = 0; i < video.size(); i++)
    {
        writer.write(video[video.size() - (i + 1)]); // 프레임을 거꾸로 저장
    }

    VideoCapture recap("NewVideo.avi");

    if (!recap.isOpened())
    {
        cerr << "Error, Can't Open Video file!" << "\n";
        return -1;
    }

    while (1)
    {
        Mat frame;
        recap >> frame;

        if (frame.empty())
            break;

        imshow("reverseVideo", frame);
        if (waitKey(33) > 0)
            break;
    } // 영상 결과 : 사람이 뒤로 걷는 모습

    return 0;
}