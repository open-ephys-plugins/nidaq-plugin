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

	int serial_number;

	virtual void getInfo() = 0;
};

/* API */

class NIDAQAPI : public NIDAQComponent
{
public:
	void getInfo();
};

/* Front panel */

class NIDAQmx : public NIDAQComponent
{
public:
	NIDAQmx();
	~NIDAQmx();

	void getInfo();

	void init();
	void getAIChannels();
	void getDIChannels();

private:
	Array<AnalogIn> 	ai;
	Array<DigitalIn> 	di;

};

/* Inputs */ 

class InputChannel : public NIDAQComponent
{
public:
	InputChannel(int id);
	~InputChannel();

	void getInfo();

	int samplerate;

	void startAcquisition();
	void stopAcquisition();

	void setSavingDirectory(File);
	File getSavingDirectory();

	float getFillPercentage();
	
private:
	int id; 					
	File savingDirectory;

};

class AnalogIn : public InputChannel
{

	enum SOURCE_TYPE {
		FLOATING = 0,
		GROUND_REF
	};

public:
	AnalogIn(int id);
	~AnalogIn();
	SOURCE_TYPE getSourceType();
private:
	
};

class DigitalIn : public InputChannel
{
public:
	DigitalIn(int id);
	~DigitalIn();
private:
};

#endif  // __NIDAQCOMPONENTS_H__