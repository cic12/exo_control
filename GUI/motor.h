#ifndef MOTOR_H_
#define MOTOR_H_

#include "Definitions.h" // Maxon Epos header
#include "motorThread.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//extern double demandedCurrent;
//extern short inputCurrent;
//extern double currentPosition, homePosition, previousPosition;
//
//extern bool mpc_initialised;
//extern bool mpc_complete;
//extern double motor_comms_count;
//extern bool motor_init;

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