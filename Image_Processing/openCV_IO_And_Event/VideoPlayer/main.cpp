#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <vector>
#include <iostream>
#include <cctype>

using namespace cv;
using namespace std;

// 현재 인덱스/재생상태/비디오 접근을 콜백에 전달하기 위한 컨텍스트
struct MouseCtx
{
    vector<Mat> *video;
    // 원본 프레임 벡터
    int *pIndex;
    // 현재 프레임 인덱스
    bool *pIsPlaying; // 재생 여부 (정지일 때만 그리기 허용)
    bool isDragging = false;
    Point pt1, pt2;
};

// 드래그 방향 무관 정규화 + 프레임 경계 클램핑
static Rect makeNormalizedRect(Point p1, Point p2, const Size &bound)
{
    int x1 = max(0, min(p1.x, p2.x));
    int y1 = max(0, min(p1.y, p2.y));
    int x2 = min(bound.width - 1, max(p1.x, p2.x));
    int y2 = min(bound.height - 1, max(p1.y, p2.y));
    return Rect(Point(x1, y1), Point(x2, y2));
}

// 마우스 콜백: 드래그 미리보기는 clone, 버튼 업에서 video[i]에 직접 그리기
static void onMouse(int evt, int x, int y, int /*flags*/, void *param)
{
    auto *ctx = reinterpret_cast<MouseCtx *>(param);
    if (!ctx || !ctx->video || !ctx->pIndex || !ctx->pIsPlaying)
        return;
    // 정지 상태에서만 그리기
    if (*(ctx->pIsPlaying))
        return;
    int idx = *(ctx->pIndex);
    if (idx < 0 || idx >= static_cast<int>(ctx->video->size()))
        return;
    Mat &frame = (*(ctx->video))[idx]; // 현재 프레임(원본, 저장 대상)
    if (evt == EVENT_LBUTTONDOWN)
    {
        ctx->isDragging = true;
        ctx->pt1 = ctx->pt2 = Point(x, y);
        // 미리보기
        Mat preview = frame.clone();
        rectangle(preview, makeNormalizedRect(ctx->pt1, ctx->pt2, frame.size()), Scalar(0, 0, 255), 2);
        imshow("video", preview);
    }
    else if (evt == EVENT_MOUSEMOVE && ctx->isDragging)
    {
        ctx->pt2 = Point(x, y);
        // 미리보기
        Mat preview = frame.clone();
        rectangle(preview, makeNormalizedRect(ctx->pt1, ctx->pt2, frame.size()), Scalar(0, 0, 255), 2);
        imshow("video", preview);
    }
    else if (evt == EVENT_LBUTTONUP && ctx->isDragging)
    {
        ctx->isDragging = false;
        ctx->pt2 = Point(x, y);

        Rect r = makeNormalizedRect(ctx->pt1, ctx->pt2, frame.size());
        if (r.width > 0 && r.height > 0)
        {
            // 원본 프레임에 직접 그려 저장
            rectangle(frame, r, Scalar(0, 0, 255), 2);
        }
        // 저장된 원본 즉시 표시
        imshow("video", frame);
    }
}

int main()
{
    VideoCapture capture("0.avi");
    if (!capture.isOpened())
    {
        cerr << "Error, could not load a camera or video." << endl;
        return -1;
    }
    vector<Mat> video;
    while (true)
    {
        Mat frame;
        capture >> frame;
        if (frame.empty())
            break;
        video.push_back(frame);
    }
    if (video.empty())
    {
        cerr << "No frames loaded." << endl;
        return -1;
    }

    int i = 0;
    bool isPlaying = false;
    const int playDelayMs = 33; // ~30fps
    namedWindow("video", WINDOW_AUTOSIZE);

    // 마우스 컨텍스트 설정
    MouseCtx ctx;
    ctx.video = &video;
    ctx.pIndex = &i;
    ctx.pIsPlaying = &isPlaying;
    setMouseCallback("video", onMouse, &ctx);
    // 첫 프레임 표시
    imshow("video", video[i]);
    while (true)
    {
        int delay = isPlaying ? playDelayMs : 0;
        int key = waitKey(delay);
        if (key < 0)
        {
            // 키 입력 없고 재생 중이면 다음 프레임
            if (isPlaying)
            {
                if (i + 1 < static_cast<int>(video.size()))
                {
                    i++;
                }
                else
                {
                    isPlaying = false; // 끝에 도달하면 정지
                }
                imshow("video", video[i]);
            }
            continue;
        }
        char c = static_cast<char>(std::tolower(static_cast<unsigned char>(key)));
        if (c == 27)
        { // ESC
            break;
        }
        else if (c == ' ')
        { // 재생/정지 토글
            isPlaying = !isPlaying;
            imshow("video", video[i]);
        }
        else if (c == 'p')
        { // 이전 프레임
            isPlaying = false;
            if (i > 0)
                i--;
            imshow("video", video[i]);
        }
        else if (c == 'n')
        { // 다음 프레임
            isPlaying = false;
            if (i + 1 < static_cast<int>(video.size()))
                i++;
            imshow("video", video[i]);
        }
    }
    destroyAllWindows();

    return 0;
}
