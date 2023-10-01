#include <chrono>

#include "udputil.hpp"
#include "colorgroup.hpp"
#include "lcmutil.hpp"

#include <QCoreApplication>
#include <QThread>
#include <opencv4/opencv2/opencv.hpp>

#define DEBUG true

using namespace std;
using namespace cv;

enum Mode {
    track,
    limit,
    resident,
    cross,
    stair,
};

class TimerThread : public QThread {
public:
    int curr_mode = track;
    int next_color = blue;
    int stage = 0;
    int laps = 1;

    chrono::time_point<chrono::steady_clock> start_time;

    void start() {
        start_time = chrono::steady_clock::now();
        QThread::start();
    }

    void stop() {
        requestInterruption();
        QThread::wait();
    }

    void run() override {
        // while thread is running
        while (!isInterruptionRequested()) {
            switch (curr_mode) {
                case limit: {
                    QThread::sleep(1);
                    stage++;
                    QThread::sleep(6);
                    stage++;
                    break;
                }
                case resident: {
                    QThread::sleep(5);
                    stage++;
                    QThread::sleep(5);
                    stage++;
                    QThread::sleep(10);
                    stage++;
                    QThread::sleep(5);
                    stage++;
                    QThread::sleep(15);
                    stage++;
                    QThread::sleep(10);
                    stage++;
                    break;
                }
                case cross: {
                    QThread::sleep(1);
                    stage++;
                    break;
                }
                case stair: {
                    QThread::sleep(7);
                    stage++;
                    break;
                }
            }
            QThread::msleep(10);
            curr_mode = track;
            stage = 0;
        }

        cout << "Running Time: "
             << chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - start_time).count()
             << "s" << endl;
    };
};

const static int TICK = 50;

VideoCapture cap;
UdpUtil udp;
ColorGroup colorGroup;
DogPose dogPose;
TimerThread timer;

bool checkColorBarExist(Mat &curr_frame, int color, bool draw_line = DEBUG) {
    Mat color_frame, canny_frame;
    vector<cv::Vec2f> lines;
    inRange(curr_frame, colorGroup.get_min(color), colorGroup.get_max(color), color_frame);
    Canny(color_frame, canny_frame, 50, 150, 3);
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

            cv::line(curr_frame, pt1, pt2, Scalar(0, 0, 0), 5, cv::LINE_AA);
        }
    }
    return horizontal_lines > 2;
}

int getCenterLine(Mat &curr_frame, bool draw_line = DEBUG) {
    Mat color_frame, canny_frame;
    vector<cv::Vec4i> lines;
    inRange(curr_frame, colorGroup.get_min(white), colorGroup.get_max(white), color_frame);
    Canny(color_frame, canny_frame, 50, 150, 3);
    HoughLinesP(canny_frame, lines, 1, CV_PI / 180, 100, 200, 10);

    if (!lines.empty()) {
        // 选取两条最长的直线
        vector<cv::Vec4i> longestLines;
        sort(lines.begin(), lines.end(), [](const cv::Vec4i &a, const cv::Vec4i &b) {
            return cv::norm(cv::Point(a[0], a[1]) - cv::Point(a[2], a[3])) >
                   cv::norm(cv::Point(b[0], b[1]) - cv::Point(b[2], b[3]));
        });
        longestLines.emplace_back(lines[0]);
        longestLines.emplace_back(lines[1]);

        // 计算中线位置
        cv::Point middlePoint;
        middlePoint.x = (longestLines[0][2] + longestLines[1][2]) / 2;
        middlePoint.y = (longestLines[0][3] + longestLines[1][3]) / 2;

        if (draw_line) {
            // 在原始图像上绘制中线
            cv::line(curr_frame, cv::Point(longestLines[0][0], longestLines[0][1]),
                     cv::Point(longestLines[0][2], longestLines[0][3]), cv::Scalar(0, 0, 255), 2);
            cv::line(curr_frame, cv::Point(longestLines[1][0], longestLines[1][1]),
                     cv::Point(longestLines[1][2], longestLines[1][3]), cv::Scalar(0, 0, 255), 2);
            cv::circle(curr_frame, middlePoint, 5, cv::Scalar(0, 255, 0), -1);
        }

        return middlePoint.x;
    }
    return 200;
}

bool judgmentLeft(Mat &image, int i, int j) {
    if ((int) image.at<uchar>(i, j) == 0) {
        if ((int) image.at<uchar>(i, j + 3) == 255 && (int) image.at<uchar>(i, j + 5) == 255)
            return true;
        else
            return false;
    } else if ((int) image.at<uchar>(i, j) == 255 && j == 0) {
        if ((int) image.at<uchar>(i, j + 3) == 255 && (int) image.at<uchar>(i, j + 5) == 255)
            return true;
        else
            return false;
    } else {
        return false;
    }
}

