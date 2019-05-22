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

class NIDAQAPI : public NIDAQComponent
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

	friend class NIDAQThread;

private:
	Array<String> devices;
	
};

struct VRange {
	NIDAQ::float64 vmin, vmax;
	VRange() : vmin(0), vmax(0) {}
	VRange(NIDAQ::float64 rmin, NIDAQ::float64 rmax)
		: vmin(rmin), vmax(rmax) {}
};

class NIDAQmx : public NIDAQComponent, public Thread
{
public:
	NIDAQmx(const char* deviceName);
	~NIDAQmx();

	DataBuffer* aiBuffer;

	int64 ai_timestamp;
	uint64 eventCode;

	void getInfo();

	void connect();

	void getAIChannels();
	void getAIVoltageRanges();
	void getDIChannels();

	void run();

	NIDAQ::float64  data[8000];

	friend class NIDAQThread;

private:
	String deviceName;
	String productName;
	NIDAQ::int32 deviceCategory;

	bool simAISamplingSupported;

	Array<AnalogIn> 	ai;
	Array<DigitalIn> 	di;

	Array<VRange> aiVRanges;
	Array<VRange> diVRanges;

	VRange voltageRange;

	float samplerate;
	float bitVolts;
	Array<float> sampleRates;

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
	enum SOURCE_TYPE {
		FLOATING = 0,
		GROUND_REF
	};

public:
	AnalogIn();
	AnalogIn(String id);
	~AnalogIn();

	void setVoltageRange(VRange range);
	void setSourceType();
	SOURCE_TYPE getSourceType();

	friend class NIDAQmx;

private:
	SOURCE_TYPE sourceType;
	VRange voltageRange;
};

class DigitalIn : public InputChannel
{
public:
	DigitalIn(String id);
	~DigitalIn();
private:
};

#endif  // __NIDAQCOMPONENTS_H__