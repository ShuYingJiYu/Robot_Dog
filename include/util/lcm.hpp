#include <iostream>
#include <lcm/lcm-cpp.hpp>

#include "lcm/robot_control_lcmt.hpp"
#include "dog.h"

using namespace std;

lcm::LCM lcm_socket("udpm://239.255.76.67:7667?ttl=1");

class LCMUtil {
public:
    LCMUtil() {

        if (!lcm_socket.good()) {
            cout << "LCM 初始化失败" << endl;
        } else {
            cout << "LCM 初始化成功" << endl;
        }
    };

    robot_control_lcmt ctl{};

    void send(DogPose pose, bool send_control_mode = true) {
        if (send_control_mode) {
            ctl.control_mode = pose.control_mode;
            lcm_socket.publish("voice_lcm", &ctl);
        }

        ctl.v_des[0] = pose.v_des[0];
        ctl.v_des[1] = pose.v_des[1];
        ctl.v_des[2] = pose.v_des[2];
        ctl.step_height_lcm = pose.step_height;
        ctl.stand_height_lcm = pose.stand_height;
        ctl.gait_type = pose.gesture_type;
        ctl.rpy_des[0] = pose.rpy_des[0];
        ctl.rpy_des[1] = pose.rpy_des[1];
        lcm_socket.publish("voice_lcm", &ctl);
    }
};