#include "colorgroup.h"
#include <QFile>
#include <QDebug>
#include <QDir>
#include <QHostAddress>
#include <thread>
#include <QString>
#include <opencv4/opencv2/imgcodecs.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/opencv.hpp>
#include <QMutex>

#define goalIp "10.0.0.30"
#define nativeIp "10.0.0.34"
QMutex g_mutex;

colorGroup::colorGroup() {
    QFile file(QDir::toNativeSeparators(QDir::currentPath() + "/" + "colorGroup.txt"));
    // QFile file("colorGroup.txt");
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug() << "未能成功读取颜色阈值" << endl;
        return;
    } else {
        qDebug() << "成功读取颜色阈值" << endl;
    }
    //按行读取文本内的信息
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        QString str(line);
        QStringList list = str.split(" ");
        //黄色
        if (list[0] == "yellow") {
            yellowMin.val[0] = list[1].toInt();
            yellowMin.val[1] = list[2].toInt();
            yellowMin.val[2] = list[3].toInt();
            yellowMax.val[0] = list[4].toInt();
            yellowMax.val[1] = list[5].toInt();
            yellowMax.val[2] = list[6].toInt();
            //蓝色
        } else if (list[0] == "blue") {
            blueMin.val[0] = list[1].toInt();
            blueMin.val[1] = list[2].toInt();
            blueMin.val[2] = list[3].toInt();
            blueMax.val[0] = list[4].toInt();
            blueMax.val[1] = list[5].toInt();
            blueMax.val[2] = list[6].toInt();
            //紫色
        } else if (list[0] == "violet") {
            violetMin.val[0] = list[1].toInt();
            violetMin.val[1] = list[2].toInt();
            violetMin.val[2] = list[3].toInt();
            violetMax.val[0] = list[4].toInt();
            violetMax.val[1] = list[5].toInt();
            violetMax.val[2] = list[6].toInt();
            //棕色
        } else if (list[0] == "brown") {
            brownMin.val[0] = list[1].toInt();
            brownMin.val[1] = list[2].toInt();
            brownMin.val[2] = list[3].toInt();
            brownMax.val[0] = list[4].toInt();
            brownMax.val[1] = list[5].toInt();
            brownMax.val[2] = list[6].toInt();
            //绿色
        } else if (list[0] == "green") {
            greenMin.val[0] = list[1].toInt();
            greenMin.val[1] = list[2].toInt();
            greenMin.val[2] = list[3].toInt();
            greenMax.val[0] = list[4].toInt();
            greenMax.val[1] = list[5].toInt();
            greenMax.val[2] = list[6].toInt();
            //红色
        } else if (list[0] == "red") {
            redMin.val[0] = list[1].toInt();
            redMin.val[1] = list[2].toInt();
            redMin.val[2] = list[3].toInt();
            redMax.val[0] = list[4].toInt();
            redMax.val[1] = list[5].toInt();
            redMax.val[2] = list[6].toInt();
            //白色
        } else if (list[0] == "white") {
            whiteMin.val[0] = list[1].toInt();
            whiteMin.val[1] = list[2].toInt();
            whiteMin.val[2] = list[3].toInt();
            whiteMax.val[0] = list[4].toInt();
            whiteMax.val[1] = list[5].toInt();
            whiteMax.val[2] = list[6].toInt();
        } else if (list[0] == "orange") {
            orangeMin.val[0] = list[1].toInt();
            orangeMin.val[1] = list[2].toInt();
            orangeMin.val[2] = list[3].toInt();
            orangeMax.val[0] = list[4].toInt();
            orangeMax.val[1] = list[5].toInt();
            orangeMax.val[2] = list[6].toInt();
        }
    }
    //绑定当前的udp地址与端口
    udpsocket.bind(QHostAddress(nativeIp), 30014);
    //设置视频编码时的编码参数
    parameter.push_back(IMWRITE_JPEG_QUALITY);
    parameter.push_back(50);//进行50%的压缩
}

