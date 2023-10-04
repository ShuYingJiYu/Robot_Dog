#ifndef COLORGROUP_H
#define COLORGROUP_H

#include <opencv4/opencv2/opencv.hpp>
#include <QByteArray>
#include <QVector>
#include <QUdpSocket>
#include <QThread>

#define yellow 0
#define white 1
#define blue 2
#define violet 3
#define brown 4
#define green 5
#define red 6
#define orange 7
using namespace cv;
using namespace std;

class colorGroup : public QThread {
public:
    colorGroup();

    Scalar brownMin = Scalar(79, 74, 118);
    Scalar brownMax = Scalar(255, 255, 255);
    //黄色
    Scalar yellowMin = Scalar(0, 0, 0);
    Scalar yellowMax = Scalar(255, 255, 255);
    //绿色
    Scalar greenMin = Scalar(0, 0, 0);
    Scalar greenMax = Scalar(98, 116, 114);

    Scalar violetMin = Scalar(12, 123, 79);
    Scalar violetMax = Scalar(93, 200, 149);
    //蓝色
    Scalar blueMin = Scalar(158, 109, 0);
    Scalar blueMax = Scalar(255, 209, 84);
    //红色
    Scalar redMin = Scalar(158, 109, 0);
    Scalar redMax = Scalar(255, 209, 84);
    //
    Scalar whiteMin = Scalar(0, 155, 1);
    Scalar whiteMax = Scalar(200, 200, 200);

    Scalar orangeMin = Scalar(0, 0, 0);
    Scalar orangeMax = Scalar(255, 255, 255);
public:
    void save();

    void showPicture(Mat &image, int destination);

    void sendColorThreadhold();

    void chooseColor(int number);

    void setColorThreadhold(QByteArray colorarray);

    void run();

    bool ifRunContinueFlag = false;
private:
    QByteArray msg;
    QUdpSocket udpsocket;
    vector<uchar> data_encode;
    vector<int> parameter;
    int color = red;
    Mat image;
    int destination = 0;
};

#endif // COLORGROUP_H
