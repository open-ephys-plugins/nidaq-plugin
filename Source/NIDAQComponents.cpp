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

NIDAQComponent::NIDAQComponent() : serial_number(0) {}
NIDAQComponent::~NIDAQComponent() {}

void NIDAQAPI::getInfo()
{
	//TODO
}

NIDAQmx::NIDAQmx()
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
		sampleRates.add(1000 * rate);
	}

	run();
}

NIDAQmx::~NIDAQmx() {
}

void NIDAQmx::getInfo()
{
	
}

void NIDAQmx::connect()
{
	char data[2048] = { 0 };
	NIDAQ::DAQmxGetSysDevNames(data, sizeof(data));
	//This will return a comma-seperated list of dev names into data
	//For now we are only supporting the first device found
	//TODO: Add support for multiple devices
	deviceName = new String(&data[0]);
	std::cout << "Device Name: " << String(deviceName->getCharPointer()) << std::endl;

	NIDAQ::DAQmxGetDevProductType(deviceName->getCharPointer(), &data[0], sizeof(data));
	productName = new String(&data[0]);
	std::cout << "Product Name: " << String(productName->getCharPointer()) << std::endl;

	NIDAQ::int32 buf;
	NIDAQ::DAQmxGetDevProductCategory(deviceName->getCharPointer(), &buf);
	deviceCategory = new String(buf);
	std::cout << "Device Category: " << String(deviceCategory->getCharPointer()) << std::endl;

}

void NIDAQmx::getAIChannels()
{

	char data[2048];
	NIDAQ::DAQmxGetDevAIPhysicalChans(deviceName->getCharPointer(), &data[0], sizeof(data));

	StringArray channel_list;
	channel_list.addTokens(&data[0], ", ", "\"");

	std::cout << "Found analog inputs:" << std::endl;
	for (int i = 0; i < channel_list.size(); i++)
	{
		if (channel_list[i].length() > 0)
		{
			std::cout << channel_list[i] << std::endl;
			ai.add(AnalogIn(channel_list[i]));
		}
	}

}

void NIDAQmx::getAIVoltageRanges()
{
	NIDAQ::float64 data[512];
	NIDAQ::DAQmxGetDevAIVoltageRngs(deviceName->getCharPointer(), &data[0], sizeof(data));

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
	NIDAQ::DAQmxGetDevTerminals(deviceName->getCharPointer(), &data[0], sizeof(data));
	std::cout << "Found PFI Channels: " << std::endl;

	StringArray channel_list;
	channel_list.addTokens(&data[0], ", ", "\"");

	for (int i = 0; i < channel_list.size(); i++)
	{
		StringArray channel_type;
		channel_type.addTokens(channel_list[i], "/", "\"");
		if (channel_list[i].contains("PFI"))
		{
			std::cout << channel_list[i].substring(1) << std::endl;
			di.add(DigitalIn(channel_list[i].substring(1)));
		}
	}
}

void NIDAQmx::run()
{

	NIDAQ::TaskHandle taskHandle = 0;
	NIDAQ::DAQmxCreateTask("", &taskHandle);

	for (int i = 0; i < ai.size(); i++) {
		NIDAQ::DAQmxCreateAIVoltageChan(
			taskHandle,
			ai[i].id.getCharPointer(),		// physical channel name
			ai[i].id.getCharPointer(),		// channel name
			DAQmx_Val_Cfg_Default,			// terminal configuration
			ai[i].voltageRange.vmin,		// channel max and min
			ai[i].voltageRange.vmax,
			DAQmx_Val_Volts,
			NULL);
	}
	
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

DigitalIn::~DigitalIn()
{

}



