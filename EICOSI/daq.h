#ifndef DAQ_H_
#define DAQ_H_

#include <stdio.h>
#include "NIDAQmx.h"

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

#endif