/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2019 Allen Institute for Brain Science and Open Ephys

------------------------------------------------------------------

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <chrono>

#include "NIDAQComponents.h"

#define ERR_BUFF_SIZE 2048
#define STR2CHR( jString ) ((jString).toUTF8())
#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

NIDAQ::int32 CVICALLBACK EveryNCallbackWrapper(NIDAQ::TaskHandle taskHandle, NIDAQ::int32 everyNsamplesEventType, NIDAQ::uInt32 nSamples, void *callbackData) {
	NIDAQmx * this_ = reinterpret_cast<NIDAQmx*>(callbackData);
	return this_->EveryNCallback(taskHandle, everyNsamplesEventType, nSamples, callbackData);
}

NIDAQ::int32 CVICALLBACK DoneCallbackWrapper(NIDAQ::TaskHandle taskHandle, NIDAQ::int32 status, void *callbackData) {
	NIDAQmx * this_ = reinterpret_cast<NIDAQmx*>(callbackData);
	return this_->DoneCallback(taskHandle, status, callbackData);
}


NIDAQComponent::NIDAQComponent() : serial_number(0) {}
NIDAQComponent::~NIDAQComponent() {}

void NIDAQAPI::getInfo()
{
	//TODO
}

NIDAQmxDeviceManager::NIDAQmxDeviceManager()
{

}

NIDAQmxDeviceManager::~NIDAQmxDeviceManager()
{

}

void NIDAQmxDeviceManager::scanForDevices()
{

	/* Device names */
	char data[2048] = { 0 };
	NIDAQ::DAQmxGetSysDevNames(data, sizeof(data));
	//This will return a comma-seperated list of dev names into data
	//For now we are only supporting the first device found
	//TODO: Add support for string splitting multiple devices
	devices.add(&data[0]);
	printf("Device Name: %s\n", devices[0]);

}

NIDAQmx::NIDAQmx(const char* theDevice) : Thread(String(0)), deviceName(theDevice)
{

	connect();
	getAIChannels();
	getAIVoltageRanges();
	getDIChannels();

	const float rates[] = { 
		1.0, 1.25, 1.5,
		2.0, 2.5, 3.0, 3.33,
		4.0, 5, 6.25, 8.0,
		10.0, 12.5, 15.0,
		20.0, 25.0, 30.0 };

	for (auto rate : rates)
	{
		sampleRates.add(1000.0f * rate);
	}

	//Default to 30000 kS/S at voltage range -10 - 10 V
	sampleRateIndex = sampleRates.size()-1; //30000
	samplerate = sampleRates[sampleRateIndex];

	voltageRangeIndex = aiVRanges.size() - 1; //-10/10
	voltageRange = aiVRanges[voltageRangeIndex];

}

NIDAQmx::~NIDAQmx() {
}

void NIDAQmx::getInfo()
{
	
}

void NIDAQmx::connect()
{

	/* Get product name */
	char data[2048] = { 0 };
	NIDAQ::DAQmxGetDevProductType(STR2CHR(deviceName), &data[0], sizeof(data));
	productName = String(&data[0]);
	printf("Product Name: %s\n", productName);

	/* Get category type */
	NIDAQ::DAQmxGetDevProductCategory(STR2CHR(deviceName), &deviceCategory);
	printf("Device Category: %i\n", deviceCategory);

	/* Get simultaneous sampling supported */
	NIDAQ::bool32 supported = false;
	NIDAQ::DAQmxGetDevAISimultaneousSamplingSupported(STR2CHR(deviceName), &supported);
	simAISamplingSupported = supported == 1;
	printf("Simultaneous sampling %ssupported\n", simAISamplingSupported ? "" : "NOT ");

}

void NIDAQmx::getAIChannels()
{

	char data[2048];
	NIDAQ::DAQmxGetDevAIPhysicalChans(STR2CHR(deviceName), &data[0], sizeof(data));

	StringArray channel_list;
	channel_list.addTokens(&data[0], ", ", "\"");

	std::cout << "Found analog inputs:" << std::endl;
	for (int i = 0; i < channel_list.size(); i++)
	{
		if (channel_list[i].length() > 0)
		{
			printf("%s\n", channel_list[i].toUTF8());
			ai.add(AnalogIn(channel_list[i].toUTF8()));
		}
	}

}

void NIDAQmx::getAIVoltageRanges()
{

	NIDAQ::float64 data[512];
	NIDAQ::DAQmxGetDevAIVoltageRngs(STR2CHR(deviceName), &data[0], sizeof(data));

	for (int i = 0; i < 512; i += 2)
	{
		NIDAQ::float64 vmin = data[i];
		NIDAQ::float64 vmax = data[i + 1];
		if (vmin == vmax)
			break;
		aiVRanges.add(VRange(vmin, vmax));
	}

}

void NIDAQmx::getDIChannels()
{

	char data[2048];
	NIDAQ::DAQmxGetDevTerminals(STR2CHR(deviceName), &data[0], sizeof(data));
	std::cout << "Found PFI Channels: " << std::endl;

	StringArray channel_list;
	channel_list.addTokens(&data[0], ", ", "\"");

	for (int i = 0; i < channel_list.size(); i++)
	{
		StringArray channel_type;
		channel_type.addTokens(channel_list[i], "/", "\"");
		//printf("%s\n", channel_list[i]);
		if (channel_list[i].contains("PFI"))
		{
			printf("%s\n", channel_list[i].substring(1).toUTF8());
			di.add(DigitalIn(channel_list[i].substring(1).toUTF8()));
		}
	}

}

