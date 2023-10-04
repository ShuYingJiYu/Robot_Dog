// file: send_message.cpp
//
// LCM example program.
//
// compile with:
//  $ g++ -o send_message send_message.cpp -llcm
//
// On a system with pkg-config, you can also use:
//  $ g++ -o send_message send_message.cpp `pkg-config --cflags --libs lcm`

#include <iostream>
#include <unistd.h>//include sleep module
#include "lcmutil.h"

using namespace std;

lcm::LCM lcm1("udpm://239.255.76.67:7667?ttl=1");

lcmUtil::lcmUtil() {
    //lcm::LCM lcm("udpm://239.255.76.67:7667?ttl=1");

    if (!lcm1.good()) {
        cout << "LCM 初始化失败" << endl;
    } else {
        cout << "LCM 初始化成功" << endl;
    }
}

void lcmUtil::send(float v_des[3], int gait_type, float step_height, float stand_height, float rpy_des[]) {

    ctl.v_des[0] = v_des[0];//前进 <0.5
    ctl.v_des[1] = v_des[1];//横移 <abs(0.4)
    ctl.v_des[2] = v_des[2];//旋转 <abs(2)
    ctl.step_height_lcm = step_height;//提升至0.14即可
    ctl.stand_height_lcm = stand_height;//降低至0.2即可
    ctl.gait_type = gait_type;//运动是3，停止是4
    ctl.rpy_des[0] = rpy_des[0];//rpy_des[0](横滚角)
    ctl.rpy_des[1] = rpy_des[1];//rpy_des[1](俯仰角)
    lcm1.publish("robot_control_command", &ctl);
}

void lcmUtil::send(int control_mode) {
    ctl.control_mode = control_mode;
    lcm1.publish("robot_control_command", &ctl);
}
