#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#define PI 3.141592265358979323846

using namespace std;
using namespace cv;

int main(void)
{
    Mat src = imread("Lena.png");
    if (src.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }
    /*
    Affine Transformation
    좌상단 0, 우상단 1, 좌하단 2 로
    원본 영상에서의 0,1,2값을 변환 후 영상에서 점 3개에 매핑시켜 행렬을 자동으로 만들어줌
    "getAffineTransform"
    */

    Point2f srcTri[3]; // 원본 영상에서의 점 3개
    srcTri[0] = Point2f(10.f, 30.f);
    srcTri[1] = Point2f(src.cols - 1.f, 0.f);
    srcTri[2] = Point2f(20.f, src.rows - 1.f);

    Point2f dstTri[3]; // 변환 후 영상에서의 점 3개
    dstTri[0] = Point2f(50.f, src.rows * 0.33f);
    dstTri[1] = Point2f(src.cols * 0.85f, src.rows * 0.25f);
    dstTri[2] = Point2f(src.cols * 0.15f, src.rows * 0.7f);

    // 원본 영상에서의 점 3개와 변환 후 영상에서의 점 3개를 주면 Affine 행렬 M을 자동으로 계산해주는 함수
    Mat warp_mat = getAffineTransform(srcTri, dstTri); // 자동으로 계산된 Affine 행렬 M을 저장
    /*
    내부적으로,
    3개의 대응점에 대해, X' = A*X + t
    를 만족하는 A, t를 선형 방정식 6개로 만들고
    이를 풀어서 m_00 ~ m_12 값을 구한다.
    */

    Mat warp_dst = Mat::zeros(src.rows, src.cols, src.type());

    // 실제 이미지 변환 : Affine 행렬을 이용해 이미지를 실제로 변환(워핑)하는 함수
    warpAffine(src, warp_dst, warp_mat, warp_dst.size());
    /*
    src : 입력 이미지
    dst : 출력 이미지
    M : 2x3 Affine 행렬(getAffineTransform으로 구한 행렬)
    dsize : 출력 영상의 크기
    flags : 보간 방식, 역변환 사용 여부 등
    borderMode : 변환 후 이미지 밖에서 참조되는 픽셀을 어떻게 채울지
    borderValue : BORDER_CONSTANT일 때 바깥 영역을 채울 색(기본은 검정)
    */

    Point center = Point(warp_dst.cols / 2, warp_dst.rows / 2);
    double angle = -50.0;
    double scale = 0.6;

    // 회전 + 스케일 특화 : 회전은 많이 쓰는 변환이므로, OpenCV는 중심점 기준 회전 + 스케일링을 손쉽게 만들 수 있는 함수를 제공한다.
    Mat rot_mat = getRotationMatrix2D(center, angle, scale);
    /*
    center : 회전 중심(예: 이미지 중심)
    angle : 시계 반대 방향(degree 단위)
    scale : 확대/축소 계수 (1.0이면 순수 회전)
    */

    Mat warp_rotate_dst;
    warpAffine(warp_dst, warp_rotate_dst, rot_mat, warp_dst.size());

    imshow("Source image", src);
    imshow("Warp", warp_dst);
    imshow("Warp + Rotate", warp_rotate_dst);

    waitKey(0);

    return 0;
}