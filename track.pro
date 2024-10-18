QT += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += src/main.cpp
HEADERS += include/dog.h include/stage.hpp include/util/color.hpp include/util/lcm.hpp include/util/udp.hpp

#导入lcm 的库
INCLUDEPATH += ./include
INCLUDEPATH += ./LCM
INCLUDEPATH += /usr/local/include
LIBS += -L/usr/local/lib/ -llcm
#导入opencv 的库
INCLUDEPATH += /usr/local/include/opencv4/
INCLUDEPATH += /usr/local/lib/
LIBS += /usr/local/lib/libopencv_videoio.so \
    -lopencv_calib \
    -lopencv_core \
    -lopencv_features2d \
    -lopencv_flann \
    -lopencv_highgui \
    -lopencv_imgproc \
    -lopencv_ml \
    -lopencv_objdetect \
    -lopencv_photo \
    -lopencv_stitching \
    -lopencv_wechat_qrcode
    -lopencv_video
LIBS +=/usr/local/lib/libopencv_core.so
LIBS +=/usr/local/lib/libopencv_imgcodecs.so
