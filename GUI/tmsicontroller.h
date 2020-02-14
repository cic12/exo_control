#pragma once

#include "tmsisdk.h"
#include "daq.h"

#include <thread>
#include <chrono>
#include <QDebug>
#include <iomanip>
#include <fstream>
#include <QSettings>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <mutex>

class TMSiController
{
public:
	TMSiController();

	DAQ *daq;

	bool locateDLL();
	bool initialiseLibrary();
	bool findDevice();
	bool connect();
	bool freeDeviceListCall();
	bool getSignalFormatCall();
	bool setSampleRate(ULONG rateHz);

	ULONG getSampleRate();
	bool setRefCalculation(int in);

	bool startStream();
	bool endStream();
	bool createRecordingFile(QString fileName);
	bool addRecordingLine(float time, int* currentSample);
	bool endRecordingFile();

	void reset();

	int* queryPortTypes();

	std::atomic<float> currentReadTime{ 0 };
	std::atomic<bool> recording;
	int initialSample = 1;
	int currentSample[34];
	bool createdRecording = false;
	QString filePath;
	QString generateFilePath();
private:
	void streamProcess();

	HINSTANCE LibHandle;
	HANDLE Handle;
	int ErrorCode = 0;
	int DeviceCount;
	char FrontEndName[MAX_FRONTENDNAME_LENGTH];
	SIGNAL_FORMAT *signalFormat;
	char** DeviceList;
	ULONG SampleRateInMilliHz = MAX_SAMPLE_RATE;
	ULONG SignalBufferSizeInSamples = MAX_BUFFER_SIZE;
	unsigned int* SignalBuffer;
	int CurrentChannel = 0;
	int ErrorCount = 0;
	int ZeroCount = 0;

	int signalSet = 0;
	int signalTypes[32];

	std::mutex m_mutex;

	std::ofstream writeStream;

	int sampleRateHz;
	std::atomic<bool> streaming;

	QVector<float> timeStampsC;
	int previousSaw = 63;
	QVector<int> *record = new QVector<int>();
	bool safeStart = true;

	PLIBRARYINIT libraryInit;
	PGETDEVICELIST getDeviceList;
	PFREEDEVICELIST freeDeviceList;
	POPEN openConnection;
	PGETSIGNALFORMAT getSignalFormat;
	PSETSIGNALBUFFER setSignalBuffer;
	PSTART startStreaming;
	PGETSAMPLES getSamples;
	PSTOP stop;
	PCLOSE close;
	PLIBRARYEXIT libraryExit;
	PSETREFCALCULATION setRef;
};