#pragma once

#include <QThread>
#include <QMutex>

#include "hebi.h"
#include "lookup.hpp" // THESE INCLUDES MAKE REFERENCING ESSENTIAL -> CHANGE TO EXTERNAL STATIC LIBRARY
#include "group_command.hpp" // "
#include "group_feedback.hpp" // "
#include "log_file.hpp" // "

using namespace hebi;

extern bool mpc_complete;

class MotorThread : public QThread
{
	Q_OBJECT

public:
	MotorThread(QObject *parent);
	void run();

	bool motor_init = 0;
	double demandedTorque = 0;
	double motor_comms_count = 0;
	double currentPosition = 0;
	double previousPosition = 0;
private:
	Lookup lookup;
	std::shared_ptr<hebi::Group> group;
	QMutex mutex;
};