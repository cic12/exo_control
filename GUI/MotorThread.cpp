#include "motorThread.h"

MotorThread::MotorThread(QObject *parent)
	: QThread(parent)
{
}

MotorThread::~MotorThread()
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
	group->start

	motor_init = 1;
	
	while (!mpc_complete) {
		mutex.lock();
		if (!group->getNextFeedback(group_feedback)) {
			continue;
		}
		efforts[0] = -demandedTorque;
		group_command.setEffort(efforts);
		group->sendCommand(group_command);
		auto pos = group_feedback.getPosition();
		currentPosition = -pos[0];
		motor_comms_count++;
		mutex.unlock();
	}
	//group->
	auto log_file = group->stopLog();
}	