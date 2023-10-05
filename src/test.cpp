//
// Created by sakurapuare on 23-10-5.
//
#include <opencv4/opencv2/opencv.hpp>
#include "util/color.hpp"

#include <iostream>

using namespace std;
using namespace cv;

// 实例化色彩阈值类
ColorThread color;
static Mat kernel = getStructuringElement(MORPH_RECT, Size(2, 2));

void PreProcessFrame(Mat &before, Mat &after, int c) {
    // min, max
    Scalar min = color.get_min(c);
    Scalar max = color.get_max(c);

    inRange(before, min, max, after);
    erode(after, after, kernel, Point(-1, -1), 2);
    dilate(after, after, kernel, Point(-1, -1), 5);
}

bool checkColorBarExist(Mat &curr_frame, int c, bool draw_line = true) {
    Mat color_frame, canny_frame;
    PreProcessFrame(curr_frame, color_frame, c);
    imwrite("image/blue_image.jpg", color_frame);
    vector<cv::Vec2f> lines;
    Canny(color_frame, canny_frame, 1, 1, 3);
    imwrite("image/blue_canny.jpg", canny_frame);
    HoughLines(canny_frame, lines, 1, CV_PI / 180, 100);

    // count horizontal lines
    int horizontal_lines = 0;
    for (auto &line: lines) {
        float rho = line[0];
        float theta = line[1];
        if (theta > CV_PI / 4 && theta < 3 * CV_PI / 4)
            horizontal_lines++;
        if (draw_line) {
            cv::Point pt1, pt2;
            double a = cos(theta), b = sin(theta);
            double x0 = a * rho, y0 = b * rho;
            pt1.x = cvRound(x0 + 1000 * (-b));
            pt1.y = cvRound(y0 + 1000 * (a));
            pt2.x = cvRound(x0 - 1000 * (-b));
            pt2.y = cvRound(y0 - 1000 * (a));

            cv::line(curr_frame, pt1, pt2, Scalar(0, 255, 255), 5, cv::LINE_AA);
        }
    }
    return horizontal_lines > 2;
}


int main() {
    // read image
    Mat image = imread("image/blue.jpg");
    resize(image, image, Size(400, 300));
    GaussianBlur(image, image, Size(3, 3), 0, 0);

//    PreProcessFrame(image, image, blue);
    imwrite("image/blue_image.jpg", image);

    cout << checkColorBarExist(image, blue, true);
    imwrite("image/blue_line.jpg", image);
    return 0;
}