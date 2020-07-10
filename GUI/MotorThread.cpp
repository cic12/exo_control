#include "motorThread.h"

MotorThread::MotorThread(QObject *parent)
	: QThread(parent)
{
}

MotorThread::~MotorThread()
{

}

void MotorThread::run() {
	group = lookup.getGroupFromNames({ "X8-9" }, { "X-80768" });
	if (!group) {
		throw "Group not found!Check that the family and name of a module on the network matches what is given in the source file.";
		terminate();
	}
	GroupCommand group_command(group->size());
	Eigen::VectorXd efforts(1);
	GroupFeedback group_feedback(group->size());
	group->setFeedbackFrequencyHz(500);
	group->startLog("../res/logs");
	
	motor_initialised = true;
	
	while (!mpc_complete) {
		if (!group->getNextFeedback(group_feedback)) {
			continue;
		}
		mutex.lock();
		efforts[0] = -demandedTorque;
		group_command.setEffort(efforts);
		group->sendCommand(group_command);
		auto pos = group_feedback.getPosition();
		auto effort = group_feedback.getEffort();
		currentPosition = -pos[0];
		torque = -effort[0];
		motor_comms_count++;
		mutex.unlock();
	}
	auto log_file = group->stopLog();
	group->~Group();
}