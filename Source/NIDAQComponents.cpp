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
#include <math.h>

#include "NIDAQComponents.h"

static int32 GetTerminalNameWithDevPrefix(NIDAQ::TaskHandle taskHandle, const char terminalName[], char triggerName[]);

static int32 GetTerminalNameWithDevPrefix(NIDAQ::TaskHandle taskHandle, const char terminalName[], char triggerName[])
{

	NIDAQ::int32	error = 0;
	char			device[256];
	NIDAQ::int32	productCategory;
	NIDAQ::uInt32	numDevices, i = 1;

	DAQmxErrChk(NIDAQ::DAQmxGetTaskNumDevices(taskHandle, &numDevices));
	while (i <= numDevices) {
		DAQmxErrChk(NIDAQ::DAQmxGetNthTaskDevice(taskHandle, i++, device, 256));
		DAQmxErrChk(NIDAQ::DAQmxGetDevProductCategory(device, &productCategory));
		if (productCategory != DAQmx_Val_CSeriesModule && productCategory != DAQmx_Val_SCXIModule) {
			*triggerName++ = '/';
			strcat(strcat(strcpy(triggerName, device), "/"), terminalName);
			break;
		}
	}

Error:
	return error;
}

NIDAQComponent::NIDAQComponent() : serial_number(0) {}
NIDAQComponent::~NIDAQComponent() {}

void NIDAQAPI::getInfo()
{
	//TODO
}

NIDAQmxDeviceManager::NIDAQmxDeviceManager() {}

NIDAQmxDeviceManager::~NIDAQmxDeviceManager() {}

void NIDAQmxDeviceManager::scanForDevices()
{

	char data[2048] = { 0 };
	NIDAQ::DAQmxGetSysDevNames(data, sizeof(data));

	StringArray deviceList; 
	deviceList.addTokens(&data[0], ", ", "\"");

	StringArray deviceNames;
	StringArray productList;

	for (int i = 0; i < deviceList.size(); i++)
		if (deviceList[i].length() > 0)
			devices.add(deviceList[i].toUTF8());

}

String NIDAQmxDeviceManager::getDeviceFromIndex(int index)
{
	return devices[index];
}

String NIDAQmxDeviceManager::getDeviceFromProductName(String productName)
{
	for (auto device : devices)
	{
		ScopedPointer<NIDAQmx> n = new NIDAQmx(STR2CHR(device));
		if (n->getProductName() == productName)
			return device;
	}
	return "";

}

int NIDAQmxDeviceManager::getNumAvailableDevices()
{
	return devices.size();
}

NIDAQmx::NIDAQmx() : Thread("NIDAQmx_Thread") {};

NIDAQmx::NIDAQmx(const char* deviceName) 
	: Thread("NIDAQmx_Thread"),
	deviceName(deviceName)
{

	adcResolution = 0; //bits

	connect();

	float sample_rates[NUM_SAMPLE_RATES] = {
		1000.0f, 1250.0f, 1500.0f,
		2000.0f, 2500.0f,
		3000.0f, 3330.0f,
		4000.0f,
		5000.0f,
		6250.0f,
		8000.0f,
		10000.0f,
		12500.0f,
		15000.0f,
		20000.0f,
		25000.0f,
		30000.0f
	};

	int idx = 0;
	while (sample_rates[idx] <= sampleRateRange.smaxm && idx < NUM_SAMPLE_RATES)
		sampleRates.add(sample_rates[idx++]); 

	// Default to highest sample rate
	samplerate = sampleRates[sampleRates.size() - 1];

	// Default to largest voltage range
	voltageRange = aiVRanges[aiVRanges.size() - 1];

	// Enable all channels by default
	for (int i = 0; i < aiChannelEnabled.size(); i++)
		aiChannelEnabled.set(i, true);

	for (int i = 0; i < diChannelEnabled.size(); i++)
		diChannelEnabled.set(i, true);

}

NIDAQmx::~NIDAQmx() {
}

String NIDAQmx::getProductName()
{
	return productName;
}

String NIDAQmx::getSerialNumber()
{
	return String(serialNum);
}

