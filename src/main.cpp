#define DEBUG true

#include <chrono>

#include "dog.hpp"
#include "udputil.hpp"
#include "colorgroup.hpp"
#include "lcmutil.hpp"

#include <QCoreApplication>
#include <QThread>
#include <opencv4/opencv2/opencv.hpp>

using namespace std;
using namespace cv;


class TimerThread : public QThread {
public:
    int curr_mode = STOP;
    int next_color = blue;
    int stage = 0;
    int laps = 1;

    chrono::time_point<chrono::steady_clock> start_time;

    void start_thread() {
        start_time = chrono::steady_clock::now();
        QThread::start();
    }

    void stop_thread() {
        requestInterruption();
        QThread::wait();
    }

    void run() override {
        // while thread is running
        while (!isInterruptionRequested()) {
            switch (curr_mode) {
                case LIMIT: {
                    QThread::sleep(1);
                    stage++;
                    QThread::sleep(6);
                    stage++;
                    QThread::msleep(10);
                    curr_mode = TRACK;
                    stage = 0;
                    break;
                }
                case RESIDENT: {
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
                    QThread::msleep(10);
                    curr_mode = TRACK;
                    stage = 0;
                    break;
                }
                case CROSS: {
                    QThread::sleep(1);
                    stage++;
                    QThread::msleep(10);
                    curr_mode = TRACK;
                    stage = 0;
                    break;
                }
                case STAIR: {
                    QThread::sleep(7);
                    stage++;
                    QThread::msleep(10);
                    curr_mode = TRACK;
                    stage = 0;
                    break;
                }
            }
            QThread::msleep(10);
        }

        cout << "Running Time: "
             << chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - start_time).count()
             << "s" << endl;
    };
};

const static int TICK = 50;

VideoCapture cap;
UdpUtil udp;
ColorGroup color;
DogPose dogPose;
TimerThread timer;

bool checkColorBarExist(Mat &curr_frame, int c, bool draw_line = DEBUG) {
    Mat color_frame, canny_frame;
    vector<cv::Vec2f> lines;
    inRange(curr_frame, color.get_min(c), color.get_max(c), color_frame);
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


int getCenterLine(Mat &frame, bool draw_line = DEBUG) {
    int sum = 0;
    for (int i = 0; i < frame.rows; i++) {
        int l, r;
        for (l = 1; l < frame.cols - 1; l++) {
            int c = frame.at<uchar>(i, l) +
                    frame.at<uchar>(i, l + 1) +
                    frame.at<uchar>(i, l - 1);
            if (c == 255 * 3)
                break;
        }
        for (r = frame.cols - 2; r >= 1; r--) {
            int c = frame.at<uchar>(i, r) +
                    frame.at<uchar>(i, r + 1) +
                    frame.at<uchar>(i, r - 1);
            if (c == 255 * 3)
                break;
        }

        int center = (l + r) >> 1;
        sum += center;
        if (draw_line)
            frame.at<uchar>(i, center) = 0;
    }

    int average_line = sum / frame.rows;
    if (average_line < 0 || average_line > frame.rows)
        return frame.rows / 2; // default value
    else
        return average_line;
}

void ProcessFrame(Mat &frame) {
    Mat resized_frame, blur_frame;
    resize(frame, resized_frame, Size(400, 300));
    medianBlur(resized_frame, blur_frame, 5);

    Mat black_frame;
    inRange(blur_frame, color.get_min(white), color.get_max(white), black_frame);

    static float goal_average = 200; // 目标中线均值
    auto curr_average = (float) getCenterLine(blur_frame); // 当前中线均值

    // 颜色变换
    if (timer.stage == 0) {
        if (timer.next_color == blue && checkColorBarExist(black_frame, blue)) {
            timer.curr_mode = LIMIT;
            if (timer.laps == 1) timer.next_color = violet;
            else timer.next_color = green;
        } else if (timer.next_color == violet && checkColorBarExist(black_frame, violet)) {
            timer.curr_mode = RESIDENT;
            timer.next_color = green;
        } else if (timer.next_color == green && checkColorBarExist(black_frame, green)) {
            timer.curr_mode = CROSS;
            if (timer.laps == 1) timer.next_color = yellow;
            else timer.next_color = red;
        } else if (timer.next_color == red && checkColorBarExist(black_frame, red)) {
            timer.curr_mode = RESIDENT;
            timer.next_color = yellow;
        } else if (timer.next_color == yellow && checkColorBarExist(black_frame, yellow)) {
            timer.curr_mode = CROSS;
            timer.next_color = red;
            timer.laps++;
        }
        timer.stage++;
    }


    // 动作变换
    if (timer.curr_mode == STOP) {
        dogPose.gesture_type = 6;
        dogPose.v_des[0] = 0;
        dogPose.v_des[1] = 0;
        dogPose.v_des[2] = 0;
        dogPose.step_height = 0.04;
        dogPose.stand_height = 0.3;
    } else if (timer.curr_mode == TRACK) {
        goal_average = 200;
        dogPose.gesture_type = 3;
        dogPose.step_height = 0.03;
        dogPose.stand_height = 0.3;
        dogPose.rpy_des[0] = dogPose.rpy_des[1] = dogPose.rpy_des[2] = 0;
        dogPose.v_des[0] = 0.2;
        dogPose.v_des[1] = 0.001f * (goal_average - curr_average);
        dogPose.v_des[2] = 0.008f * (goal_average - curr_average);
    } else if (timer.curr_mode == LIMIT) {
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
    } else if (timer.curr_mode == RESIDENT) {
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
    } else if (timer.curr_mode == CROSS) {
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
    } else if (timer.curr_mode == STAIR) {
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
#ifndef DEBUG
    freopen("/dev/null", "w", stdout);
#endif

    bool showImage = false;

    // parse arguments
    if (argc >= 2) {
        for (int i = 1; i < argc; i++) {
            if (argv[i] == string("stop")) timer.curr_mode = STOP;
            if (argv[i] == string("track")) timer.curr_mode = TRACK;

            if (argv[i] == string("showImage")) showImage = true;
        }
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
    cout << "opened camera: " << camera_id << endl;

    auto *pLcmUtil = new lcmUtil;

    // start to balance
    cout << "start to balance";
    dogPose.control_mode = HALF_SQUAT;
    pLcmUtil->send(dogPose, true);
    QThread::sleep(5);
    dogPose.control_mode = STAND;
    pLcmUtil->send(dogPose, true);
    QThread::sleep(5);

    // start main
    timer.start_thread();
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
            color.showPicture(blur_frame);
            color.start(); // thread1

            switch (udp.ifReceiveInfoFlag) {
                case 0:
                    break;
                case 1: // choose color and return this color threshold
                    cout << "choose color and return this color threshold" << endl;
                    color.setColor(udp.color);
                    color.sendColorThreshold();
                    color.ifRunContinueFlag = true;
                    break;
                case 2: // set color threshold
                    cout << "set color threshold" << endl;
                    color.setColorThreshold(udp.colorThreadhold);
                    break;
                case 3: // save color threshold
                    cout << "save color threshold" << endl;
                    color.save();
                    break;
            }
            udp.ifReceiveInfoFlag = 0;
        }
        QThread::msleep((unsigned long) (1000 / TICK));
    }
    timer.stop_thread();
    return 0;
}