//将当前的阈值信息保存到文本文档内
void colorGroup::save() {
    qDebug() << "save successful" << endl;
    QFile file(QDir::toNativeSeparators(QDir::currentPath() + "/" + "colorGroup.txt"));
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug() << "未能成功写入" << endl;
        return;
    }
    QString dataAll = "";
    QString dataline = "";
    //save yellow
    dataline.append("yellow ");
    for (int i = 0; i <= 2; i++)
        dataline.append(QString::number(yellowMin.val[i]) + " ");
    for (int i = 0; i <= 1; i++)
        dataline.append(QString::number(yellowMax.val[i]) + " ");
    dataline.append(QString::number(yellowMax.val[2]) + "\n");
    dataAll.append(dataline);
    dataline.clear();
    //save blue
    dataline.append("blue ");
    for (int i = 0; i <= 2; i++)
        dataline.append(QString::number(blueMin.val[i]) + " ");
    for (int i = 0; i <= 1; i++)
        dataline.append(QString::number(blueMax.val[i]) + " ");
    dataline.append(QString::number(blueMax.val[2]) + "\n");
    dataAll.append(dataline);
    dataline.clear();
    //save violet
    dataline.append("violet ");
    for (int i = 0; i <= 2; i++)
        dataline.append(QString::number(violetMin.val[i]) + " ");
    for (int i = 0; i <= 1; i++)
        dataline.append(QString::number(violetMax.val[i]) + " ");
    dataline.append(QString::number(violetMax.val[2]) + "\n");
    dataAll.append(dataline);
    dataline.clear();
    //save green
    dataline.append("green ");
    for (int i = 0; i <= 2; i++)
        dataline.append(QString::number(greenMin.val[i]) + " ");
    for (int i = 0; i <= 1; i++)
        dataline.append(QString::number(greenMax.val[i]) + " ");
    dataline.append(QString::number(greenMax.val[2]) + "\n");
    dataAll.append(dataline);
    dataline.clear();
    //save brown
    dataline.append("brown ");
    for (int i = 0; i <= 2; i++)
        dataline.append(QString::number(brownMin.val[i]) + " ");
    for (int i = 0; i <= 1; i++)
        dataline.append(QString::number(brownMax.val[i]) + " ");
    dataline.append(QString::number(brownMax.val[2]) + "\n");
    dataAll.append(dataline);
    dataline.clear();
    //save red
    dataline.append("red ");
    for (int i = 0; i <= 2; i++)
        dataline.append(QString::number(redMin.val[i]) + " ");
    for (int i = 0; i <= 1; i++)
        dataline.append(QString::number(redMax.val[i]) + " ");
    dataline.append(QString::number(redMax.val[2]) + "\n");
    dataAll.append(dataline);
    dataline.clear();
    //save white
    dataline.append("white ");
    for (int i = 0; i <= 2; i++)
        dataline.append(QString::number(whiteMin.val[i]) + " ");
    for (int i = 0; i <= 1; i++)
        dataline.append(QString::number(whiteMax.val[i]) + " ");
    dataline.append(QString::number(whiteMax.val[2]) + "\n");
    dataAll.append(dataline);
    dataline.clear();
    dataline.append("orange ");
    for (int i = 0; i <= 2; i++)
        dataline.append(QString::number(orangeMin.val[i]) + " ");
    for (int i = 0; i <= 1; i++)
        dataline.append(QString::number(orangeMax.val[i]) + " ");
    dataline.append(QString::number(orangeMax.val[2]) + "\n");
    dataAll.append(dataline);
    dataline.clear();
    qDebug() << dataAll << endl;
    file.close();
    file.open(QIODevice::WriteOnly | QIODevice::Truncate);
    QTextStream in(&file);//写入
    in << dataAll;
    file.close();
    qDebug() << "save successful" << endl;
}

//显示图像，或者发送图像到上位机
void colorGroup::showPicture(Mat &im, int de) {
    image = im;
    destination = de;
}

//执行图像发送（或者在当前界面显示图像）线程
void colorGroup::run() {
    g_mutex.lock();
    if (ifRunContinueFlag) {
        Mat ZoomOutimage;
        Mat Blurimage;
        Mat transformImage;
        resize(image, ZoomOutimage, Size(400, 300));//改变图像尺寸为400*300
        medianBlur(ZoomOutimage, Blurimage, 5);//中值滤波
        switch (color) {
            case yellow:
                inRange(Blurimage, yellowMin, yellowMax, transformImage);//输出图像为单通道二值图
                break;
            case blue:
                inRange(Blurimage, blueMin, blueMax, transformImage);//输出图像为单通道二值图
                break;
            case green:
                inRange(Blurimage, greenMin, greenMax, transformImage);//输出图像为单通道二值图
                break;
            case violet:
                inRange(Blurimage, violetMin, violetMax, transformImage);//输出图像为单通道二值图
                break;
            case brown:
                inRange(Blurimage, brownMin, brownMax, transformImage);//输出图像为单通道二值图
                break;
            case red:
                inRange(Blurimage, redMin, redMax, transformImage);//输出图像为单通道二值图
                break;
            case white:
                inRange(Blurimage, whiteMin, whiteMax, transformImage);//输出图像为单通道二值图
                break;
            case orange:
                inRange(Blurimage, orangeMin, orangeMax, transformImage);//输出图像为单通道二值图
                break;
        }
        if (destination == 0) {
            imshow("原图", ZoomOutimage);
            imshow("二值图像", transformImage);
            waitKey(1);
        } else if (destination == 1) {
            imencode(".jpg", ZoomOutimage, data_encode, parameter);
            for (unsigned int i = 0; i < data_encode.size(); i++) {
                msg[i] = data_encode[i];
            }
            udpsocket.writeDatagram(msg, QHostAddress(goalIp), 30015);
            imencode(".jpg", transformImage, data_encode, parameter);
            for (unsigned int i = 0; i < data_encode.size(); i++) {
                msg[i] = data_encode[i];
            }
            udpsocket.writeDatagram(msg, QHostAddress(goalIp), 30016);
        }
    }
    g_mutex.unlock();
}

