#ifndef MOTOR_H_
#define MOTOR_H_

#include <iostream>
#include "Definitions.h"

void enableDevice();

void disableDevice();

void openDevice();

void closeDevice();

//void moveDevice(long TargetPosition, int absolute, int immediately);

void getOperationMode();

void currentMode();

void setCurrent(short TargetCurrent);

void getCurrentPosition(long& CurrentPosition);

void getCurrentVelocity(long& CurrentVelocity);

void definePosition(long& HomePosition);

//void getPositionObject(int& CurrentPos);

#endif