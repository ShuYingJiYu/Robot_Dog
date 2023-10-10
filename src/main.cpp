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
// 关闭颜色识别
static bool disable_color_change = false;

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
        return true;
    }
    return false;
}

//! \brief 最小二乘法
//! \param data 数据
//! \param len 数据长度
//! \return 斜率
double leastSquaresMethod(const int data[], int len) {
    double sum_x = 0, sum_y = 0, sum_xy = 0, sum_xx = 0;
    for (int i = 0; i < len; i++) {
        sum_x += i;
        sum_y += data[i];
        sum_xx += i * i;
        sum_xy += i * data[i];
    }
    double k = (len * sum_xy - sum_x * sum_y) / (len * sum_xx - sum_x * sum_x);
    return k;
}

int left_arr[300], right_arr[300];

void getCenterLine(Mat &curr_frame, int &center, double &k, bool draw_line = true) {
    // 获取道路边缘
    for (int i = 0; i < curr_frame.rows; i++) {
        int l, r;
        for (l = 1; l < curr_frame.cols - 1; l += 2) {
            int c = curr_frame.at<uchar>(i, l) +
                    curr_frame.at<uchar>(i, l + 1) +
                    curr_frame.at<uchar>(i, l - 1) +
                    curr_frame.at<uchar>(i, l + 2) +
                    curr_frame.at<uchar>(i, l - 2);
            if (c == 255 * 5)
                break;
        }
        for (r = curr_frame.cols - 2; r >= 1; r -= 2) {
            int c = curr_frame.at<uchar>(i, r) +
                    curr_frame.at<uchar>(i, r + 1) +
                    curr_frame.at<uchar>(i, r - 1) +
                    curr_frame.at<uchar>(i, r + 2) +
                    curr_frame.at<uchar>(i, r - 2);
            if (c == 255 * 5)
                break;
        }
        left_arr[i] = l;
        right_arr[i] = r;
    }

    // 算出中线
    int sum = 0;
    for (int i = 0; i < curr_frame.rows; i++) {
        int c = (left_arr[i] + right_arr[i]) >> 1;
        sum += c;
        if (draw_line)
            curr_frame.at<uchar>(i, c) = 0;
    }
    center = sum / curr_frame.rows;

    // 最小二乘法
    double left_k, right_k;
    left_k = leastSquaresMethod(left_arr, 300);
    right_k = leastSquaresMethod(right_arr, 300);
    k = (left_k + right_k) / 2;
}

