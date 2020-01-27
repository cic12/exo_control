#pragma once

#include <QThread>
#include <QMutex>

#include "hebi.h"
#include "lookup.hpp" // THESE INCLUDES MAKE REFERENCING ESSENTIAL -> CHANGE TO EXTERNAL STATIC LIBRARY
#include "group_command.hpp" // "
#include "group_feedback.hpp" // "
#include "log_file.hpp" // "

using namespace hebi;

extern double demandedCurrent;
extern short inputCurrent;
extern double currentPosition, homePosition, previousPosition;

extern bool mpc_initialised;
extern bool mpc_complete;
extern double motor_comms_count;
extern bool motor_init;

class MotorThread : public QThread
{
	Q_OBJECT

public:
	MotorThread(QObject *parent);
	void run();

private:
	std::shared_ptr<hebi::Group> group;
	GroupCommand *group_command;
	GroupFeedback *group_feedback;
	Eigen::VectorXd efforts;
	QMutex mutex;
};