bool judgmentRight(Mat &image, int i, int j) {
    if ((int) image.at<uchar>(i, j) == 0) {
        if ((int) image.at<uchar>(i, j - 3) == 255 && (int) image.at<uchar>(i, j - 5) == 255)
            return true;
        else
            return false;
    } else if ((int) image.at<uchar>(i, j) == 255 && j == 399) {
        if ((int) image.at<uchar>(i, j - 3) == 255 && (int) image.at<uchar>(i, j - 5) == 255)
            return true;
        else
            return false;
    } else {
        return false;
    }
}

int getAverage(Mat &frame) {
    int average = 200; // 中线均值
    int number = 0;
    int value[150] = {0}; // 存储每个点对应的x轴对应的值
    float sum = 0;
    int left[480] = {0};     // 存储左侧边缘
    float middle[480] = {0}; // 存储中线
    int right[480] = {0};    // 存储右侧边缘
    // 提取赛道左侧边缘
    for (int i = 0; i < frame.rows; i++) {
        for (int j = 0; j < frame.cols - 3; j++) {
            if (judgmentLeft(frame, i, j)) {
                if (j == 0)
                    left[i] = 1;
                else
                    left[i] = j;
                break;
            }
        }
    }
    // 提取赛道右侧边缘
    for (int i = 0; i < frame.rows; i++) {
        for (int j = frame.cols - 1; j >= 3; j--) {
            if (judgmentRight(frame, i, j)) {
                right[i] = j;
                break;
            }
        }
    }
    // 提取中线,并将中线绘制到图上
    number = 0;
    for (int i = 0; i < (frame.rows / 2.0); i++) {
        if ((left[i] < (left[i + 1] + 4)) && (left[i] > (left[i + 1] - 4)) && (left[i] != 0))
            if ((right[i] < (right[i + 1] + 4)) && (right[i] > (right[i + 1] - 4)) && (right[i] != 0)) {
                middle[i] = (left[i] + right[i]) / 2.0;
                frame.at<uchar>(i, (int) middle[i]) = 0;
                value[number] = middle[i];
                number++;
            }
    }
    // 计算中线平均值
    sum = 0;
    if (number != 0) {
        for (int i = 0; i < number; i++) {
            sum = sum + value[i];
        }
        average = sum / number;
        // 限幅
        if (average > 400 || average < 0) {
            average = 200;
        }
    }
    return average;
}

void ProcessFrame(Mat &frame) {
    Mat resized_frame, blur_frame;
    resize(frame, resized_frame, Size(400, 300));
    medianBlur(resized_frame, blur_frame, 5);

    Mat black_frame;
    inRange(blur_frame, colorGroup.get_min(white), colorGroup.get_max(white), black_frame);

    static float goal_average = 200; // 目标中线均值
    auto curr_average = (float) getAverage(blur_frame); // 当前中线均值

    // 颜色变换
    if (timer.stage == 0) {
        if (timer.next_color == blue && checkColorBarExist(black_frame, blue)) {
            timer.curr_mode = limit;
            if (timer.laps == 1) timer.next_color = violet;
            else timer.next_color = green;
        } else if (timer.next_color == violet && checkColorBarExist(black_frame, violet)) {
            timer.curr_mode = resident;
            timer.next_color = green;
        } else if (timer.next_color == green && checkColorBarExist(black_frame, green)) {
            timer.curr_mode = cross;
            if (timer.laps == 1) timer.next_color = yellow;
            else timer.next_color = red;
        } else if (timer.next_color == red && checkColorBarExist(black_frame, red)) {
            timer.curr_mode = resident;
            timer.next_color = yellow;
        } else if (timer.next_color == yellow && checkColorBarExist(black_frame, yellow)) {
            timer.curr_mode = cross;
            timer.next_color = red;
            timer.laps++;
        }
        timer.stage++;
    }


    // 动作变换
    if (timer.curr_mode == track) {
        goal_average = 200;
        dogPose.gesture_type = 3;
        dogPose.step_height = 0.03;
        dogPose.stand_height = 0.3;
        dogPose.rpy_des[0] = dogPose.rpy_des[1] = dogPose.rpy_des[2] = 0;
        dogPose.v_des[0] = 0.2;
        dogPose.v_des[1] = 0.001f * (goal_average - curr_average);
        dogPose.v_des[2] = 0.008f * (goal_average - curr_average);
    } else if (timer.curr_mode == limit) {
        goal_average = 200;
        dogPose.gesture_type = 3;
        dogPose.step_height = 0.01;
        dogPose.rpy_des[0] = dogPose.rpy_des[1] = dogPose.rpy_des[2] = 0;
        dogPose.v_des[0] = 0.2;
        dogPose.v_des[1] = 0.001f * (goal_average - curr_average);
        dogPose.v_des[2] = 0.006f * (goal_average - curr_average);

        if (timer.stage == 1)
            dogPose.stand_height = 0.2;
        else if (timer.stage == 2)
            dogPose.stand_height = 0.15;
        else if (timer.stage == 3) {
            dogPose.stand_height = 0.3;
        }
    } else if (timer.curr_mode == resident) {
        switch (timer.stage) {
            case 1:
                dogPose.stand_height = 0.3;
                dogPose.v_des[0] = 0.03;
                break;
            case 2:
                dogPose.stand_height = 0.3;
                dogPose.v_des[0] = 0.0;
                dogPose.gesture_type = 6;
                dogPose.rpy_des[0] = 0.3;
                dogPose.control_mode = 3;
                break;
            case 3:
                dogPose.gesture_type = 6;
                dogPose.step_height = 0.00;
                dogPose.stand_height = 0.3;
                dogPose.rpy_des[0] = 0;
                dogPose.rpy_des[1] = 0;
                dogPose.rpy_des[2] = 0;
                dogPose.v_des[0] = 0.0;
                dogPose.control_mode = 12;
                break;
            case 4:
                dogPose.control_mode = 3;
                break;
            case 5:
                dogPose.control_mode = 15;
                break;
            case 6:
                dogPose.control_mode = 12;
                break;
            case 7:
                dogPose.control_mode = 11;
                break;
        }
    } else if (timer.curr_mode == cross) {
        if (timer.stage == 1) {
            if (timer.laps == 1) {
                goal_average = 180;
                dogPose.gesture_type = 3;
                dogPose.step_height = 0.04;
                dogPose.stand_height = 0.3;
                dogPose.rpy_des[0] = 0;
                dogPose.rpy_des[1] = 0;
                dogPose.rpy_des[2] = 0;
                dogPose.v_des[0] = 0.2;
                dogPose.v_des[2] = -0.2;
            } else {
                goal_average = 180;
                dogPose.gesture_type = 3;
                dogPose.step_height = 0.03;
                dogPose.stand_height = 0.3;
                dogPose.rpy_des[0] = 0;
                dogPose.rpy_des[1] = 0;
                dogPose.rpy_des[2] = 0;
                dogPose.v_des[0] = 0.2;
                dogPose.v_des[2] = 0.2;
            }
        }
    } else if (timer.curr_mode == stair) {
        goal_average = 180;
        dogPose.gesture_type = 3;
        dogPose.step_height = 0.1;
        dogPose.stand_height = 0.3;
        dogPose.rpy_des[0] = dogPose.rpy_des[1] = dogPose.rpy_des[2] = 0;
        dogPose.v_des[0] = 0.3;
        dogPose.v_des[1] = 0.005f * (goal_average - curr_average);
    }
}

