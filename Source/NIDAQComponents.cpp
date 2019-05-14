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

#include "NIDAQComponents.h"

void NIDAQAPI::getInfo()
{
	//TODO
}

NIDAQComponent::NIDAQComponent() : serial_number(-1) {}

NIDAQmx::NIDAQmx() {}

NIDAQmx::~NIDAQmx() {}

void NIDAQmx::getInfo()
{
	
}

void NIDAQmx::init()
{

	std::cout << "In init() method!!!" << std::endl;

	char data[2048] = { 0 };

	NIDAQ::DAQmxGetSysDevNames(data, sizeof(data));

	std::cout << "Found NI-DAQ device: " << data << std::endl;


}

void NIDAQmx::getAIChannels()
{

	NIDAQ::uInt32 dataSize = 65536;
	std::vector<char> data(dataSize);
	//NIDAQ::DAQmxGetDevAIPhysicalChans(dataSize);
	for (int i = 0; i < 8; i++)
	{
		ai.add(AnalogIn(i));
	}
}

void NIDAQmx::getDIChannels()
{
	for (int i = 0; i < 8; i++)
	{
		di.add(DigitalIn(i));
	}
}

InputChannel::InputChannel(int id) : id(id), samplerate(1e+6)
{

}

InputChannel::~InputChannel()
{

}

void InputChannel::getInfo()
{
	//TODO
}

AnalogIn::AnalogIn(int id) : InputChannel(id)
{
}

AnalogIn::~AnalogIn()
{

}

DigitalIn::DigitalIn(int id) : InputChannel(id)
{

}

DigitalIn::~DigitalIn()
{

}


