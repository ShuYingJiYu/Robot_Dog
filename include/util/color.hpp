#ifndef COLOR_HPP
#define COLOR_HPP

#include <opencv4/opencv2/opencv.hpp>
#include <QByteArray>
#include <QVector>
#include <QUdpSocket>
#include <QThread>
#include "color.hpp"
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

#define goalIp "10.0.0.30"
#define nativeIp "10.0.0.34"

using namespace std;

QMutex qMutex;

enum Color {
    yellow,
    white,
    blue,
    violet,
    brown,
    green,
    red,
    orange,
    Color_COUNT,
};

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

using namespace cv;
using namespace std;

class ColorThread : public QThread {
public:
    ColorThread() {
        color_list.resize(Color_COUNT);
        color_list[yellow] = {{0,   0,   0},
                              {255, 255, 255}};
        color_list[white] = {{0,   155, 1},
                             {200, 200, 200}};
        color_list[blue] = {{158, 109, 0},
                            {255, 209, 84}};
        color_list[violet] = {{12, 123, 79},
                              {93, 200, 149}};
        color_list[brown] = {{79,  74,  118},
                             {255, 255, 255}};
        color_list[green] = {{0,  0,   0},
                             {98, 116, 114}};
        color_list[red] = {{158, 109, 0},
                           {255, 209, 84}};
        color_list[orange] = {{0,   0,   0},
                              {255, 255, 255}};

        read(); // 读取本地颜色阈值

        // 绑定当前的udp地址与端口
        udp_socket.bind(QHostAddress(nativeIp), 30014);
        // 设置视频编码时的编码参数
        parameter.push_back(IMWRITE_JPEG_QUALITY);
        parameter.push_back(50); // 进行50%的压缩
    };

    void save() {
        // open file
        ofstream file("ColorGroup.txt");
        if (!file.is_open()) {
            cerr << "未能成功写入" << endl;
            return;
        } else
            cerr << "成功写入" << endl;

        for (int i = 0; i < color_list.size(); i++) {
            file << index2color[i] << " ";
            for (int j = 0; j < 3; j++)
                file << color_list[i].first.val[j] << " ";
            for (int j = 0; j < 3; j++)
                file << color_list[i].second.val[j] << " ";
            file << endl;
        }

        file.close();
        cout << "save successful";
    };

    int get_color() const {
        return color;
    }

    Scalar &get_min(int c) {
        return color_list[c].first;
    }

    Scalar &get_max(int c) {
        return color_list[c].second;
    }

    pair<Scalar, Scalar> &get_color_pair(int c) {
        return color_list[c];
    }

    void showRawImage(Mat &image) {
        raw_img = image;
    };

    void showBinaryImage(Mat &image) {
        binary_img = image;
    };

    void setColor(int new_color) {
        color = new_color;
    };

    void setThreshold(const unsigned char *arr) {
        color_list[color].first = Scalar(arr[0] & 0xff, arr[1] & 0xff, arr[2] & 0xff);
        color_list[color].second = Scalar(arr[3] & 0xff, arr[4] & 0xff, arr[5] & 0xff);

        cout << "color "
             << index2color[color]
             << " set to: ";
        for (int i = 0; i < 6; i++)
            cout << (arr[i] & 0xff) << " ";
        cout << endl;
    };

    void sendThreshold() {
        for (int i = 0; i < 3; i++)
            message_array[i] = (char) color_list[color].first.val[i];
        for (int i = 0; i < 3; i++)
            message_array[i + 3] = (char) color_list[color].second.val[i];
        udp_socket.writeDatagram(message_array, QHostAddress(goalIp), 30017);
    };

    void run() override {
        qMutex.lock();
        if (destination == 0) {
            imshow("原图", raw_img);
            imshow("二值图像", binary_img);
            waitKey(1);
        } else if (destination == 1) {
            imencode(".jpg", raw_img, encoded_image, parameter);
            for (unsigned int i = 0; i < encoded_image.size(); i++)
                message_array[i] = (char) encoded_image[i];
            udp_socket.writeDatagram(message_array, QHostAddress(goalIp), 30015);

            imencode(".jpg", binary_img, encoded_image, parameter);
            for (unsigned int i = 0; i < encoded_image.size(); i++)
                message_array[i] = (char) encoded_image[i];
            udp_socket.writeDatagram(message_array, QHostAddress(goalIp), 30016);
        }
        qMutex.unlock();
    };

    string DebugString() const {
        string str = "\nColor:\n";
        str += "\tcolor: " + std::to_string(color) + "\n";
        for (auto &c: color_list) {
            str += "\t" + index2color[color] + ": ";
            for (int i = 0; i <= 2; i++)
                str += std::to_string(c.first.val[i]) + " ";
            for (int i = 0; i <= 2; i++)
                str += std::to_string(c.second.val[i]) + " ";
            str += "\n";
        }
        return str;
    };

    bool run_continue_flag = true;

private:
    vector<pair<Scalar, Scalar>> color_list{};
    vector<int> parameter;
    vector<uchar> encoded_image;
    QUdpSocket udp_socket;
    QByteArray message_array;
    int color = white;
    Mat raw_img, binary_img;

    int destination = 1;

    void read() {
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
    };
};


#endif // COLOR_HPP
