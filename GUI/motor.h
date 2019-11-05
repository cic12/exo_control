#ifndef MOTOR_H_
#define MOTOR_H_

#include "Definitions.h" // Maxon Epos header
#include "hebi.h"
#include "lookup.hpp" // THESE INCLUDES MAKE REFERENCING ESSENTIAL -> CHANGE TO EXTERNAL STATIC LIBRARY
#include "group_command.hpp" // "
#include "group_feedback.hpp" // "
#include "log_file.hpp" // "

extern double demandedCurrent;
extern short inputCurrent;
extern double currentPosition, homePosition, previousPosition;

extern bool mpc_initialised;
extern bool mpc_complete;
extern double motor_comms_count;

void enableDevice();
void disableDevice();
void openDevice();
void closeDevice();
void getOperationMode();
void currentMode();
void setCurrent(short TargetCurrent);
void getCurrentPosition(long& CurrentPosition);
void getCurrentVelocity(long& CurrentVelocity);
void definePosition(long& HomePosition);

#endif