void NIDAQmx::connect()
{
	/* Get category type */
	NIDAQ::DAQmxGetDevProductCategory(STR2CHR(deviceName), &deviceCategory);
	printf("Device Category: %i\n", deviceCategory);

	/* Get product name */
	char pname[2048] = { 0 };
	NIDAQ::DAQmxGetDevProductType(STR2CHR(deviceName), &pname[0], sizeof(pname));
	productName = String(&pname[0]);
	printf("Product Name: %s\n", productName);

	isUSBDevice = false;
	if (productName.contains("USB"))
		isUSBDevice = true;

	NIDAQ::DAQmxGetDevProductNum(STR2CHR(deviceName), &productNum);
	printf("Product Num: %d\n", productNum);

	NIDAQ::DAQmxGetDevSerialNum(STR2CHR(deviceName), &serialNum);
	printf("Serial Num: %d\n", serialNum);

	/* Get simultaneous sampling supported */
	NIDAQ::bool32 supported = false;
	NIDAQ::DAQmxGetDevAISimultaneousSamplingSupported(STR2CHR(deviceName), &supported);
	simAISamplingSupported = (supported == 1);
	//printf("Simultaneous sampling %ssupported\n", simAISamplingSupported ? "" : "NOT ");

	/* Get device sample rates */
	NIDAQ::float64 smin;
	NIDAQ::DAQmxGetDevAIMinRate(STR2CHR(deviceName), &smin);

	NIDAQ::float64 smaxs;
	NIDAQ::DAQmxGetDevAIMaxSingleChanRate(STR2CHR(deviceName), &smaxs);

	NIDAQ::float64 smaxm;
	NIDAQ::DAQmxGetDevAIMaxMultiChanRate(STR2CHR(deviceName), &smaxm);

	fflush(stdout);

	getAIVoltageRanges();
	getAIChannels();
	getDIChannels();

	if (!simAISamplingSupported)
		smaxm = smaxs / ai.size();

	sampleRateRange = SRange(smin, smaxs, smaxm);

	printf("Min sample rate: %1.2f\n", sampleRateRange.smin);
	printf("Max single channel sample rate: %1.2f\n", sampleRateRange.smaxs);
	printf("Max multi channel sample rate: %1.2f\n", sampleRateRange.smaxm);

}

void NIDAQmx::getAIVoltageRanges()
{

	NIDAQ::float64 data[512];
	NIDAQ::DAQmxGetDevAIVoltageRngs(STR2CHR(deviceName), &data[0], sizeof(data));

	//printf("Detected voltage ranges:\n");
	for (int i = 0; i < 512; i += 2)
	{
		NIDAQ::float64 vmin = data[i];
		NIDAQ::float64 vmax = data[i + 1];
		if (vmin == vmax || vmax < 1e-2)
			break;
		//printf("Vmin: %f Vmax: %f \n", vmin, vmax);
		aiVRanges.add(VRange(vmin, vmax));
	}

	fflush(stdout);

}

void NIDAQmx::getAIChannels()
{

	NIDAQ::int32	error = 0;
	char			errBuff[ERR_BUFF_SIZE] = { '\0' };

	NIDAQ::TaskHandle adcResolutionQuery;
	NIDAQ::DAQmxCreateTask("ADCResolutionQuery", &adcResolutionQuery);

	char data[2048];
	NIDAQ::DAQmxGetDevAIPhysicalChans(STR2CHR(deviceName), &data[0], sizeof(data));

	StringArray channel_list;
	channel_list.addTokens(&data[0], ", ", "\"");

	int aiCount = 0;

	VRange vRange = aiVRanges[aiVRanges.size() - 1];

	for (int i = 0; i < channel_list.size(); i++)
	{
		if (channel_list[i].length() > 0 && aiCount++ < MAX_ANALOG_CHANNELS)
		{

			/* Get channel termination */
			NIDAQ::int32 termCfgs;
			NIDAQ::DAQmxGetPhysicalChanAITermCfgs(channel_list[i].toUTF8(), &termCfgs);

			printf("%s - ", channel_list[i].toUTF8());
			printf("Terminal Config: %d\n", termCfgs);

			ai.add(AnalogIn(channel_list[i].toUTF8()));

			terminalConfig.add(termCfgs);

			if (termCfgs & DAQmx_Val_Bit_TermCfg_RSE)
			{
				st.add(SOURCE_TYPE::RSE);
			}
			else if (termCfgs & DAQmx_Val_Bit_TermCfg_NRSE)
			{
				st.add(SOURCE_TYPE::NRSE);
			}
			else if (termCfgs & DAQmx_Val_Bit_TermCfg_Diff)
			{
				st.add(SOURCE_TYPE::DIFF);
			}
			else 
			{
				st.add(SOURCE_TYPE::PSEUDO_DIFF);
			}

			/* Get channel ADC resolution */
			DAQmxErrChk(NIDAQ::DAQmxCreateAIVoltageChan(
				adcResolutionQuery,			//task handle
				STR2CHR(ai[aiCount-1].id),	//NIDAQ physical channel name (e.g. dev1/ai1)
				"",							//user-defined channel name (optional)
				DAQmx_Val_Cfg_Default,		//input terminal configuration
				vRange.vmin,				//min input voltage
				vRange.vmax,				//max input voltage
				DAQmx_Val_Volts,			//voltage units
				NULL));

			DAQmxErrChk(NIDAQ::DAQmxGetAIResolution(adcResolutionQuery, channel_list[i].toUTF8(), &adcResolution));
			aiChannelEnabled.add(true);

		}
	}

	fflush(stdout);
	NIDAQ::DAQmxStopTask(adcResolutionQuery);
	NIDAQ::DAQmxClearTask(adcResolutionQuery);

Error:

	if (DAQmxFailed(error))
		NIDAQ::DAQmxGetExtendedErrorInfo(errBuff, ERR_BUFF_SIZE);

	if (adcResolutionQuery != 0) {
		// DAQmx Stop Code
		NIDAQ::DAQmxStopTask(adcResolutionQuery);
		NIDAQ::DAQmxClearTask(adcResolutionQuery);
	}

	if (DAQmxFailed(error))
		printf("DAQmx Error: %s\n", errBuff);
	fflush(stdout);

	return;

}

