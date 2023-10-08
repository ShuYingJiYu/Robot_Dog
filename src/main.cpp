//
// Created by sakurapuare on 23-10-5.
//

#include <thread>
#include <chrono>

#include "dog.h"
#include "stage.hpp"
#include "util/color.hpp"
#include "util/udp.hpp"
#include "util/lcm.hpp"

#include <opencv4/opencv2/opencv.hpp>


using namespace cv;
using namespace std;

// 全局摄像头
VideoCapture cap;
// 全局图像
Mat raw_frame, binary_frame, processed_frame;
// 实例化色彩阈值类
ColorThread color;
// 实例化udp类
UdpThread udp;
// 狗的全局姿态
DogPose pose;
// 实例化lcm类
LCMUtil *pLcmUtil = new LCMUtil();
// 实例化定时器线程
TimerThread timer;
// 全局运行次数
static int running_count = 0;
// 全局核
static Mat kernel = getStructuringElement(MORPH_RECT, Size(2, 2));

void debug() {
    if (running_count % 20 == 0)
        cout << "\n  =====  running: " << running_count << "  =====  " << endl
             << timer.DebugString() << endl
             << pose.DebugString() << endl;
    running_count++;
}

void PreProcessFrame(Mat &before, Mat &after, int c) {
    // min, max
    Scalar min = color.get_min(c);
    Scalar max = color.get_max(c);

    inRange(before, min, max, after);
    erode(after, after, kernel, Point(-1, -1), 2);
    dilate(after, after, kernel, Point(-1, -1), 5);
    // morphology
//    morphologyEx(after, after, MORPH_OPEN, kernel);
//    morphologyEx(after, after, MORPH_CLOSE, kernel);
}


bool checkColorBarExist(Mat &curr_frame, int c, bool draw_line = true) {
    Mat color_frame, canny_frame;
    PreProcessFrame(curr_frame, color_frame, c);
    vector<cv::Vec2f> lines;
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

    if (horizontal_lines > 1) {
        cout << "horizontal_lines: " << horizontal_lines << endl;
        return horizontal_lines > 1;
    }
    return false;
}


int getCenterLine(Mat &curr_frame, bool draw_line = true) {
    int sum = 0;
    for (int i = 0; i < curr_frame.rows; i++) {
        int l, r;
        for (l = 1; l < curr_frame.cols - 1; l++) {
            int c = curr_frame.at<uchar>(i, l) +
                    curr_frame.at<uchar>(i, l + 1) +
                    curr_frame.at<uchar>(i, l - 1);
            if (c == 255 * 3)
                break;
        }
        for (r = curr_frame.cols - 2; r >= 1; r--) {
            int c = curr_frame.at<uchar>(i, r) +
                    curr_frame.at<uchar>(i, r + 1) +
                    curr_frame.at<uchar>(i, r - 1);
            if (c == 255 * 3)
                break;
        }

        int center = (l + r) >> 1;
        sum += center;
        if (draw_line)
            curr_frame.at<uchar>(i, center) = 0;
    }

    int average_line = sum / curr_frame.rows;
    if (average_line < 0 || average_line > curr_frame.rows)
        return curr_frame.rows / 2; // default value
    else
        return average_line;
}


