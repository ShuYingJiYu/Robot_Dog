//using namespace std;

enum DogModeType {
    STAND = 11,
    HALF_SQUAT = 12,
};

enum TaskMode {
    STOP,
    TRACK,
    LIMIT,
    RESIDENT,
    CROSS,
    STAIR,
};

class DogPose {
public:
    // gesture type
    int gesture_type = 3;
    // control mode
    int control_mode = 0;
    // default to 0.04
    float step_height = 0.04;
    // default to 0.3
    float stand_height = 0.3;
    // speed x y z axes0.2, 0, 0
    float v_des[3] = {0.0, 0, 0};
    // 俯仰
    float rpy_des[3] = {0, 0, 0};
};
