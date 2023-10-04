#ifndef LCMUTIL_H
#define LCMUTIL_H

#include <iostream>
#include <QString>
#include <lcm/lcm-cpp.hpp>
#include "robot_control_lcmt.hpp"

class lcmUtil {
public:
    lcmUtil();

    void send(float v_des[3], int gait_type, float step_height, float stand_height, float rpy_des[]);

    void send(int control_mode);

    robot_control_lcmt ctl;
};

#endif // LCMUTIL_H
