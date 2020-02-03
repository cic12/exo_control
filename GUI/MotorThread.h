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
	~MotorThread();
	void run();

	bool motor_init = 0;
	bool mpc_complete = 0;
	double demandedTorque = 0;
	double motor_comms_count = 0;
	double currentPosition = 0;
	double previousPosition = 0;
private:
	Lookup lookup;
	std::shared_ptr<hebi::Group> group;
	QMutex mutex;
};