#include "tmsicontroller.h"

TMSiController::TMSiController()
{
	daq = new DAQ();

	locateDLL();
	initialiseLibrary();
	bool found = findDevice();
	bool connected = connect();
	bool getFormat = getSignalFormatCall();
	int* signaltypes = queryPortTypes();
	setSampleRate(MAX_SAMPLE_RATE);
	ULONG sampleRate = getSampleRate();
}

bool TMSiController::locateDLL()
{
	// Path to DLL
	TCHAR Path[MAX_PATH];
	GetSystemDirectory(Path, sizeof(Path) / sizeof(TCHAR));
	lstrcat(Path, L"\\TMSiSDK.dll");
	LibHandle = LoadLibrary(Path);
	return LibHandle;
}

bool TMSiController::initialiseLibrary()
{
	// TMSi functions
	libraryInit = (PLIBRARYINIT)GetProcAddress(LibHandle, "LibraryInit");
	getDeviceList = (PGETDEVICELIST)GetProcAddress(LibHandle, "GetDeviceList");
	freeDeviceList = (PFREEDEVICELIST)GetProcAddress(LibHandle, "FreeDeviceList");
	openConnection = (POPEN)GetProcAddress(LibHandle, "Open");
	getSignalFormat = (PGETSIGNALFORMAT)GetProcAddress(LibHandle, "GetSignalFormat");
	setSignalBuffer = (PSETSIGNALBUFFER)GetProcAddress(LibHandle, "SetSignalBuffer");
	startStreaming = (PSTART)GetProcAddress(LibHandle, "Start");
	getSamples = (PGETSAMPLES)GetProcAddress(LibHandle, "GetSamples");
	stop = (PSTOP)GetProcAddress(LibHandle, "Stop");
	close = (PCLOSE)GetProcAddress(LibHandle, "Close");
	libraryExit = (PLIBRARYEXIT)GetProcAddress(LibHandle, "LibraryExit");
	setRef = (PSETREFCALCULATION)GetProcAddress(LibHandle, "SetRefCalculation");

	Handle = libraryInit(TMSiConnectionUSB, &ErrorCode);
	return Handle;
}

bool TMSiController::findDevice()
{
	DeviceList = getDeviceList(Handle, &DeviceCount);
	return (DeviceCount > 0);
}

bool TMSiController::connect()
{
	bool con = openConnection(Handle, DeviceList[0]);
	if (con)
	{
		//Check maximum sample rate and buffer size
		setSignalBuffer(Handle, &SampleRateInMilliHz, &SignalBufferSizeInSamples);
	}
	SignalBufferSizeInSamples = 2176; // 16 sample min buffer.
	setRefCalculation(1);
	return con;
}

bool TMSiController::freeDeviceListCall()
{
	freeDeviceList(Handle, DeviceCount, DeviceList);
	return true;
}

bool TMSiController::getSignalFormatCall()
{
	signalFormat = getSignalFormat(Handle, FrontEndName);

	signalSet = 1;
	return !(signalFormat == nullptr);
}

int* TMSiController::queryPortTypes()
{
	//int types[32];

	for (int i = 0; i < 32; i++)
	{
		if (signalSet == 1)
			signalTypes[i] = signalFormat[i].Type;
		else {
			signalTypes[i] = 0;
		}
	}
	return signalTypes;
}

bool TMSiController::setSampleRate(ULONG rateHz)
{
	sampleRateHz = rateHz;
	SampleRateInMilliHz = rateHz * 1000;
	return setSignalBuffer(Handle, &SampleRateInMilliHz, &SignalBufferSizeInSamples); //Min buffer
}

ULONG TMSiController::getSampleRate()
{
	return SampleRateInMilliHz / 1000;
}

bool TMSiController::setRefCalculation(int in)
{
	return setRef(Handle, in);
}

bool TMSiController::startStream()
{
	// Create minimum buffer
	SignalBuffer = (unsigned int*)malloc(2176);
	if (startStreaming(Handle))
	{
		while (!safeStart)
		{

		}
		streaming.store(true);
		std::thread t1(&TMSiController::streamProcess, this);
		t1.detach();
		return true;
	}
	return false;
}

QString TMSiController::generateFilePath()
{
	QSettings settings("ICL", "TMSi");
	QDateTime now = QDateTime::currentDateTime();

	QString nowString = "recording_" + now.toString("dd_MM_yy__hh_mm");
	QString recording = settings.value("recording-name").toString() == "_NONE_" ? nowString : settings.value("recording-name").toString();

	QString project = settings.value("project").toString() == "_NONE_" ? "NoProject" : settings.value("project").toString();


	QString sessionString = "session_" + settings.value("boot-time").toString();
	QString session = settings.value("session").toString() == "_NONE_" ? sessionString : settings.value("session").toString();

	QString fullPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "//TMSiLite//" + project + "//" + session;

	//fullPath += nowString; // ADDED

	//if (!QDir().exists(fullPath))
		//qDebug() << QDir().mkdir(fullPath);
	//QString patient = settings.value("patient").toString() == "_NONE_" ? "NoPatient" : settings.value("patient").toString();

	//fullPath += "//" + patient;
	//Dir().mkdir(fullPath);

	fullPath += "//test.csv";
	qDebug() << fullPath;
	return fullPath;
}

