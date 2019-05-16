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

#define STR2CHR( jString ) ((jString).toUTF8())

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

NIDAQmx::NIDAQmx(const char* deviceName) : deviceName(deviceName)
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
	samplerate = 30000;

	run();
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
		if (channel_list[i].contains("PFI"))
		{
			printf("%s\n", channel_list[i].substring(1).toUTF8());
			di.add(DigitalIn(channel_list[i].substring(1).toUTF8()));
		}
	}

}

void NIDAQmx::run()
{

	NIDAQ::TaskHandle taskHandle = 0;
	NIDAQ::uInt32 numberOfSamplesPerChannel = 1000;
	NIDAQ::DAQmxCreateTask("", &taskHandle);
	NIDAQ::int32 numSamplesRead;

	NIDAQ::int16 data[8000];

	NIDAQ::int32 error;

	/* Configure analog input channels */
	for (int i = 0; i < ai.size(); i++) {

		printf("Channel: %s\n", STR2CHR(ai[i].id));
		printf("Vmin: %f\n", ai[i].voltageRange.vmin);
		printf("Vmax: %f\n", ai[i].voltageRange.vmax);
		printf("Sample rate: %f\n", samplerate);
		printf("Samples per channel %d\n", numberOfSamplesPerChannel);

		const char* id = STR2CHR(ai[i].id);

		error = NIDAQ::DAQmxCreateAIVoltageChan(
			taskHandle,
			STR2CHR(ai[i].id),			// physical channel name
			STR2CHR(String("Channel")+String(i)),					// channel name
			DAQmx_Val_Cfg_Default,		// terminal configuration
			ai[i].voltageRange.vmin,						// channel max and min
			ai[i].voltageRange.vmax,
			DAQmx_Val_Volts,
			NULL);

		printf("Init vChan: %i | error: %i\n", i, error);
	}

	/* Configure sampling rate */
	error = NIDAQ::DAQmxCfgSampClkTiming(
		taskHandle, "",
		1000,
		DAQmx_Val_Rising,
		DAQmx_Val_FiniteSamps,
		numberOfSamplesPerChannel);

	printf("CfgSmpRate error: %i\n", error);

	/* Start task */
	error = NIDAQ::DAQmxStartTask(taskHandle);
	printf("Start error: %i\n", error);

	/* Read data */
	error = NIDAQ::DAQmxReadBinaryI16(
		taskHandle,
		numberOfSamplesPerChannel,
		10.0, //timeout
		DAQmx_Val_GroupByChannel,
		data,
		8000,
		&numSamplesRead,
		NULL);
	printf("Read error: %i\n", error);

	printf("Acquired %d points \n", numSamplesRead);

	for (NIDAQ::uInt32 i = 0; i < ai.size(); i++) {
		printf("%s Data: ", String("Channel") + String(i));
		for (NIDAQ::uInt32 j = 0; j<numberOfSamplesPerChannel; j++){
			printf("@%d\n", i*numberOfSamplesPerChannel + j);
			printf("%d,", data[i*numberOfSamplesPerChannel + j]);
		}
		printf("\n");
	}

	NIDAQ::DAQmxStopTask(taskHandle);
	NIDAQ::DAQmxClearTask(taskHandle);

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