void ProcessFrame() {

    // white binary frame
    PreProcessFrame(raw_frame, binary_frame, white);
    static float goal_average = 200;                       // 目标中线均值
    auto curr_average = (float) getCenterLine(binary_frame); // 当前中线均值
//    cout << "curr_average: " << curr_average << endl;

    // 颜色变换
    if (timer.stage == 0 && timer.task != TASK_STOP) {
        if (timer.next_color == blue && checkColorBarExist(raw_frame, blue)) {
            cout << "recognized blue" << endl;
            timer.task = TASK_LIMIT;
            if (timer.laps == 1)
                timer.next_color = violet;
            else
                timer.next_color = green;
            timer.stage = 1;
        } else if (timer.next_color == violet && checkColorBarExist(raw_frame, violet)) {
            cout << "recognized violet" << endl;
            timer.task = TASK_RESIDENT;
            timer.next_color = green;
            timer.stage = 1;
        } else if (timer.next_color == green && checkColorBarExist(raw_frame, green)) {
            cout << "recognized green" << endl;
            timer.task = TASK_CROSS;
            if (timer.laps == 1)
                timer.next_color = yellow;
            else
                timer.next_color = red;
            timer.stage = 1;
        } else if (timer.next_color == red && checkColorBarExist(raw_frame, red)) {
            cout << "recognized red" << endl;
            timer.task = TASK_RESIDENT;
            timer.next_color = yellow;
            timer.stage = 1;
        } else if (timer.next_color == yellow && checkColorBarExist(raw_frame, yellow)) {
            cout << "recognized yellow" << endl;
            timer.task = TASK_UPSTAIR;
            timer.next_color = red;
            timer.laps++;
            timer.stage = 1;
        }
    }

    // 动作变换
    if (timer.task == TASK_STOP) {
        pose.gesture_type = 6;
        pose.v_des[0] = pose.v_des[1] = pose.v_des[2];
        pose.step_height = 0.04;
        pose.stand_height = 0.3;
    } else if (timer.task == TASK_TRACK) {
        goal_average = 200;
        pose.gesture_type = 3;
        pose.step_height = 0.03;
        pose.stand_height = 0.3;
        pose.v_des[0] = 0.2;
        pose.v_des[1] = 0.001f * (goal_average - curr_average);
        pose.v_des[2] = 0.008f * (goal_average - curr_average);
        pose.rpy_des[0] = pose.rpy_des[1] = pose.rpy_des[2] = 0;
    } else if (timer.task == TASK_LIMIT) {
        goal_average = 200;
        pose.gesture_type = 3;
        pose.step_height = 0.01;
        pose.rpy_des[0] = pose.rpy_des[1] = pose.rpy_des[2] = 0;
        pose.v_des[0] = 0.2;
        pose.v_des[1] = 0.001f * (goal_average - curr_average);
        pose.v_des[2] = 0.006f * (goal_average - curr_average);

        if (timer.stage == 1)
            pose.stand_height = 0.2;
        else if (timer.stage == 2)
            pose.stand_height = 0.15;
        else if (timer.stage == 3) {
            pose.stand_height = 0.3;
        }
    } else if (timer.task == TASK_RESIDENT) {
        switch (timer.stage) {
            case 1: // prepare
                pose.stand_height = 0.3;
                pose.v_des[0] = 0.03;
                break;
            case 2: // dump
                pose.stand_height = 0.3;
                pose.v_des[0] = 0.0;
                pose.gesture_type = 6;
                pose.rpy_des[0] = 0.3;
                pose.control_mode = 3;
                break;
            case 3: // ?
                pose.gesture_type = 6;
                pose.step_height = 0.00;
                pose.stand_height = 0.3;
                pose.rpy_des[0] = 0;
                pose.rpy_des[1] = 0;
                pose.rpy_des[2] = 0;
                pose.v_des[0] = 0.0;
                pose.control_mode = MODE_STABLE;
                break;
            case 4:
                pose.control_mode = 3;
                break;
            case 5:
                pose.control_mode = 15;
                break;
            case 6:
                pose.control_mode = MODE_STABLE;
                break;
            case 7:
                pose.control_mode = MODE_WALK;
                break;
        }
    } else if (timer.task == TASK_CROSS) {
        if (timer.stage == 1) {
            goal_average = 200;
            pose.gesture_type = 3;
            pose.step_height = 0.03;
            pose.stand_height = 0.3;
            pose.rpy_des[0] = pose.rpy_des[1] = pose.rpy_des[2] = 0;
            pose.v_des[0] = 0.2;//前进
            pose.v_des[1] = 0.001f * (curr_average - goal_average);//横移
            pose.v_des[2] = 0.008f * (goal_average - curr_average);//转向
        } else {
            if (timer.laps == 1) {
                // left
                goal_average = 180;
                pose.gesture_type = 3;
                pose.step_height = 0.04;
                pose.stand_height = 0.3;
                pose.v_des[0] = 0.2;
                pose.v_des[2] = -0.2;
                pose.rpy_des[0] = pose.rpy_des[1] = pose.rpy_des[2] = 0;
            } else {
                // right
                goal_average = 180;
                pose.gesture_type = 3;
                pose.step_height = 0.03;
                pose.stand_height = 0.3;
                pose.v_des[0] = 0.2;
                pose.v_des[2] = 0.2;
                pose.rpy_des[0] = pose.rpy_des[1] = pose.rpy_des[2] = 0;
            }
        }

    } else if (timer.task == TASK_UPSTAIR) {
        goal_average = 180;
        pose.gesture_type = 3;
        pose.step_height = 0.1;
        pose.stand_height = 0.3;
        pose.v_des[0] = 0.3;
        pose.v_des[1] = 0.005f * (goal_average - curr_average);
        pose.rpy_des[0] = pose.rpy_des[1] = pose.rpy_des[2] = 0;
    }
}

