#include <QByteArray>
#include <QVector>
#include <QUdpSocket>
#include <QThread>
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
#include <fstream>

enum Color {
    yellow,
    white,
    blue,
    violet,
    brown,
    green,
    red,
    orange
};

#define goalIp "10.0.0.30"
#define nativeIp "10.0.0.34"

using namespace std;
using namespace cv;

QMutex qMutex;

unordered_map<string, int> color2index{
        {"yellow", yellow},
        {"white",  white},
        {"blue",   blue},
        {"violet", violet},
        {"brown",  brown},
        {"green",  green},
        {"red",    red},
        {"orange", orange}};
unordered_map<int, string> index2color{
        {yellow, "yellow"},
        {white,  "white"},
        {blue,   "blue"},
        {violet, "violet"},
        {brown,  "brown"},
        {green,  "green"},
        {red,    "red"},
        {orange, "orange"}};

class ColorGroup : public QThread {
public:
    vector<pair<Scalar, Scalar>> color_list{
            {Scalar(0, 0, 0),     Scalar(255, 255, 255)},     // yellow
            {Scalar(0, 155, 1),   Scalar(200, 200, 200)},   // white
            {Scalar(158, 109, 0), Scalar(255, 209, 84)},  // blue
            {Scalar(12, 123, 79), Scalar(93, 200, 149)},  // violet
            {Scalar(79, 74, 118), Scalar(255, 255, 255)}, // brown
            {Scalar(0, 0, 0),     Scalar(98, 116, 114)},      // green
            {Scalar(158, 109, 0), Scalar(255, 209, 84)},  // red
            {Scalar(0, 0, 0),     Scalar(255, 255, 255)},     // orange
    };

    ColorGroup() {
        // curr dir
        std::ifstream file("ColorGroup.txt");
        if (!file.is_open()) {
            cerr << "未能成功读取颜色阈值" << endl;
            return;
        } else
            cerr << "成功读取颜色阈值" << endl;

        string line, curr_color;
        int color_index, c_min[3], c_max[3];
        while (getline(file, line)) {
            istringstream iss(line);
            iss >> curr_color;

            for (int &i: c_min)
                iss >> i;
            for (int &i: c_max)
                iss >> i;

            if (color2index.find(curr_color) == color2index.end())
                continue;
            color_index = color2index[curr_color];
            color_list[color_index].first = Scalar(c_min[0], c_min[1], c_min[2]);
            color_list[color_index].second = Scalar(c_max[0], c_max[1], c_max[2]);
        }

        // 绑定当前的udp地址与端口
        udp_socket.bind(QHostAddress(nativeIp), 30014);
        // 设置视频编码时的编码参数
        parameter.push_back(IMWRITE_JPEG_QUALITY);
        parameter.push_back(50); // 进行50%的压缩
    };

    void save();

    // 显示图像，或者发送图像到上位机
    void showPicture(Mat &new_image) {
        image = new_image;
    }

    // 发送当前的阈值信息到上位机
    void sendColorThreshold() {
        for (int i = 0; i < 3; i++)
            message_array[i] = (char) color_list[color].first.val[i];
        for (int i = 0; i < 3; i++)
            message_array[i + 3] = (char) color_list[color].second.val[i];
        udp_socket.writeDatagram(message_array, QHostAddress(goalIp), 30017);
    };

    Scalar get_min(int c) {
        return color_list[c].first;
    }

    Scalar get_max(int c) {
        return color_list[c].second;
    }

    void setDestination(int new_destination) {
        destination = new_destination;
    };

    void setColor(int new_color) {
        color = new_color;
    };

    // 设置颜色的阈值
    void setColorThreshold(QByteArray &color_array) {
        color_list[color].first = Scalar(color_array[0] & 0xff, color_array[1] & 0xff, color_array[2] & 0xff);
        color_list[color].second = Scalar(color_array[3] & 0xff, color_array[4] & 0xff, color_array[5] & 0xff);

        cout << "color "
             << index2color[color]
             << " threshold set to: ";
        for (int i = 0; i <= 5; i++)
            cout << (color_array[i] & 0xff) << " ";
        cout << endl;
    };

    void run() override;

    bool ifRunContinueFlag = false;

private:
    QByteArray message_array;
    QUdpSocket udp_socket;
    vector<uchar> data_encode;
    vector<int> parameter;
    int color = red;
    Mat image;
    int destination = 0;
};

// 将当前的阈值信息保存到文本文档内
void ColorGroup::save() {
    // open file
    ofstream file("ColorGroup.txt");
    if (!file.is_open()) {
        cerr << "未能成功写入" << endl;
        return;
    } else
        cerr << "成功写入" << endl;

    for (auto &c: color_list) {
        file << index2color[color] << " ";
        for (int i = 0; i <= 2; i++)
            file << c.first.val[i] << " ";
        for (int i = 0; i <= 2; i++)
            file << c.second.val[i] << " ";
        file << endl;
    }

    file.close();
    qDebug() << "save successful";
}

// 执行图像发送（或者在当前界面显示图像）线程
void ColorGroup::run() {
    qMutex.lock();
    if (ifRunContinueFlag) {
        Mat resize_image, blur_image, trans_image;
        resize(image, resize_image, Size(400, 300));                                         // 改变图像尺寸为400*300
        medianBlur(resize_image, blur_image, 5);                                             // 中值滤波
        inRange(blur_image, color_list[color].first, color_list[color].second, trans_image); // 输出图像为单通道二值图
        if (destination == 0) {
            imshow("原图", resize_image);
            imshow("二值图像", trans_image);
            waitKey(1);
        } else if (destination == 1) {
            imencode(".jpg", resize_image, data_encode, parameter);
            for (unsigned int i = 0; i < data_encode.size(); i++)
                message_array[i] = (char) data_encode[i];
            udp_socket.writeDatagram(message_array, QHostAddress(goalIp), 30015);

            imencode(".jpg", trans_image, data_encode, parameter);
            for (unsigned int i = 0; i < data_encode.size(); i++)
                message_array[i] = (char) data_encode[i];
            udp_socket.writeDatagram(message_array, QHostAddress(goalIp), 30016);
        }
    }
    qMutex.unlock();
}