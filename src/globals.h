#ifndef GLOBALS_H
#define GLOBALS_H

#include <scssdk.h>

extern scs_log_t game_log;

enum shifter_mode_t {
    SM_UNKNOWN, SM_ARCADE, SM_AUTOMATIC, SM_MANUAL, SM_HSHIFTER
};

extern shifter_mode_t active_shifter;
extern bool active_shifter_manual;

enum pos_t {
    POS_M, POS_D, POS_N, POS_R, POS_P, POS_U
};

extern pos_t pos;
extern bool active_brake;
// extern bool active_neutral;
extern float active_cc;

#endif // GLOBALS_H
