#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

Mat addSaltPepperNoiseGray(const Mat &src, double p_salt, double p_pepper)
{
    Mat dst = src.clone();
    RNG rng((uint64)-1);

    const double thr_salt = p_salt;
    const double thr_pepper = p_salt + p_pepper;

    for (int h = 0; h < src.rows; ++h)
    {
        uchar *drow = dst.ptr<uchar>(h);
        for (int w = 0; w < src.cols; ++w)
        {
            double u = rng.uniform(0.0, 1.0);
            if (u < thr_salt)
                drow[w] = 255; // salt
            else if (u < thr_pepper)
                drow[w] = 0; // pepper
        }
    }
    return dst;
}

int main(void)
{
    Mat src = imread("Lena.png", IMREAD_GRAYSCALE);

    if (src.empty())
    {
        cerr << "Image Not Found" << "\n";
        return -1;
    }

    double p_salt = 0.01, p_pepper = 0.01;
    cout << "salt 확률: ";
    cin >> p_salt;
    cout << "pepper 확률: ";
    cin >> p_pepper;

    Mat SP_img = addSaltPepperNoiseGray(src, p_salt, p_pepper);

    imshow("Original", src);
    imshow("Salt and Pepper Noise", SP_img);
    imwrite("lena256SPN.png", SP_img);

    waitKey(0);

    return 0;
}