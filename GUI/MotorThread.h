#pragma once

#include <QThread>
#include <QMutex>

#include "hebi.h"
#include "lookup.hpp"
#include "group_command.hpp"
#include "group_feedback.hpp"
#include "log_file.hpp"

using namespace hebi;

class MotorThread : public QThread
{
	Q_OBJECT

public:
	MotorThread(QObject *parent);
	void run();

	bool motor_initialised = false;
	bool mpc_complete = false;
	double demandedTorque = 0;
	double currentTorque = 0;
	long int motor_comms_count = 0;
	double time = 0;
	double currentPosition = 0.2 + 0.625 * M_PI;
	double previousPosition = 0;
	double currentVelocity = 0;
	double accelerometer[3] = { 0 , 0 , 0 };

private:
	Lookup lookup;
	std::shared_ptr<hebi::Group> group;
	QMutex mutex;
//signals:
//	void motor_run();
};