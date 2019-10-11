#include "motor.h"

HANDLE keyHandle;
DWORD errorCode;
WORD nodeID;
__int8 mode;

using namespace std;

bool mpc_initialised = 0, mpc_complete = 0;
double motor_comms_count = 0;
double demandedCurrent = 0; // was short
short inputCurrent = 0;
double currentPosition = 0, homePosition = 0; // was long
testParams test0;

void motorComms() {
	if (mpc_initialised) {
		if (test0.Motor) {
			setCurrent(demandedCurrent);
			inputCurrent = demandedCurrent; // for debugging
			//getCurrentPosition(currentPosition); // wrong data type
			motor_comms_count++;
		}
	}
}

void enableDevice()
{
	errorCode = 0;
	int IsInFault = FALSE;

	if (VCS_GetFaultState(keyHandle, nodeID, &IsInFault, &errorCode))
	{
		if (IsInFault && !VCS_ClearFault(keyHandle, 1, &errorCode))
		{
			cout << "Clear fault failed! , error code=" << errorCode << endl;
			return;
		}

		errorCode = 0;
		int IsEnabled = FALSE;

		if (VCS_GetEnableState(keyHandle, nodeID, &IsEnabled, &errorCode))
		{
			if (!IsEnabled && !VCS_SetEnableState(keyHandle, nodeID, &errorCode))
			{
				cout << "Set enable state failed!, error code=" << errorCode << endl;
			}
			else
			{
				cout << "Enable succeeded!" << endl;
			}
		}
	}
	else
	{
		cout << "Get fault state failed!, error code, error code=" << errorCode << endl;
	}
}

void disableDevice() {
	errorCode = 0;
	int IsInFault = FALSE;

	if (VCS_GetFaultState(keyHandle, nodeID, &IsInFault, &errorCode))
	{
		if (IsInFault && !VCS_ClearFault(keyHandle, nodeID, &errorCode))
		{
			cout << "Clear fault failed!, error code=" << errorCode << endl;
			return;
		}

		int IsEnabled = FALSE;
		if (VCS_GetEnableState(keyHandle, nodeID, &IsEnabled, &errorCode))
		{
			if (IsEnabled && !VCS_SetDisableState(keyHandle, nodeID, &errorCode))
			{
				cout << "Set disable state failed!, error code=" << errorCode << endl;
			}
			else
			{
				cout << "Set disable state succeeded!" << endl;
			}
		}
	}
	else
	{
		cout << "Get fault state failed!, error code=" << errorCode << endl;
	}
}

void openDevice()
{
	char* deviceName = "EPOS2";
	char* protocolStackName = "MAXON SERIAL V2";
	char* interfaceName = "USB";
	char* portName = "USB0";
	unsigned long timeOut = 500;
	unsigned long baudRate = 1000000;

	errorCode = 0;

	keyHandle = VCS_OpenDevice(deviceName, protocolStackName, interfaceName, portName, &errorCode);

	if (keyHandle == 0)
	{
		cout << "Open device failure, error code=" << errorCode << endl;
	}
	else
	{
		cout << "Open device success!" << endl;
	}

	errorCode = 0;

	if (!VCS_SetProtocolStackSettings(keyHandle, baudRate, timeOut, &errorCode))
	{
		cout << "Set protocol stack settings failed!, error code=" << errorCode << endl;
		closeDevice();
	}

	enableDevice();
}

void closeDevice()
{
	disableDevice();

	errorCode = 0;

	cout << "Closing Device!" << endl;

	if (keyHandle != 0) {
		VCS_CloseDevice(keyHandle, &errorCode);
	}
	VCS_CloseAllDevices(&errorCode);
}

void getOperationMode() {
	errorCode = 0;
	if (!VCS_GetOperationMode(keyHandle, nodeID, &mode, &errorCode)) {
		cout << " error while getting operation mode , error code=" << errorCode << endl;
	}
	cout << "Operation Mode: " << static_cast<int>(mode) << endl;
}

void currentMode() {
	errorCode = 0;
	if (!VCS_ActivateCurrentMode(keyHandle, nodeID, &errorCode))
	{
		cout << "Activate current mode failed!" << endl;
	}
}

void setCurrent(short TargetCurrent) {
	errorCode = 0;
	if (!VCS_SetCurrentMust(keyHandle, nodeID, TargetCurrent, &errorCode))
	{
		cout << "Setting current failed!, error code=" << errorCode << endl;
	}
}

void getCurrentPosition(long& CurrentPosition) {
	errorCode = 0;
	if (!VCS_GetPositionIs(keyHandle, nodeID, &CurrentPosition, &errorCode)) {
		cout << " error while getting current position , error code=" << errorCode << endl;
	}
}

void getCurrentVelocity(long& CurrentVelocity) {
	errorCode = 0;
	if (!VCS_GetVelocityIsAveraged(keyHandle, nodeID, &CurrentVelocity, &errorCode)) {
		cout << " error while getting current velocity , error code=" << errorCode << endl;
	}
}

void definePosition(long& HomePosition) {
	errorCode = 0;
	if (!VCS_DefinePosition(keyHandle, nodeID, HomePosition, &errorCode)) {
		cout << " error while defining position , error code=" << errorCode << endl;
	}
}