int main(int argc, char *argv[]) {
    bool showImage = false;
    // parse arguments
    if (argc >= 2 && argv[1] == string("show")) {
        showImage = true;
    }

    // open camera
    int camera_id;
    for (camera_id = 0; camera_id <= 3; camera_id++) {
        cap.open(camera_id);
        if (cap.isOpened())
            break;
    }
    if (!cap.isOpened())
        return -1;

    // unknown fucking stupid process
    auto *pLcmUtil = new lcmUtil;
    dogPose.control_mode = 12;
    pLcmUtil->send(dogPose, true);
    QThread::sleep(10);
    dogPose.control_mode = 11;
    pLcmUtil->send(dogPose, true);

    timer.start();
    Mat frame, resized_frame, blur_frame;
    while (timer.laps <= 3) {
        // reopen camera
        if (cap.get(CAP_PROP_HUE) == -1) {
            while (!cap.isOpened()) {
                for (camera_id = 0; camera_id <= 3; camera_id++) {
                    cap.open(camera_id);
                    if (cap.isOpened())
                        break;
                }
                QThread::msleep(100);
            }
        }

        cap >> frame;
        // if not empty
        if (!frame.empty()) {
            // preprocess frame
            resize(frame, resized_frame, Size(400, 300));
            //            medianBlur(resized_frame, blur_frame, 5);
            GaussianBlur(resized_frame, blur_frame, cv::Size(5, 5), 0);
            ProcessFrame(blur_frame);
            pLcmUtil->send(dogPose, true);
        }

        // show or send picture
        if (showImage) {
            colorGroup.showPicture(blur_frame);
            colorGroup.start(); // thread1

            switch (udp.ifReceiveInfoFlag) {
                case 0:
                    break;
                case 1: // choose color and return this color threshold
                    cout << "choose color and return this color threshold" << endl;
                    colorGroup.setColor(udp.color);
                    colorGroup.sendColorThreshold();
                    colorGroup.ifRunContinueFlag = true;
                    break;
                case 2: // set color threshold
                    cout << "set color threshold" << endl;
                    colorGroup.setColorThreshold(udp.colorThreadhold);
                    break;
                case 3: // save color threshold
                    cout << "save color threshold" << endl;
                    colorGroup.save();
                    break;
            }
            udp.ifReceiveInfoFlag = 0;
        }
        QThread::msleep((unsigned long) (1000 / TICK));
    }
    timer.stop();
    return 0;
}
