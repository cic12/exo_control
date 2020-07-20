#include "motorThread.h"

MotorThread::MotorThread(QObject *parent)
	: QThread(parent)
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
		auto t = group_feedback.getTime();
		auto pos = group_feedback.getPosition();
		auto vel = group_feedback.getVelocity();
		auto effort = group_feedback.getEffort();
		auto accel = group_feedback.getAccelerometer();
		auto orient = group_feedback[0].mobile().arOrientation().get();
		time = t;
		currentPosition = -pos[0];
		currentVelocity = -vel[0];
		currentTorque = -effort[0];
		accelerometer[0] = accel(0,0);
		accelerometer[1] = accel(0,1);
		accelerometer[2] = accel(0,2);
		orientation[0] = orient.getW();
		orientation[1] = orient.getX();
		orientation[2] = orient.getY();
		orientation[3] = orient.getZ();
		motor_comms_count++;
		mutex.unlock();
	}
	auto log_file = group->stopLog(); // collect log file name for test name
	group->~Group();
}