void TMSiController::streamProcess()
{
	safeStart = false;
	while (streaming.load())
	{
		std::this_thread::sleep_for(std::chrono::nanoseconds(200));
		int ReturnedBytes = getSamples(Handle, (PULONG)SignalBuffer, 2176);
		if (ReturnedBytes < 0) {
			ErrorCount++;
		}
		else if (ReturnedBytes == 0)
		{
			ZeroCount++;
		}
		else if (ReturnedBytes > 0)
		{
			int ReturnedSamples = ReturnedBytes / (34 * 4);
			//For each sample - packet {1 x 32 reads + 2 internal channels}
			for (int sc = 0; sc < ReturnedSamples; sc++) //sample count
			{
				//For each channel
				m_mutex.lock();
				for (int ci = 0; ci < 34; ci++) //channel index
				{
					int bufferIndex = ci + (sc * 34);

					if (SignalBuffer[ci] == OVERFLOW_32BITS && (signalFormat[ci].Type == CHANNELTYPE_EXG || signalFormat[ci].Type == CHANNELTYPE_BIP || signalFormat[ci].Type == CHANNELTYPE_AUX))
					{
						currentSample[ci] = 0; // Set it to a value you find a good sign of a overflow
					}
					else
					{
						switch (signalFormat[CurrentChannel].Format)
						{
						case SF_UNSIGNED: // unsigned integer
							currentSample[ci] = (SignalBuffer[bufferIndex] * signalFormat[ci].UnitGain + signalFormat[ci].UnitOffSet);
							break;
						case SF_INTEGER: // signed integer
							currentSample[ci] = (((int)SignalBuffer[bufferIndex]) *  signalFormat[ci].UnitGain + signalFormat[ci].UnitOffSet);
							break;
						default:
							currentSample[ci] = 0; // For unknown types, set the value to zero
							break;
						}
					}
					record->push_back(currentSample[ci]);
				}

				m_mutex.unlock();
				if (initialSample == 0)
				{
					int ticks = (previousSaw == 63) ? (currentSample[33] + 1) / 2 : (currentSample[33] - previousSaw) / 2;
					float ms = float(ticks) / sampleRateHz * 1000;
					previousSaw = currentSample[33];
					currentReadTime.store(currentReadTime.load() + ms);
				}
				else
				{
					previousSaw = currentSample[33];
					initialSample = 0;
				}
				timeStampsC.push_back(currentReadTime.load());

				daq->AIdata[0] = (double)currentSample[16] / 1000;
				daq->AIdata[1] = (double)currentSample[17] / 1000;

				daq->AIm[0] = daq->emgProcess(daq->AIdata[0], 0);
				daq->AIm[1] = daq->emgProcess(daq->AIdata[1], 1);

				daq->daq_aiFile << daq->AIdata[0] << "," << daq->AIdata[1] << "," << daq->AIm[0] << "," << daq->AIm[1] << "\n";

				//std::this_thread::sleep_for(std::chrono::microseconds(500));

				//Sleep(1);
				//recording.load() ? fileManager->writeLine(currentReadTime.load(), currentSample) : 0;
				//if (createdRecording) {
					//addRecordingLine(currentReadTime, currentSample);
				//}

			}
		}
		//std::this_thread::sleep_for(std::chrono::nanoseconds(200));
	}

	safeStart = true;

	// End of Streaming
	qDebug() << "ENDED22";
	//endRecordingFile();
}

bool TMSiController::endStream()
{
	streaming.store(false);
	stop(Handle);
	currentReadTime = 0.0;
	initialSample = 1;
	return true;
}

void TMSiController::reset()
{
	currentReadTime = 0.0;
	timeStampsC.clear();
	record->clear();
	initialSample = 1;
}

bool TMSiController::createRecordingFile(QString fileName)
{
	std::string utf8_text = fileName.toUtf8().constData();
	if (QFile().exists(fileName))
	{
		fileName = fileName.split(".")[0] + "_2.csv";
		int rep = 2;
		while (QFile().exists(fileName))
		{
			int pos = fileName.lastIndexOf("_");
			fileName = fileName.left(pos);
			fileName = fileName + "_" + QString::number(rep) + ".csv";
			rep++;
		}
	}

	QFile file(fileName);
	file.open(QFile::ReadWrite);
	writeStream.open(utf8_text);

	//QStringList names = ChannelManager::getChannelNames();
	writeStream << "TIME(ms),";
	for (int i = 0; i < 32; i++)
	{
		QString name = "<name>";//names[i];
		writeStream << name.toLatin1().data() << ",";
	}
	writeStream << std::endl;

	qDebug() << fileName;
	return true;
}

bool TMSiController::addRecordingLine(float time, int* currentSample)
{
	writeStream << std::fixed << std::setprecision(3) << time << ",";
	for (int j = 0; j < 32; j++)
	{
		writeStream << currentSample[j] << ",";
	}
	writeStream << std::endl;
	return true;
}

bool TMSiController::endRecordingFile()
{
	writeStream.close();
	return true;
}