#include "mythread.h"
#include <QDebug>

myThread::myThread() {
    // timerId=startTimer(1000);
}

void myThread::setParameter() {
    //parameter=c;
}

void myThread::timerEvent(QTimerEvent *) {
    stripe = false;
    ok = true;
    killTimer(timerId);
}

void myThread::run() {
    while (1) {
        int i = 0;
        switch (mode) {
            case import://import
                QThread::msleep(5000);
                mode = upslope;
                break;
            case orange_stop:
                QThread::msleep(15000);
                mode = track;
                break;
            case residence://residence
                QThread::msleep(5000);
                residenceTransientProcess = 2;
                QThread::msleep(5000);
                residenceTransientProcess = 3;
                QThread::msleep(10000);
                residenceTransientProcess = 4;
                QThread::msleep(5000);
                residenceTransientProcess = 5;
                QThread::msleep(20000);
                residenceTransientProcess = 6;
                QThread::msleep(10000);
                residenceTransientProcess = 7;
                break;
            case upstair://upstair
                QThread::msleep(7000);
                mode = track;
                break;
            case limitHeight://limitHeight
                QThread::msleep(1000);
                limitHeightTransientProcess = 2;
                QThread::msleep(6000);
                limitHeightTransientProcess = 1;
                QThread::msleep(1000);
                mode = track;
                break;
            case upslope://upslope
                QThread::msleep(7000);
                mode = track;
                break;
            case obstruct://obstruct
                QThread::msleep(14000);
                mode = track;
                break;
            case divergeleft:
                divergeTransientProcess = 1;
                QThread::msleep(7000);
                divergeTransientProcess = 2;
                QThread::msleep(3000);
                mode = track;
                break;
            case divergeright:
                diverge2TransientProcess = 1;
                QThread::msleep(7000);
                diverge2TransientProcess = 2;
                QThread::msleep(3000);
                mode = track;
                break;
            default:
                QThread::msleep(10);
                break;
        }

    }
}
