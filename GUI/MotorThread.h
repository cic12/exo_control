#pragma once

#include <QThread>
#include "mpcThread.h" // For test config and motor refs

#include "hebi.h"
#include "lookup.hpp" // THESE INCLUDES MAKE REFERENCING ESSENTIAL -> CHANGE TO EXTERNAL STATIC LIBRARY
#include "group_command.hpp" // "
#include "group_feedback.hpp" // "
#include "log_file.hpp" // "

using namespace hebi;

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