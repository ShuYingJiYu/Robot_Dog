
#include "dog.h"
#include "util/color.hpp"

#include <chrono>
#include <string>

#include <QThread>

using namespace std;


class TimerThread : public QThread {
public:
    int task = TASK_STOP;
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
            switch (task) {
                case TASK_LIMIT: {
                    QThread::msleep(800);
                    stage = 2;
                    QThread::msleep(4000);
                    stage = 3;
                    QThread::msleep(800);
                    task = TASK_TRACK;
                    stage = 0;
                    break;
                }
                case TASK_RESIDENT: {
                    QThread::msleep(1500);
                    stage = 2;
                    QThread::msleep(2000);
                    stage = 3;
                    QThread::msleep(800);
                    stage = 4;
                    QThread::msleep(1500);
                    stage = 5;
                    QThread::sleep(7);
                    stage = 6;
                    QThread::msleep(5500);
                    stage = 7;
                    QThread::msleep(500);
                    task = TASK_TRACK;
                    stage = 0;
                    break;
                }
                case TASK_CROSS: {
                    QThread::msleep(400);
                    stage =2;
                    QThread::msleep(3000);
                    stage =3;
                    QThread::msleep(3000); //3000
                    task = TASK_TRACK;
                    stage = 0;
                    break;
                }
                case TASK_UPSTAIR: {
                    QThread::msleep(400);
                    stage = 2;
                    QThread::sleep(5);
                    task = TASK_TRACK;
                    stage = 0;
                    break;
                }
            }
            QThread::msleep(50);
        }
        cout << "Running Time: "
             << chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - start_time).count()
             << "s" << endl;
    };

    string DebugString() const {
        string str = "\nTimer:\n";
        str += "\ttask: " + std::to_string(task) + "\n";
        str += "\tnext_color: " + std::to_string(next_color) + "\n";
        str += "\tstage: " + std::to_string(stage) + "\n";
        str += "\tlaps: " + std::to_string(laps);
        return str;
    }
};