//发送当前的阈值信息到上位机
void colorGroup::sendColorThreadhold() {
    switch (color) {
        case yellow://yellow
            for (int i = 0; i < 3; i++)
                msg[i] = yellowMin.val[i];
            for (int i = 0; i < 3; i++)
                msg[i + 3] = yellowMax.val[i];
            break;
        case blue://blue
            for (int i = 0; i < 3; i++)
                msg[i] = blueMin.val[i];
            for (int i = 0; i < 3; i++)
                msg[i + 3] = blueMax.val[i];
            break;
        case green://green
            for (int i = 0; i < 3; i++)
                msg[i] = greenMin.val[i];
            for (int i = 0; i < 3; i++)
                msg[i + 3] = greenMax.val[i];
            break;
        case violet://violet
            for (int i = 0; i < 3; i++)
                msg[i] = violetMin.val[i];
            for (int i = 0; i < 3; i++)
                msg[i + 3] = violetMax.val[i];
            break;
        case brown://brown
            for (int i = 0; i < 3; i++)
                msg[i] = brownMin.val[i];
            for (int i = 0; i < 3; i++)
                msg[i + 3] = brownMax.val[i];
            break;
        case red://red
            for (int i = 0; i < 3; i++)
                msg[i] = redMin.val[i];
            for (int i = 0; i < 3; i++)
                msg[i + 3] = redMax.val[i];
            break;
        case white://red
            for (int i = 0; i < 3; i++)
                msg[i] = whiteMin.val[i];
            for (int i = 0; i < 3; i++)
                msg[i + 3] = whiteMax.val[i];
            break;
        case orange://red
            for (int i = 0; i < 3; i++)
                msg[i] = orangeMin.val[i];
            for (int i = 0; i < 3; i++)
                msg[i + 3] = orangeMax.val[i];
            break;
    }
    udpsocket.writeDatagram(msg, QHostAddress(goalIp), 30017);
}

//选择当前的颜色
void colorGroup::chooseColor(int Color) {
    color = Color;
}

//设置颜色的阈值
void colorGroup::setColorThreadhold(QByteArray colorarray) {
    switch (color) {
        case yellow:
            for (int i = 0; i <= 2; i++)
                yellowMin.val[i] = colorarray[i] & 0xff;
            for (int i = 0; i <= 2; i++)
                yellowMax.val[i] = colorarray[i + 3] & 0xff;
            std::cout << "yellow:";
            break;
        case blue:
            for (int i = 0; i <= 2; i++)
                blueMin.val[i] = colorarray[i] & 0xff;
            for (int i = 0; i <= 2; i++)
                blueMax.val[i] = colorarray[i + 3] & 0xff;
            std::cout << "blue:";
            break;
        case green:
            for (int i = 0; i <= 2; i++)
                greenMin.val[i] = colorarray[i] & 0xff;
            for (int i = 0; i <= 2; i++)
                greenMax.val[i] = colorarray[i + 3] & 0xff;
            std::cout << "green:";
            break;
        case violet:
            for (int i = 0; i <= 2; i++)
                violetMin.val[i] = colorarray[i] & 0xff;
            for (int i = 0; i <= 2; i++)
                violetMax.val[i] = colorarray[i + 3] & 0xff;
            std::cout << "violet:";
            break;
        case brown:
            for (int i = 0; i <= 2; i++)
                brownMin.val[i] = colorarray[i] & 0xff;
            for (int i = 0; i <= 2; i++)
                brownMax.val[i] = colorarray[i + 3] & 0xff;
            std::cout << "brown:";
            break;
        case red:
            for (int i = 0; i <= 2; i++)
                redMin.val[i] = colorarray[i] & 0xff;
            for (int i = 0; i <= 2; i++)
                redMax.val[i] = colorarray[i + 3] & 0xff;
            std::cout << "red:";
            break;
        case white:
            for (int i = 0; i <= 2; i++)
                whiteMin.val[i] = colorarray[i] & 0xff;
            for (int i = 0; i <= 2; i++)
                whiteMax.val[i] = colorarray[i + 3] & 0xff;
            std::cout << "white:";
            break;
        case orange:
            for (int i = 0; i <= 2; i++)
                orangeMin.val[i] = colorarray[i] & 0xff;
            for (int i = 0; i <= 2; i++)
                orangeMax.val[i] = colorarray[i + 3] & 0xff;
            std::cout << "orange:";
            break;
    }

    for (int i = 0; i <= 5; i++) {
        std::cout << (colorarray[i] & 0xff) << " ";
    }
    std::cout << std::endl;
}