void NIDAQmx::getDIChannels()
{

	char data[2048];
	//NIDAQ::DAQmxGetDevTerminals(STR2CHR(deviceName), &data[0], sizeof(data)); //gets all terminals
	//NIDAQ::DAQmxGetDevDIPorts(STR2CHR(deviceName), &data[0], sizeof(data));	//gets line name
	NIDAQ::DAQmxGetDevDILines(STR2CHR(deviceName), &data[0], sizeof(data));	//gets ports on line
	printf("Found digital inputs: \n");

	StringArray channel_list;
	channel_list.addTokens(&data[0], ", ", "\"");

	int diCount = 0;

	for (int i = 0; i < channel_list.size(); i++)
	{
		StringArray channel_type;
		channel_type.addTokens(channel_list[i], "/", "\"");
		if (channel_list[i].length() > 0 && diCount++ < MAX_DIGITAL_CHANNELS)
		{
			printf("%s\n", channel_list[i].toUTF8());
			di.add(DigitalIn(channel_list[i].toUTF8()));
			diChannelEnabled.add(false);
		}
	}

	fflush(stdout);

}

int NIDAQmx::getActiveDigitalLines()
{
	uint16 linesEnabled = 0;
	for (int i = 0; i < diChannelEnabled.size(); i++)
	{
		if (diChannelEnabled[i])
			linesEnabled += pow(2, i);
	}
	return linesEnabled;
}

void NIDAQmx::toggleSourceType(int index)
{

	SOURCE_TYPE current = st[index];
	int next = (static_cast<int>(current)+1) % NUM_SOURCE_TYPES;
	SOURCE_TYPE source = static_cast<SOURCE_TYPE>(next);
	while (!((1 << next) & terminalConfig[index]))
		source = static_cast<SOURCE_TYPE>(++next % NUM_SOURCE_TYPES);
	st.set(index,source);

}

