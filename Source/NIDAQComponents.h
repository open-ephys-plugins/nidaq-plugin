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

#ifndef __NIDAQCOMPONENTS_H__
#define __NIDAQCOMPONENTS_H__

#include <DataThreadHeaders.h>
#include <stdio.h>
#include <string.h>

#include "nidaq-api/NIDAQmx.h"

#define NUM_SOURCE_TYPES 4
#define CHANNEL_BUFFER_SIZE 500
#define MAX_ANALOG_CHANNELS 8
#define NUM_SAMPLE_RATES 17
#define MAX_DIGITAL_CHANNELS 8
#define ERR_BUFF_SIZE 2048
#define STR2CHR( jString ) ((jString).toUTF8())
#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

class NIDAQmx;
class InputChannel;
class AnalogInput;
class DigitalInput;
class Trigger;
class Counter;
class UserDefined;

enum SOURCE_TYPE {
	RSE = 0,
	NRSE,
	DIFF,
	PSEUDO_DIFF
};

struct SettingsRange {
	NIDAQ::float64 min, max;
	SettingsRange() : min(0), max(0) {}
	SettingsRange(NIDAQ::float64 min_, NIDAQ::float64 max_)
		: min(min_), max(max_) {}
};

class InputChannel
{
public:
	InputChannel(String name_) : name(name_) {};
	InputChannel() : name("") {};
	~InputChannel() {};

	String getName() { return name; }

	void setEnabled(bool) { enabled = true; }
	bool isEnabled() { return enabled; }

private:
	String name;
	bool enabled = false;
};

class AnalogInput : public InputChannel
{

public:
	AnalogInput(String name, NIDAQ::int32 terminalConfig);
	AnalogInput() : InputChannel() {};
	~AnalogInput() {};

	SOURCE_TYPE getSourceType() { return sourceTypes[sourceTypeIndex]; }
	void setSourceType(int index) { sourceTypeIndex = index; }

private:
	int sourceTypeIndex = 0;
	Array<SOURCE_TYPE> sourceTypes;

};

class NIDAQDevice
{

public:

	NIDAQDevice(String name_) : name(name_) {};
	NIDAQDevice() {};
	~NIDAQDevice() {};

	String getName() { return name; }

	String productName;

	bool isUSBDevice;
	bool simAISamplingSupported;

	NIDAQ::int32 deviceCategory;
	NIDAQ::uInt32 productNum;
	NIDAQ::uInt32 serialNum;
	NIDAQ::uInt32 numAIChannels;
	NIDAQ::uInt32 numAOChannels;
	NIDAQ::uInt32 numDIChannels;
	NIDAQ::uInt32 numDOChannels;

	SettingsRange sampleRateRange;

	Array<SettingsRange> voltageRanges;
	Array<NIDAQ::float64> adcResolutions;

private:

	String name;

};

class NIDAQmxDeviceManager
{
public:

	NIDAQmxDeviceManager() {};
	~NIDAQmxDeviceManager() {};

	void scanForDevices();

	int getNumAvailableDevices() { return devices.size(); }

	int getDeviceIndexFromName(String deviceName);
	NIDAQDevice* getDeviceAtIndex(int index) { return devices[index]; }

	NIDAQDevice* getDeviceFromName(String deviceName);

	friend class NIDAQThread;

private:

	OwnedArray<NIDAQDevice> devices;
	int activeDeviceIndex;
};

class NIDAQmx : public Thread
{
public:

	NIDAQmx(NIDAQDevice* device_);
	~NIDAQmx() {};

	/* Pointer to the active device */
	NIDAQDevice* device;

	/* Connects to the active device */
	void connect(); 

	String getProductName() { return device->productName; };
	String getSerialNumber() { return String(device->serialNum); };

	void getNumAnalogInputs();
	int getActiveDigitalLines();

	NIDAQ::float64 getSampleRate() { return sampleRates[sampleRateIndex]; };
	SettingsRange getVoltageRange() { return device->voltageRanges[voltageRangeIndex]; };

	SOURCE_TYPE getSourceTypeForInput(int analogIntputIndex);

	void setSampleRate(int index) { sampleRateIndex = index; };
	void setVoltageRange(int index) { voltageRangeIndex = index; };
	void toggleSourceType(int analogInputIndex);

	void run();

	Array<NIDAQ::float64> sampleRates;

	OwnedArray<AnalogInput> 	ai;
	OwnedArray<InputChannel> 	di;

	friend class NIDAQThread;

private:

	/* Manages connected NIDAQ devices */
	ScopedPointer<NIDAQmxDeviceManager> dm;

	int deviceIndex = 0;
	int sampleRateIndex = 0;
	int voltageRangeIndex = 0;

	NIDAQ::float64		ai_data[CHANNEL_BUFFER_SIZE * MAX_ANALOG_CHANNELS];
	NIDAQ::uInt8		di_data_8[CHANNEL_BUFFER_SIZE];  //PXI devices use 8-bit read
	NIDAQ::uInt32		di_data_32[CHANNEL_BUFFER_SIZE]; //USB devices use 32-bit read

	int64 ai_timestamp;
	uint64 eventCode;

	DataBuffer* aiBuffer;

};

#endif  // __NIDAQCOMPONENTS_H__