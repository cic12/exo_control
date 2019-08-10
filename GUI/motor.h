#ifndef MOTOR_H_
#define MOTOR_H_

#include <iostream>
#include "Definitions.h"
#include "MyThread.h"

extern bool mpc_complete;
extern short demandedCurrent;
extern short inputCurrent;
extern long currentPosition;
extern testParams test0;

void motorComms();
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