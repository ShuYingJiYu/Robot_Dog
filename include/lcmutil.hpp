#include <iostream>
#include <lcm/lcm-cpp.hpp>

#include "robot_control_lcmt.hpp"

using namespace std;

lcm::LCM lcm_socket("udpm://239.255.76.67:7667?ttl=1");

class lcmUtil {
public:
    lcmUtil();

    robot_control_lcmt ctl{};

    void send(DogPose shit, bool send_control_mode);
};

lcmUtil::lcmUtil() {
    // lcm::LCM lcm("udpm://239.255.76.67:7667?ttl=1");

    if (!lcm_socket.good()) {
        cout << "LCM 初始化失败" << endl;
    } else {
        cout << "LCM 初始化成功" << endl;
    }
}

// void lcmUtil::send(const float v_des[3], int gait_type, float step_height, float stand_height, const float rpy_des[]) {
//     ctl.v_des[0] = v_des[0]; // 前进 <0.5
//     ctl.v_des[1] = v_des[1]; // 横移 <abs(0.4)
//     ctl.v_des[2] = v_des[2]; // 旋转 <abs(2)
//     ctl.step_height_lcm = step_height;// 提升至0.14即可
//     ctl.stand_height_lcm = stand_height;// 降低至0.2即可
//     ctl.gait_type = gait_type;// 运动是3，停止是4
//     ctl.rpy_des[0] = rpy_des[0];// rpy_des[0](横滚角)
//     ctl.rpy_des[1] = rpy_des[1];// rpy_des[1](俯仰角)
//     lcm_socket.publish("voice_lcm", &ctl);
// }
// void lcmUtil::send(int control_mode) {
//     ctl.control_mode = control_mode;
//     lcm_socket.publish("voice_lcm", &ctl);
// }

void lcmUtil::send(DogPose shit, bool send_control_mode = false) {
    if (send_control_mode) {
        ctl.control_mode = shit.control_mode;
        lcm_socket.publish("voice_lcm", &ctl);
    }

    ctl.v_des[0] = shit.v_des[0];             // 前进 <0.5
    ctl.v_des[1] = shit.v_des[1];             // 横移 <abs(0.4)
    ctl.v_des[2] = shit.v_des[2];             // 旋转 <abs(2)
    ctl.step_height_lcm = shit.step_height;   // 提升至0.14即可
    ctl.stand_height_lcm = shit.stand_height; // 降低至0.2即可
    ctl.gait_type = shit.gesture_type;        // 运动是3，停止是4
    ctl.rpy_des[0] = shit.rpy_des[0];         // rpy_des[0](横滚角)
    ctl.rpy_des[1] = shit.rpy_des[1];         // rpy_des[1](俯仰角)
    lcm_socket.publish("voice_lcm", &ctl);
}