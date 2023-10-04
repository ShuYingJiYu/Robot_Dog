#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>
#include <iostream>

enum Mode {
    import,
    residence,
    track,
    revise,
    upstair,
    limitHeight,
    upslope,
    obstruct,
    stop,
    divergeright,
    divergeleft,
    orange_stop
};

class myThread : public QThread {
public:
    myThread();

    void setParameter();

    void timerEvent(QTimerEvent *);

    void run() override;

    bool stripe = false;
    int residenceTransientProcess = 1;
    int limitHeightTransientProcess = 1;
    int divergeTransientProcess = 1;
    int diverge2TransientProcess = 1;
    bool limitEndAndQRcodeStartFlag = false;
    bool ok = true;
    int timerId;
    int mode = track;
};

#endif // MYTHREAD_H
