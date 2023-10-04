#include <iostream>
#include <lcm/lcm-cpp.hpp>

#include "robot_control_lcmt.hpp"

using namespace std;

lcm::LCM lcm_socket("udpm://239.255.76.67:7667?ttl=1");

class lcmUtil {
public:
    lcmUtil();

    robot_control_lcmt ctl{};

    void send(DogPose pose, bool send_control_mode);
};

lcmUtil::lcmUtil() {
    // lcm::LCM lcm("udpm://239.255.76.67:7667?ttl=1");

    if (!lcm_socket.good()) {
        cout << "LCM 初始化失败" << endl;
    } else {
        cout << "LCM 初始化成功" << endl;
    }
}

void lcmUtil::send(DogPose pose, bool send_control_mode = false) {
    if (send_control_mode) {
        ctl.control_mode = pose.control_mode;
        lcm_socket.publish("voice_lcm", &ctl);
    }

    ctl.v_des[0] = pose.v_des[0];             // 前进 <0.5
    ctl.v_des[1] = pose.v_des[1];             // 横移 <abs(0.4)
    ctl.v_des[2] = pose.v_des[2];             // 旋转 <abs(2)
    ctl.step_height_lcm = pose.step_height;   // 提升至0.14即可
    ctl.stand_height_lcm = pose.stand_height; // 降低至0.2即可
    ctl.gait_type = pose.gesture_type;        // 运动是3，停止是4
    ctl.rpy_des[0] = pose.rpy_des[0];         // rpy_des[0](横滚角)
    ctl.rpy_des[1] = pose.rpy_des[1];         // rpy_des[1](俯仰角)
    lcm_socket.publish("voice_lcm", &ctl);
}