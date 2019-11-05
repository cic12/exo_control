#include "motorThread.h"

MotorThread::MotorThread(QObject *parent)
	: QThread(parent)
{
}

void MotorThread::run() {
	if (test0.Device == 2) {
		openDevice();
		long home = homePosition;
		definePosition(home); // Mini rig
		currentMode();
		long pos;
		getCurrentPosition(pos);
		currentPosition = pos;
		previousPosition = currentPosition / 168000.f + M_PI / 2;
	}
	else if (test0.Device == 1) {

	}
	//Get group
	Lookup lookup;
	auto group = lookup.getGroupFromNames({ "X8-9" }, { "X-80768" });
	if (!group) {
		//std::cout
		//	<< "Group not found! Check that the family and name of a module on the network" << std::endl
		//	<< "matches what is given in the source file." << std::endl;
		terminate();
	}
	GroupCommand group_command(group->size());
	Eigen::VectorXd efforts(1);
	GroupFeedback group_feedback(group->size());
	group->setFeedbackFrequencyHz(500); // Change to 500 Hz

	// Start logging in the background
	std::string log_path = group->startLog("./logs");

	if (log_path.empty()) {
		//std::cout << "~~ERROR~~\n"
		//	<< "Target directory for log file not found!\n"
		//	<< "HINT: Remember that the path declared in 'group->startLog()' "
		//	<< "is relative to your current working directory...\n";
		//return 1;
		terminate();
	}

	long pos;
	while (!mpc_complete) {
		if (mpc_initialised) {
			if (test0.Device == 2) {
				setCurrent(demandedCurrent);
				inputCurrent = demandedCurrent; // for debugging
				getCurrentPosition(pos); // wrong data type
				currentPosition = pos;
				motor_comms_count++;
			}
			else if (test0.Device == 1){
				if (!group->getNextFeedback(group_feedback)) {
					continue;
				}
				efforts[0] = demandedCurrent;
				group_command.setEffort(efforts);
				group->sendCommand(group_command);
				auto pos = group_feedback.getPosition();
				currentPosition = pos[0];
				motor_comms_count++;
			}
		}
	}

	// Stop logging
	auto log_file = group->stopLog();
}