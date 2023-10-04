
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
                    QThread::sleep(1);
                    stage++;
                    QThread::sleep(6);
                    stage++;
                    QThread::msleep(10);
                    task = TASK_TRACK;
                    stage = 0;
                    break;
                }
                case TASK_RESIDENT: {
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
                    task = TASK_TRACK;
                    stage = 0;
                    break;
                }
                case TASK_CROSS: {
                    QThread::sleep(1);
                    stage++;
                    QThread::msleep(10);
                    task = TASK_TRACK;
                    stage = 0;
                    break;
                }
                case TASK_UPSTAIR: {
                    QThread::sleep(7);
                    stage++;
                    QThread::msleep(10);
                    task = TASK_TRACK;
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

    string DebugString() const {
        string str = "\nTimer:\n";
        str += "\ttask: " + std::to_string(task) + "\n";
        str += "\tnext_color: " + std::to_string(next_color) + "\n";
        str += "\tstage: " + std::to_string(stage) + "\n";
        str += "\tlaps: " + std::to_string(laps);
        return str;
    }
};