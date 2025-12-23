#include "globals.h"

scs_log_t game_log = nullptr;

shifter_mode_t active_shifter = SM_UNKNOWN;
bool active_shifter_manual = false;

pos_t pos = POS_U;

bool active_brake = false;
// bool active_neutral = false;
float active_cc = 0.0;