void ProcessFrame() {
    // white binary frame
    PreProcessFrame(raw_frame, binary_frame, white);
    static int goal_average = 200;                       // 目标中线均值
    double k;
    int curr_average;
    getCenterLine(binary_frame, curr_average, k);   // 当前中线均值
    // cout << "curr_average: " << curr_average << endl;
    // cout << "curr_k" << k << endl;

    // 颜色变换
    if (timer.stage == 0 && timer.task != TASK_STOP && !disable_color_change) {
        if (timer.next_color == blue && checkColorBarExist(raw_frame, blue)) {
            cout << "recognized blue" << endl;
            timer.task = TASK_LIMIT;
            if (timer.laps == 1)
                timer.next_color = violet;
            else if(timer.laps == 2)
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
                timer.next_color = brown;
            timer.stage = 1;
        } else if (timer.next_color == brown && checkColorBarExist(raw_frame, brown)) {
            cout << "recognized brown" << endl;
            timer.task = TASK_RESIDENT;
            timer.next_color = yellow;
            timer.stage = 1;
        } else if (timer.next_color == yellow && checkColorBarExist(raw_frame, yellow)) {
            cout << "recognized yellow" << endl;
            timer.task = TASK_UPSTAIR;
            timer.next_color = blue;
            timer.laps++;
            timer.stage = 1;
        }
    }

    // 动作变换
    if (timer.task == TASK_STOP) {
        pose.gesture_type = GES_STAND;
        pose.v_des[0] = pose.v_des[1] = pose.v_des[2] = 0;
        pose.step_height = 0.04;
        pose.stand_height = 0.3;
    } else if (timer.task == TASK_TRACK) {
        goal_average = 185;
        pose.gesture_type = GES_FAST_WALK;
        pose.step_height = 0.18;
        pose.stand_height = 0.3;
        pose.v_des[1] = 0.00017f * (float) (curr_average - goal_average);
        pose.v_des[2] = 0.012f * (float) (goal_average - curr_average);
        if((timer.next_color == green or timer.next_color == yellow) and timer.stage == 0 )
            pose.v_des[0] = 0.45 - fabs(pose.v_des[2]);
        else
            pose.v_des[0] = 0.45;
//        pose.v_des[2] = (float) (0.8 * k);
        pose.rpy_des[0] = pose.rpy_des[1] = pose.rpy_des[2] = 0;
    } else if (timer.task == TASK_LIMIT) {
        goal_average = 200;
        pose.gesture_type = GES_SLOW_WALK;
        pose.step_height = 0.01;
        pose.v_des[0] = 0.2;
        pose.v_des[1] = 0.001f * (float) (curr_average - goal_average);
        pose.v_des[2] = 0.006f * (float) (goal_average - curr_average);
        pose.rpy_des[0] = pose.rpy_des[1] = pose.rpy_des[2] = 0;
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
                pose.step_height = 0.03;
                pose.stand_height = 0.3;
                pose.v_des[0] = 0.07;
                pose.v_des[1] = 0.0001f * (float) (curr_average - goal_average);
                pose.v_des[2] = 0.001f * (float) (goal_average - curr_average);
                // magic number
                break;
            case 2: // dump
                pose.gesture_type = GES_STAND;
                pose.control_mode = MODE_SQUAT;
                pose.stand_height = 0.3;
                pose.v_des[0] = 0.0;
                pose.rpy_des[0] = 0.3;
                break;
            case 3: // stable
                pose.gesture_type = GES_STAND;
                pose.control_mode = MODE_STABLE;
                pose.step_height = 0.00;
                pose.stand_height = 0.3;
                pose.v_des[0] = 0.0;
                pose.rpy_des[0] = pose.rpy_des[1] = pose.rpy_des[2] = 0;
                break;
            case 4: // squat
                pose.control_mode = MODE_SQUAT;
                break;
            case 5: // ring
                pose.control_mode = 15;
                break;
            case 6: // stable
                pose.control_mode = MODE_STABLE;
                break;
            case 7: // continue
                pose.control_mode = MODE_WALK;

                break;
        }
    } else if (timer.task == TASK_CROSS) {
        if (timer.stage == 1) {
            // 进入
            goal_average = 185;
            pose.gesture_type = GES_FAST_WALK;
            pose.step_height = 0.18;
            pose.stand_height = 0.3;
            pose.v_des[0] = 0.45;
            pose.v_des[1] = 0.00017f * (float) (curr_average - goal_average);
            pose.v_des[2] = 0.012f * (float) (goal_average - curr_average);
            pose.rpy_des[0] = pose.rpy_des[1] = pose.rpy_des[2] = 0;
        } else if (timer.stage == 2) {
            pose.gesture_type = GES_MEDIUM_WALK;
            pose.step_height = 0.03;
            pose.stand_height = 0.3;
            pose.v_des[0] = 0.3;
            goal_average = 200;
            pose.rpy_des[0] = pose.rpy_des[1] = pose.rpy_des[2] = 0;
            if (timer.laps == 1) {
                // left
                pose.v_des[2] = 0.19;
                cout<<"wo ri nm ri nm ri nm\n";

            } else if (timer.laps == 2 ) {
                // right
                pose.v_des[2] = -0.19;
                cout<<"wo cnm cnm cnm\n";
            }
        } else if (timer.stage == 3) {
            goal_average = 185;
            pose.gesture_type = GES_MEDIUM_WALK;
            pose.step_height = 0.04;
            pose.stand_height = 0.3;
            pose.v_des[0] = 0.3;
            goal_average = 180;
            pose.rpy_des[0] = pose.rpy_des[1] = pose.rpy_des[2] = 0;
            if (timer.laps == 1) {
                // left
                pose.v_des[2] = 0.013 * (goal_average - curr_average);
            } else if (timer.laps == 2){
                // right
                pose.v_des[2] = 0.013 * (goal_average - curr_average);
            }
        }

    } else if (timer.task == TASK_UPSTAIR) {
        if (timer.stage == 1) {
            goal_average = 185;
            pose.gesture_type = GES_FAST_WALK;
            pose.step_height = 0.18;
            pose.stand_height = 0.3;
            pose.v_des[0] = 0.45;
            pose.v_des[1] = 0.00017f * (float) (curr_average - goal_average);
            pose.v_des[2] = 0.012f * (float) (goal_average - curr_average);
            pose.rpy_des[0] = pose.rpy_des[1] = pose.rpy_des[2] = 0;
        } else if (timer.stage == 2) {
            goal_average = 195;
            pose.gesture_type = GES_SLOW_WALK;
            pose.step_height = 0.09;
            pose.stand_height = 0.3;
            pose.v_des[0] = 0.45;
            pose.v_des[2] = 0.0005f * (float) (goal_average - curr_average);
            // stupid 2
            pose.rpy_des[0] = pose.rpy_des[1] = pose.rpy_des[2] = 0;
        }
    }
}


int main(int argc, char *argv[]) {
    // 显示图像 default: false
    bool showImage = true;

    // parse arguments
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (argv[i] == string("showImage")) cout << "showImage" << endl, showImage = true, udp.start();
            if (argv[i] == string("disable")) cout << "disable color change" << endl, disable_color_change = true;

            // laps
            if (argv[i] == string("lap1")) cout << "laps 1" << endl, timer.laps = 1;
            if (argv[i] == string("lap2")) cout << "laps 2" << endl, timer.laps = 2;

            // mode
            if (argv[i] == string("stop")) cout << "stop" << endl, timer.task = TASK_STOP;
            if (argv[i] == string("track")) cout << "track" << endl, timer.task = TASK_TRACK;
            if (argv[i] == string("limit")) cout << "limit" << endl, timer.task = TASK_LIMIT;
            if (argv[i] == string("resident")) cout << "resident" << endl, timer.task = TASK_RESIDENT;
            if (argv[i] == string("upstair")) cout << "up stair" << endl, timer.task = TASK_UPSTAIR;

            // color
            if (argv[i] == string("blue")) cout << "next blue" << endl, timer.next_color = blue;
            if (argv[i] == string("yellow")) cout << "next yellow" << endl, timer.next_color = yellow;
            if (argv[i] == string("violet")) cout << "next violet" << endl, timer.next_color = violet;
            if (argv[i] == string("green")) cout << "next green" << endl, timer.next_color = green;
            if (argv[i] == string("red")) cout << "next red" << endl, timer.next_color = red;
            if (argv[i] == string("orange")) cout << "next orange" << endl, timer.next_color = orange;
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
    while (!timer.isInterruptionRequested()) {
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
