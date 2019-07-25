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
#define CHANNEL_BUFFER_SIZE 1000
#define MAX_ANALOG_CHANNELS 8
#define NUM_SAMPLE_RATES 17
#define MAX_DIGITAL_CHANNELS 8
#define ERR_BUFF_SIZE 2048
#define STR2CHR( jString ) ((jString).toUTF8())
#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

class NIDAQmx;
class InputChannel;
class AnalogIn;
class DigitalIn;
class Trigger;
class Counter;
class UserDefined;

class NIDAQComponent
{
public:
	NIDAQComponent();
	~NIDAQComponent();
	int serial_number;
	virtual void getInfo() = 0;
};

/* API */

class NIDAQAPI
{
public:
	void getInfo();
};

class NIDAQmxDeviceManager
{
public:
	NIDAQmxDeviceManager();
	~NIDAQmxDeviceManager();

	void scanForDevices();

	String getDeviceFromIndex(int deviceIndex);
	String getDeviceFromProductName(String productName);

	int getNumAvailableDevices();

	friend class NIDAQThread;

private:
	int selectedDeviceIndex;
	StringArray devices;
	
};

struct VRange {
	NIDAQ::float64 vmin, vmax;
	VRange() : vmin(0), vmax(0) {}
	VRange(NIDAQ::float64 rmin, NIDAQ::float64 rmax)
		: vmin(rmin), vmax(rmax) {}
};

struct SRange {
	NIDAQ::float64 smin, smaxs, smaxm;
	SRange() : smin(0), smaxs(0), smaxm(0) {}
	SRange(NIDAQ::float64 smin, NIDAQ::float64 smaxs, NIDAQ::float64 smaxm)
		: smin(smin), smaxs(smaxs), smaxm(smaxm) {}
};

enum SOURCE_TYPE {
	RSE = 0,
	NRSE,
	DIFF,
	PSEUDO_DIFF
};

class NIDAQmx : public Thread
{
public:

	NIDAQmx();
	NIDAQmx(const char* deviceName);
	~NIDAQmx();

	void connect(); 

	String getProductName();
	String getSerialNumber();

	void getAIChannels();
	void getAIVoltageRanges();
	void getDIChannels();

	SOURCE_TYPE getSourceTypeForInput(int index);
	void toggleSourceType(int id);

	int getActiveDigitalLines();

	void run();

	friend class NIDAQThread;

private:

	String				deviceName;
	String				productName;
	NIDAQ::int32		deviceCategory;
	NIDAQ::uInt32		productNum;
	NIDAQ::uInt32		serialNum;
	bool				isUSBDevice;
	bool				simAISamplingSupported;
	NIDAQ::float64		adcResolution;
	SRange 				sampleRateRange;

	Array<VRange>		aiVRanges;
	VRange				voltageRange;

	Array<float>		sampleRates;
	float				samplerate;

	Array<AnalogIn> 	ai;
	Array<NIDAQ::int32> terminalConfig;
	Array<SOURCE_TYPE>  st;
	Array<bool>			aiChannelEnabled;

	Array<DigitalIn> 	di;
	Array<bool>			diChannelEnabled;

	NIDAQ::float64		ai_data[CHANNEL_BUFFER_SIZE * MAX_ANALOG_CHANNELS];
	NIDAQ::uInt8		di_data_8[CHANNEL_BUFFER_SIZE];  //PXI devices use 8-bit read
	NIDAQ::uInt32		di_data_32[CHANNEL_BUFFER_SIZE]; //USB devices use 32-bit read

	int64 ai_timestamp;
	uint64 eventCode;

	DataBuffer* aiBuffer;

};

/* Inputs */ 

class InputChannel
{
public:
	InputChannel();
	InputChannel(String id);
	~InputChannel();

	void setSampleRate(int rateIndex);
	int getSampleRate();

	void setEnabled(bool);

	void startAcquisition();
	void stopAcquisition();

	void setSavingDirectory(File);
	File getSavingDirectory();

	float getFillPercentage();

	friend class NIDAQmx;
	
private:
	String id;
	bool enabled;
	File savingDirectory;

};

class AnalogIn : public InputChannel
{

public:
	AnalogIn();
	AnalogIn(String id);
	~AnalogIn();

	Array<SOURCE_TYPE> getTerminalConfig();

private:
	NIDAQ::int32 terminalConfig;
};

class DigitalIn : public InputChannel
{
public:
	DigitalIn(String id);
	DigitalIn();
	~DigitalIn();
private:
};

#endif  // __NIDAQCOMPONENTS_H__