#include "motorThread.h"

bool mpc_initialised = 0, mpc_complete = 0, motor_init = 0;
double motor_comms_count = 0;
double demandedCurrent = 0; // was short
short inputCurrent = 0;
double currentPosition = M_PI / 8 - M_PI / 2 - M_PI, homePosition = 0, previousPosition = 0; // was long

MotorThread::MotorThread(QObject *parent)
	: QThread(parent)
{
}

void MotorThread::run() { // FUNCTION REQUIRES RESTRUCTURING
	//if (test0.Device == 2) { 
	//	openDevice();
	//	long home = homePosition;
	//	definePosition(home); // Mini rig
	//	currentMode();
	//	long pos;
	//	getCurrentPosition(pos);
	//	currentPosition = pos;
	//	previousPosition = currentPosition / 168000.f + M_PI / 2;
	//}
	//else if (test0.Device == 1) { // statement requires mutex for syncing
		//Get group

	Lookup lookup;
	auto group = lookup.getGroupFromNames({ "X8-9" }, { "X-80768" });
	if (!group) {
		//std::cout
		//	<< "Group not found! Check that the family and name of a module on the network" << std::endl
		//	<< "matches what is given in the source file." << std::endl;
		throw "HEBI Error";
		terminate();
	}
	GroupCommand group_command(group->size());
	Eigen::VectorXd efforts(1);
	GroupFeedback group_feedback(group->size());
	group->setFeedbackFrequencyHz(500);

	motor_init = 1;

	while (!mpc_complete) {
		mutex.lock();
		//if (mpc_initialised) {
			//if (test0.Device == 2) {
			//	setCurrent(demandedCurrent);
			//	inputCurrent = demandedCurrent; // for debugging
			//	getCurrentPosition(pos); // wrong data type
			//	currentPosition = pos;
			//	motor_comms_count++;
			//}
			//else if (test0.Device == 1) {
		if (!group->getNextFeedback(group_feedback)) {
			continue;
		}
		//mutex.lock();
		efforts[0] = -demandedCurrent;
		group_command.setEffort(efforts);
		group->sendCommand(group_command);
		auto pos = group_feedback.getPosition();
		currentPosition = -pos[0];
		//mutex.unlock();
		motor_comms_count++;
		//}
		mutex.unlock();
	}
	// Stop logging
	auto log_file = group->stopLog();
}
		