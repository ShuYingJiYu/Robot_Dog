#ifndef QRCODE_RECOGNIZE_THREAD_H
#define QRCODE_RECOGNIZE_THREAD_H

#include <QThread>
#include <opencv2/opencv.hpp>
#include <opencv2/wechat_qrcode.hpp>

using namespace cv;

class QRcode_recognize_thread : public QThread {
public:
    QRcode_recognize_thread();

    void run() override;

    void setParameter(Mat &picture);

    void init_QRcode();

    Mat transformimage(Mat &originalImage);

    Mat image;
    Ptr<wechat_qrcode::WeChatQRCode> detector;
    std::vector<Mat> points;

private:
};


#endif // QRCODE_RECOGNIZE_THREAD_H