void NIDAQmx::run()
{
	/* Read from USB
	https://kb.mccdaq.com/KnowledgebaseArticle50717.aspx
	*/

	/* Member function callbacks
	*/

	NIDAQ::int32 error = 0;
	NIDAQ::TaskHandle taskHandle = 0;
	char errBuff[ERR_BUFF_SIZE] = { '\0' };

	DAQmxErrChk (NIDAQ::DAQmxCreateTask("", &taskHandle));
	DAQmxErrChk (NIDAQ::DAQmxCreateAIVoltageChan(taskHandle, STR2CHR(ai[0].id), "", DAQmx_Val_Cfg_Default, -10, 10, DAQmx_Val_Volts, NULL));
	DAQmxErrChk (NIDAQ::DAQmxCfgSampClkTiming(taskHandle, "", samplerate, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 1000));

	NIDAQ::uInt32 nSamples = 1000;
	NIDAQ::uInt32 options = 0;

	NIDAQ::DAQmxRegisterEveryNSamplesEvent(taskHandle, DAQmx_Val_Acquired_Into_Buffer, nSamples, options, EveryNCallbackWrapper, &data);
	NIDAQ::DAQmxRegisterDoneEvent(taskHandle, 0, DoneCallbackWrapper, NULL);

	DAQmxErrChk ( NIDAQ::DAQmxStartTask(taskHandle) );

	printf("Acquiring samples continuously. Press Enter to interrupt\n");
	getchar();

Error:

	if (DAQmxFailed(error))
		NIDAQ::DAQmxGetExtendedErrorInfo(errBuff, ERR_BUFF_SIZE);
	if (taskHandle != 0) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		NIDAQ::DAQmxStopTask(taskHandle);
		NIDAQ::DAQmxClearTask(taskHandle);
	}
	if (DAQmxFailed(error))
		printf("DAQmx Error: %s\n", errBuff);
	printf("End of program, press Enter key to quit\n");
	getchar();

}

NIDAQ::int32 CVICALLBACK NIDAQmx::EveryNCallback(NIDAQ::TaskHandle taskHandle, NIDAQ::int32 everyNsamplesEventType, NIDAQ::uInt32 nSamples, void *callbackData)
{
	NIDAQ::int32	error = 0;
	char			errBuff[ERR_BUFF_SIZE] = { '\0' };
	static int		totalRead = 0;
	NIDAQ::int32    read = 0;
	NIDAQ::float64  data[1000];

	/*********************************************/
	// DAQmx Read Code
	/*********************************************/
	NIDAQ::int32 numSampsPerChan = 1000;
	NIDAQ::int32 arraySizeInSamps = 1000;
	DAQmxErrChk(NIDAQ::DAQmxReadAnalogF64(taskHandle, numSampsPerChan, 10.0, DAQmx_Val_GroupByScanNumber, data, arraySizeInSamps, &read, NULL));
	if (read>0) {
		printf("Acquired %d samples. Total %d\r", (int)read, (int)(totalRead += read));
		std::chrono::milliseconds t = std::chrono::duration_cast< std::chrono::milliseconds >(
			std::chrono::system_clock::now().time_since_epoch());
		long long t_ms = t.count()*std::chrono::milliseconds::period::num / std::chrono::milliseconds::period::den;
		printf("Read @ %i\n", t_ms);
		/*
		for (int i = 0; i < 10; i++)
		{
			printf("%i : %.4f\n", i, data[i]);
		}
		*/
		fflush(stdout);
	}



Error:
	if (DAQmxFailed(error)) {
		NIDAQ::DAQmxGetExtendedErrorInfo(errBuff, ERR_BUFF_SIZE);
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		NIDAQ::DAQmxStopTask(taskHandle);
		NIDAQ::DAQmxClearTask(taskHandle);
		printf("DAQmx Error: %s\n", errBuff);
	}
	return 0;
}

NIDAQ::int32 CVICALLBACK NIDAQmx::DoneCallback(NIDAQ::TaskHandle taskHandle, NIDAQ::int32 status, void *callbackData)
{
	int32   error = 0;
	char    errBuff[ERR_BUFF_SIZE] = { '\0' };

	// Check to see if an error stopped the task.
	DAQmxErrChk(status);

Error:
	if (DAQmxFailed(error)) {
		NIDAQ::DAQmxGetExtendedErrorInfo(errBuff, ERR_BUFF_SIZE);
		NIDAQ::DAQmxClearTask(taskHandle);
		printf("DAQmx Error: %s\n", errBuff);
	}
	return 0;
}

InputChannel::InputChannel()
{

}

InputChannel::InputChannel(String id) : id(id), enabled(true)
{
}

InputChannel::~InputChannel()
{
}

void InputChannel::setEnabled(bool enable)
{
	enabled = enable;
}

AnalogIn::AnalogIn()
{
}

AnalogIn::AnalogIn(String id) : InputChannel(id)
{
	voltageRange = VRange(-1.25, 1.25);
}


AnalogIn::~AnalogIn()
{

}

DigitalIn::DigitalIn(String id) : InputChannel(id)
{

}

DigitalIn::~DigitalIn()
{

}



