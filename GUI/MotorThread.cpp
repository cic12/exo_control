#include "motorThread.h"

MotorThread::MotorThread(QObject *parent)
	: QThread(parent)
{
} 

void MotorThread::run() {
	 //Get group
		Lookup lookup;

		auto group = lookup.getGroupFromNames({ "X8-9" }, { "X-80768" });

		if (!group) {
			std::cout
				<< "Group not found! Check that the family and name of a module on the network" << std::endl
				<< "matches what is given in the source file." << std::endl;
			//return -1;
			terminate();
		}

		//// Open-loop controller (effort)

	  // The command struct has fields for various commands and settings; for the
	  // actuator, we will primarily use position, velocity, and effort.
	  //
	  // Fields that are not filled in will be ignored when sending.
		GroupCommand group_command(group->size());
		// GroupCommand uses Eigen types for data interchange
		Eigen::VectorXd efforts(1);
		// Allocate feedback
		GroupFeedback group_feedback(group->size());
		group->setFeedbackFrequencyHz(500); // Change to 500 Hz

		// Start logging in the background
		std::string log_path = group->startLog("./logs");

		if (log_path.empty()) {
			std::cout << "~~ERROR~~\n"
				<< "Target directory for log file not found!\n"
				<< "HINT: Remember that the path declared in 'group->startLog()' "
				<< "is relative to your current working directory...\n";
			//return 1;
			terminate();
		}

		while (!mpc_complete) {
			//motorComms();

			if (mpc_initialised) {
				//if (test0.Motor) {
				//	setCurrent(demandedCurrent);
				//	inputCurrent = demandedCurrent; // for debugging
				//	getCurrentPosition(currentPosition);
				//	motor_comms_count++;
				//}

				group->getNextFeedback(group_feedback);

				efforts[0] = demandedCurrent;
				group_command.setEffort(efforts);
				group->sendCommand(group_command);

				
				auto pos = group_feedback.getPosition();
				currentPosition = pos[0]*10000;
			}
		}
		// Stop logging
		auto log_file = group->stopLog();
}