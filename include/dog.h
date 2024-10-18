//
// Created by sakurapuare on 23-10-5.
//

#ifndef TRACKER_DOG_H
#define TRACKER_DOG_H

#include <cstdint>
#include <string>

using namespace std;

enum ModeType {
    MODE_DEFAULT = 0,
    MODE_SQUAT = 3,
    MODE_WALK = 11,
    MODE_STABLE = 12,
};

enum GestureType {
    GES_SLOW_WALK = 3,
    GES_MEDIUM_WALK = 5,
    GES_STAND = 6,
    GES_FAST_WALK = 9,
};

enum TaskType {
    TASK_STOP,
    TASK_TRACK,
    TASK_LIMIT,
    TASK_RESIDENT,
    TASK_CROSS,
    TASK_UPSTAIR,
    TASK_RESIDENT1,
    TASK_CROSS1,
    TASK_CROSS_LEFT,
    TASK_CROSS_RIGHT,
    TASK_RESIDENT_LEFT,
    TASK_RESIDENT_RIGHT,
    TASK_END
};


// @brief 机器狗姿态
class DogPose {
public:
    // 控制模式
    int32_t control_mode = MODE_DEFAULT;

    // 步态类型
    int32_t gesture_type = GES_MEDIUM_WALK;

    // 三个方向的速度
    float v_des[3] = {0, 0, 0};

    // 步高
    float step_height = 0.04;

    // 站立高度
    float stand_height = 0.3;

    // 三个方向的欧拉角
    float rpy_des[3] = {0, 0, 0};

    string DebugString() const {
        string str = "\nDogPose:\n";
        str += "\tcontrol_mode: " + std::to_string(control_mode) + "\n";
        str += "\tgesture_type: " + std::to_string(gesture_type) + "\n";
        str += "\tv_des: " + std::to_string(v_des[0]) + " " + std::to_string(v_des[1]) + " " +
               std::to_string(v_des[2]) + "\n";
        str += "\tstep_height: " + std::to_string(step_height) + "\n";
        str += "\tstand_height: " + std::to_string(stand_height) + "\n";
        str += "\trpy_des: " + std::to_string(rpy_des[0]) + " " + std::to_string(rpy_des[1]) + " " +
               std::to_string(rpy_des[2]);
        return str;
    }
};


#endif //TRACKER_DOG_H
