#include "motorThread.h"

bool mpc_initialised = 0, mpc_complete = 0, motor_init = 0;
double motor_comms_count = 0;
double demandedCurrent = 0; // was short
short inputCurrent = 0;
double currentPosition = 0, homePosition = 0, previousPosition = 0; // was long

MotorThread::MotorThread(QObject *parent)
	: QThread(parent)
{
}

void MotorThread::run() { // FUNCTION REQUIRES RESTRUCTURING
	group = lookup.getGroupFromNames({ "X8-9" }, { "X-80768" });
	if (!group) {
		throw "Group not found!Check that the family and name of a module on the network matches what is given in the source file.";
		terminate();
	}
	GroupCommand group_command(group->size());
	Eigen::VectorXd efforts(1);
	GroupFeedback group_feedback(group->size());
	group->setFeedbackFrequencyHz(500);

	motor_init = 1;

	while (!mpc_complete) {
		mutex.lock();
		if (!group->getNextFeedback(group_feedback)) {
			continue;
		}
		efforts[0] = -demandedCurrent;
		group_command.setEffort(efforts);
		group->sendCommand(group_command);
		auto pos = group_feedback.getPosition();
		currentPosition = -pos[0];
		motor_comms_count++;
		mutex.unlock();
	}
	// Stop logging
	auto log_file = group->stopLog();
}
		