void NIDAQmx::run()
{
	/* Derived from NIDAQmx: ANSI C Example program: ContAI-ReadDigChan.c */

	NIDAQ::int32	error = 0;
	char			errBuff[ERR_BUFF_SIZE] = { '\0' };

	/**************************************/
	/********CONFIG ANALOG CHANNELS********/
	/**************************************/

	NIDAQ::int32		ai_read = 0;
	static int			totalAIRead = 0;
	NIDAQ::TaskHandle	taskHandleAI = 0;

	String usePort; //Temporary digital port restriction until software buffering is implemented

	/* Create an analog input task */
	if (isUSBDevice)
		DAQmxErrChk(NIDAQ::DAQmxCreateTask(STR2CHR("AITask_USB" + getSerialNumber()), &taskHandleAI));
	else
		DAQmxErrChk(NIDAQ::DAQmxCreateTask(STR2CHR("AITask_PXI" + getSerialNumber()), &taskHandleAI));


	/* Create a voltage channel for each analog input */
	for (int i = 0; i < ai.size(); i++)
	{
		NIDAQ::int32 termConfig;

		switch (st[i]) {
		case SOURCE_TYPE::RSE:
			termConfig = DAQmx_Val_RSE;
		case SOURCE_TYPE::NRSE:
			termConfig = DAQmx_Val_NRSE;
		case SOURCE_TYPE::DIFF:
			termConfig = DAQmx_Val_Diff;
		case SOURCE_TYPE::PSEUDO_DIFF:
			termConfig = DAQmx_Val_PseudoDiff;
		default:
			termConfig = DAQmx_Val_Cfg_Default;
		}

		DAQmxErrChk(NIDAQ::DAQmxCreateAIVoltageChan(
			taskHandleAI,					//task handle
			STR2CHR(ai[i].id),			//NIDAQ physical channel name (e.g. dev1/ai1)
			"",							//user-defined channel name (optional)
			termConfig,					//input terminal configuration
			voltageRange.vmin,			//min input voltage
			voltageRange.vmax,			//max input voltage
			DAQmx_Val_Volts,			//voltage units
			NULL));

	}

	/* Configure sample clock timing */
	DAQmxErrChk(NIDAQ::DAQmxCfgSampClkTiming(
		taskHandleAI,
		"",													//source : NULL means use internal clock
		samplerate,											//rate : samples per second per channel
		DAQmx_Val_Rising,									//activeEdge : (DAQmc_Val_Rising || DAQmx_Val_Falling)
		DAQmx_Val_ContSamps,								//sampleMode : (DAQmx_Val_FiniteSamps || DAQmx_Val_ContSamps || DAQmx_Val_HWTimedSinglePoint)
		MAX_ANALOG_CHANNELS * CHANNEL_BUFFER_SIZE));		//sampsPerChanToAcquire : 
																//If sampleMode == DAQmx_Val_FiniteSamps : # of samples to acquire for each channel
																//Elif sampleMode == DAQmx_Val_ContSamps : circular buffer size


	/* Get handle to analog trigger to sync with digital inputs */
	char trigName[256];
	DAQmxErrChk(GetTerminalNameWithDevPrefix(taskHandleAI, "ai/SampleClock", trigName));

	/************************************/
	/********CONFIG DIGITAL LINES********/
	/************************************/

	NIDAQ::int32		di_read = 0;
	static int			totalDIRead = 0;
	NIDAQ::TaskHandle	taskHandleDI = 0;

	char ports[2048];
	NIDAQ::DAQmxGetDevDIPorts(STR2CHR(deviceName), &ports[0], sizeof(ports));

	/* For now, restrict max num digital inputs until software buffering is implemented */
	if (MAX_DIGITAL_CHANNELS <= 8)
	{
		StringArray port_list;
		port_list.addTokens(&ports[0], ", ", "\"");
		usePort = port_list[0];
	}

	/* Create a digital input task using device serial number to gurantee unique task name per device */
	if (isUSBDevice)
		DAQmxErrChk(NIDAQ::DAQmxCreateTask(STR2CHR("DITask_USB"+getSerialNumber()), &taskHandleDI));
	else
		DAQmxErrChk(NIDAQ::DAQmxCreateTask(STR2CHR("DITask_PXI"+getSerialNumber()), &taskHandleDI));

	/* Create a channel for each digital input */
	DAQmxErrChk(NIDAQ::DAQmxCreateDIChan(
		taskHandleDI,
		STR2CHR(usePort),
		"",
		DAQmx_Val_ChanForAllLines));

	
	if (!isUSBDevice) //USB devices do not have an internal clock and instead use CPU, so we can't configure the sample clock timing
		DAQmxErrChk(NIDAQ::DAQmxCfgSampClkTiming(
			taskHandleDI,							//task handle
			trigName,								//source : NULL means use internal clock, we will sync to analog input clock
			samplerate,								//rate : samples per second per channel
			DAQmx_Val_Rising,						//activeEdge : (DAQmc_Val_Rising || DAQmx_Val_Falling)
			DAQmx_Val_ContSamps,					//sampleMode : (DAQmx_Val_FiniteSamps || DAQmx_Val_ContSamps || DAQmx_Val_HWTimedSinglePoint)
			CHANNEL_BUFFER_SIZE));					//sampsPerChanToAcquire : want to sync with analog samples per channel
														//If sampleMode == DAQmx_Val_FiniteSamps : # of samples to acquire for each channel
														//Elif sampleMode == DAQmx_Val_ContSamps : circular buffer size

	DAQmxErrChk(NIDAQ::DAQmxTaskControl(taskHandleAI, DAQmx_Val_Task_Commit));
	DAQmxErrChk(NIDAQ::DAQmxTaskControl(taskHandleDI, DAQmx_Val_Task_Commit));

	DAQmxErrChk(NIDAQ::DAQmxStartTask(taskHandleDI));
	DAQmxErrChk(NIDAQ::DAQmxStartTask(taskHandleAI));

	NIDAQ::int32 numSampsPerChan;
	if (isUSBDevice)
		numSampsPerChan = 100;
	else
		numSampsPerChan = CHANNEL_BUFFER_SIZE;

	NIDAQ::int32 arraySizeInSamps = ai.size() * numSampsPerChan;
	NIDAQ::float64 timeout = 5.0;

	uint64 linesEnabled = 0;

	ai_timestamp = 0;
	eventCode = 0;

	while (!threadShouldExit())
	{

		DAQmxErrChk(NIDAQ::DAQmxReadAnalogF64(
			taskHandleAI,
			numSampsPerChan,
			timeout,
			DAQmx_Val_GroupByScanNumber, //DAQmx_Val_GroupByScanNumber
			ai_data,
			arraySizeInSamps,
			&ai_read,
			NULL));

		if (getActiveDigitalLines() > 0)
		{
			if (isUSBDevice)
				DAQmxErrChk(NIDAQ::DAQmxReadDigitalU32(
					taskHandleDI,
					numSampsPerChan,
					timeout,
					DAQmx_Val_GroupByScanNumber,
					di_data_32,
					numSampsPerChan,
					&di_read,
					NULL));
			else 
				DAQmxErrChk(NIDAQ::DAQmxReadDigitalU8(
					taskHandleDI,
					numSampsPerChan,
					timeout,
					DAQmx_Val_GroupByScanNumber,
					di_data_8,
					numSampsPerChan,
					&di_read,
					NULL));
		}

		/*
		std::chrono::milliseconds last_time;
		std::chrono::milliseconds t = std::chrono::duration_cast< std::chrono::milliseconds >(
			std::chrono::system_clock::now().time_since_epoch());
		long long t_ms = t.count()*std::chrono::milliseconds::period::num / std::chrono::milliseconds::period::den;
		if (ai_read>0) {
			printf("Read @ %i | ", t_ms);
			printf("Acquired %d AI samples. Total %d | ", (int)ai_read, (int)(totalAIRead += ai_read));
			printf("Acquired %d DI samples. Total %d\n", (int)di_read, (int)(totalDIRead += di_read));
			fflush(stdout);
		}
		*/

		float aiSamples[MAX_ANALOG_CHANNELS];
		int count = 0;
		for (int i = 0; i < arraySizeInSamps; i++)
		{
	
			int channel = i % MAX_ANALOG_CHANNELS;

			aiSamples[channel] = 0;
			if (aiChannelEnabled[channel])
				aiSamples[channel] = ai_data[i];

			if (i % MAX_ANALOG_CHANNELS == 0)
			{
				ai_timestamp++;
				if (getActiveDigitalLines() > 0)
				{
					if (isUSBDevice)
						eventCode = di_data_32[count++] & getActiveDigitalLines();
					else
						eventCode = di_data_8[count++] & getActiveDigitalLines();
				}
				aiBuffer->addToBuffer(aiSamples, &ai_timestamp, &eventCode, 1);
			}

		}

		fflush(stdout);

	}

	/*********************************************/
	// DAQmx Stop Code
	/*********************************************/

	NIDAQ::DAQmxStopTask(taskHandleAI);
	NIDAQ::DAQmxClearTask(taskHandleAI);
	NIDAQ::DAQmxStopTask(taskHandleDI);
	NIDAQ::DAQmxClearTask(taskHandleDI);

	return;

Error:

	if (DAQmxFailed(error))
		NIDAQ::DAQmxGetExtendedErrorInfo(errBuff, ERR_BUFF_SIZE);

	if (taskHandleAI != 0) {
		// DAQmx Stop Code
		NIDAQ::DAQmxStopTask(taskHandleAI);
		NIDAQ::DAQmxClearTask(taskHandleAI);
	}

	if (taskHandleDI != 0) {
		// DAQmx Stop Code
		NIDAQ::DAQmxStopTask(taskHandleDI);
		NIDAQ::DAQmxClearTask(taskHandleDI);
	}
	if (DAQmxFailed(error))
		printf("DAQmx Error: %s\n", errBuff);
		fflush(stdout);

	return;

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
	
}

AnalogIn::~AnalogIn()
{

}

DigitalIn::DigitalIn(String id) : InputChannel(id)
{

}

DigitalIn::DigitalIn()
{

}

DigitalIn::~DigitalIn()
{

}