int main(int argc, char *argv[]) {
    // 显示图像 default: false
    bool showImage = false;

    // parse arguments
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (argv[i] == string("showImage")) cout << "showImage" << endl, showImage = true, udp.start();

            // mode
            if (argv[i] == string("stop")) cout << "stop" << endl, timer.task = TASK_STOP;
            if (argv[i] == string("track")) cout << "track" << endl, timer.task = TASK_TRACK;
            if (argv[i] == string("limit")) cout << "limit" << endl, timer.task = TASK_LIMIT;
            if (argv[i] == string("resident")) cout << "resident" << endl, timer.task = TASK_RESIDENT;
            if (argv[i] == string("upstair")) cout << "up stair" << endl, timer.task = TASK_UPSTAIR;
        }
    }

    // open camera
    static int camera_id = 0;
    while (!cap.isOpened() && camera_id < 10)
        cap.open(camera_id), cout << "camera_id: " << camera_id << endl, camera_id++;
    if (!cap.isOpened()) return -1;

    // init dog pose and gesture
    cout << "init dog pose and gesture" << endl;
    pose.control_mode = MODE_STABLE;
    pLcmUtil->send(pose);
    this_thread::sleep_for(chrono::seconds(5));
    pose.control_mode = MODE_WALK;
    pLcmUtil->send(pose);


    // main thread
    timer.start_thread();
    while (true) {
        // reboot camera
        if (cap.get(CAP_PROP_FRAME_WIDTH) <= 0) {
            cap.release();
            // for 1 - 10 camera
            // each camera try 3 times
            for (int i = 0; i < 10; i++) {
                cout << "camera open failed, retrying..." << endl;
                cap.open(camera_id++);
                if (cap.get(CAP_PROP_FRAME_WIDTH) > 0) break;
            }

            // if still failed, exit
            if (cap.get(CAP_PROP_FRAME_WIDTH) <= 0) {
                cout << "camera open failed" << endl;
                return -1;
            }
        }

        cap >> raw_frame; // read raw_frame
        if (raw_frame.empty()) continue; // skip empty raw_frame

        resize(raw_frame, raw_frame, Size(400, 300), 0, 0, INTER_LINEAR);
        GaussianBlur(raw_frame, raw_frame, Size(5, 5), 0, 0);

        ProcessFrame();
        pLcmUtil->send(pose);

        // show image
        if (showImage) {
            processed_frame = raw_frame.clone();
            PreProcessFrame(processed_frame, processed_frame, color.get_color());
            color.showRawImage(raw_frame);
            color.showBinaryImage(processed_frame);
            color.start();

            if (udp.receive != 0)
                switch (udp.receive) {
                    case 1:
                        // choose color and return this color threshold
                        std::cout << "choose color and return this color threshold" << std::endl;
                        color.setColor(udp.color);
                        color.sendThreshold();
                        color.run_continue_flag = true;
                        break;
                    case 2:
                        // set color threshold
                        std::cout << "set color threshold" << std::endl;
                        color.setThreshold(udp.color_threshold);
                        break;
                    case 3:
                        // save color threshold
                        std::cout << "save color threshold" << std::endl;
                        color.save();
                        break;
                }
            udp.receive = 0;
        }

        debug();

        // sleep 20ms
        this_thread::sleep_for(chrono::milliseconds(20));
    }
    timer.stop_thread();
    cap.release();
    return 0;
}