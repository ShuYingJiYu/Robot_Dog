#ifndef UDPUTIL_H
#define UDPUTIL_H

#include <QThread>

class UdpUtil : public QThread {
public:
    UdpUtil();

    int ifReceiveInfoFlag = 0;
    int color = 0;
    QByteArray colorThreadhold;
private:
    void run();

};

#endif // UDPUTIL_H
