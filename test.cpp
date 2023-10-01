#include <iostream>
#include <opencv4/opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main() {
    // read image
    Mat image = imread("./1.jpg");
    // resize image
    resize(image, image, Size(400, 300));
    imwrite("./resize.jpg", image);
    // blur image
    Mat blur_image;
    medianBlur(image, blur_image, 5);

    Mat canny_image;
    Canny(blur_image, canny_image, 50, 150, 3);

    vector<cv::Vec2f> lines;
    HoughLines(canny_image, lines, 1, CV_PI / 180, 100); // 调整参数以获得最佳结果

    for (size_t i = 0; i < lines.size(); i++) {
        float rho = lines[i][0];
        float theta = lines[i][1];

        if (theta > CV_PI / 4 && theta < 3 * CV_PI / 4) {
            // 计算直线的两个端点
            cv::Point pt1, pt2;
            double a = cos(theta), b = sin(theta);
            double x0 = a * rho, y0 = b * rho;
            pt1.x = cvRound(x0 + 1000 * (-b));
            pt1.y = cvRound(y0 + 1000 * (a));
            pt2.x = cvRound(x0 - 1000 * (-b));
            pt2.y = cvRound(y0 - 1000 * (a));

            // 在原始图像上绘制直线
            cv::line(image, pt1, pt2, cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
        }
    }

    imwrite("./2.jpg", image);

    // binary image
    //    Mat binary_image;
    //    inRange(blur_image, Scalar(0, 0, 0), Scalar(255, 255, 255), binary_image);

    // save image
    //    imwrite("./binary_image.jpg", binary_image);
}