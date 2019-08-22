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

#include "NIDAQThread.h"
#include "NIDAQEditor.h"
#include <stdexcept>

DataThread* NIDAQThread::createDataThread(SourceNode *sn)
{
	return new NIDAQThread(sn);
}

GenericEditor* NIDAQThread::createEditor(SourceNode* sn)
{
    return new NIDAQEditor(sn, this, true);
}

NIDAQThread::NIDAQThread(SourceNode* sn) : DataThread(sn), inputAvailable(false)
{

	dm = new NIDAQmxDeviceManager();

	dm->scanForDevices();

	if (dm->getNumAvailableDevices() == 0)
	{
		//Okay for now as plugin-GUI handles source init runtime errors. 
		throw std::runtime_error("No NIDAQ devices detected!");
	}

	inputAvailable = true;
	openConnection();

}


NIDAQThread::~NIDAQThread()
{
}

int NIDAQThread::openConnection()
{

	mNIDAQ = new NIDAQmx(STR2CHR(dm->getDeviceFromIndex(0)));

	sourceBuffers.add(new DataBuffer(getNumAnalogInputs(), 10000));

	mNIDAQ->aiBuffer = sourceBuffers.getLast();

	sampleRateIndex = mNIDAQ->sampleRates.size() - 1;
	setSampleRate(sampleRateIndex);

	voltageRangeIndex = mNIDAQ->aiVRanges.size() - 1;
	setVoltageRange(voltageRangeIndex);

	return 0;

}

int NIDAQThread::getNumAvailableDevices()
{
	return dm->getNumAvailableDevices();
}

void NIDAQThread::selectFromAvailableDevices()
{

	PopupMenu deviceSelect;
	StringArray productNames;
	for (int i = 0; i < getNumAvailableDevices(); i++)
	{
		ScopedPointer<NIDAQmx> n = new NIDAQmx(STR2CHR(dm->getDeviceFromIndex(i)));
		if (!(n->getProductName() == getProductName()))
		{
			deviceSelect.addItem(productNames.size() + 1, "Swap to " + n->getProductName());
			productNames.add(n->getProductName());
		}
	}
	int selectedDeviceIndex = deviceSelect.show();
	if (selectedDeviceIndex == 0) //user clicked outside of popup window
		return;

	swapConnection(productNames[selectedDeviceIndex - 1]);
}

String NIDAQThread::getProductName() const
{
	return mNIDAQ->productName;
}

int NIDAQThread::swapConnection(String productName)
{

	if (!dm->getDeviceFromProductName(productName).isEmpty())
	{
		mNIDAQ = new NIDAQmx(STR2CHR(dm->getDeviceFromProductName(productName)));

		sourceBuffers.removeLast();
		sourceBuffers.add(new DataBuffer(getNumAnalogInputs(), 10000));
		mNIDAQ->aiBuffer = sourceBuffers.getLast();

		sampleRateIndex = mNIDAQ->sampleRates.size() - 1;
		setSampleRate(sampleRateIndex);

		voltageRangeIndex = mNIDAQ->aiVRanges.size() - 1;
		setVoltageRange(voltageRangeIndex);

		return 0;
	}
	return 1;

}

void NIDAQThread::toggleSourceType(int id)
{
	mNIDAQ->toggleSourceType(id);
}

SOURCE_TYPE NIDAQThread::getSourceTypeForInput(int index)
{
	return mNIDAQ->st[index];
}

void NIDAQThread::closeConnection()
{
}

int NIDAQThread::getNumAnalogInputs() const
{
	return mNIDAQ->ai.size();
}

int NIDAQThread::getNumDigitalInputs() const
{
	return mNIDAQ->di.size();
}

void NIDAQThread::toggleAIChannel(int index)
{
	mNIDAQ->aiChannelEnabled.set(index, !mNIDAQ->aiChannelEnabled[index]);
}

void NIDAQThread::toggleDIChannel(int index)
{
	mNIDAQ->diChannelEnabled.set(index, !mNIDAQ->diChannelEnabled[index]);
}

void NIDAQThread::setVoltageRange(int rangeIndex)
{
	voltageRangeIndex = rangeIndex;
	mNIDAQ->voltageRange = mNIDAQ->aiVRanges[rangeIndex];
}

void NIDAQThread::setSampleRate(int rateIndex)
{
	sampleRateIndex = rateIndex;
	mNIDAQ->samplerate = mNIDAQ->sampleRates[rateIndex];
}

int NIDAQThread::getVoltageRangeIndex()
{
	return voltageRangeIndex;
}

int NIDAQThread::getSampleRateIndex()
{
	return sampleRateIndex;
}

Array<String> NIDAQThread::getVoltageRanges()
{
	Array<String> voltageRanges;
	for (auto range : mNIDAQ->aiVRanges)
	{
		voltageRanges.add(String(range.vmin) + "-" + String(range.vmax) + " V");
	}
	return voltageRanges;
}

Array<String> NIDAQThread::getSampleRates()
{
	Array<String> sampleRates;
	for (auto rate : mNIDAQ->sampleRates)
	{
		sampleRates.add(String(rate) + " S/s");
	}
	return sampleRates;
}

bool NIDAQThread::foundInputSource()
{
    return inputAvailable;
}

XmlElement NIDAQThread::getInfoXml()
{

	//TODO: 
	XmlElement nidaq_info("NI-DAQmx");
	XmlElement* api_info = new XmlElement("API");
	//api_info->setAttribute("version", api.version);
	nidaq_info.addChildElement(api_info);

	return nidaq_info;

}

/** Initializes data transfer.*/
bool NIDAQThread::startAcquisition()
{
	//TODO:
	mNIDAQ->startThread();
	this->startThread();
    return true;
}

/** Stops data transfer.*/
bool NIDAQThread::stopAcquisition()
{

	if (isThreadRunning())
	{
		signalThreadShouldExit();
	}
	if (mNIDAQ->isThreadRunning())
	{
		mNIDAQ->signalThreadShouldExit();
	}
    return true;
}

void NIDAQThread::setDefaultChannelNames()
{

	for (int i = 0; i < getNumAnalogInputs(); i++)
	{
		ChannelCustomInfo info;
		info.name = "AI" + String(i + 1);
		info.gain = mNIDAQ->voltageRange.vmax / float(0x7fff);
		channelInfo.set(i, info);
	}

}

bool NIDAQThread::usesCustomNames() const
{
	return true;
}

/** Returns the number of virtual subprocessors this source can generate */
unsigned int NIDAQThread::getNumSubProcessors() const
{
	return 1;
}

/** Returns the number of continuous headstage channels the data source can provide.*/
int NIDAQThread::getNumDataOutputs(DataChannel::DataChannelTypes type, int subProcessorIdx) const
{
	if (subProcessorIdx > 0) return 0;

	int numChans = 0;

	if (type == DataChannel::ADC_CHANNEL)
	{
		numChans = getNumAnalogInputs();
	}

	return numChans;
}

/** Returns the number of TTL channels that each subprocessor generates*/
int NIDAQThread::getNumTTLOutputs(int subProcessorIdx) const
{
	return getNumDigitalInputs();
}

/** Returns the sample rate of the data source.*/
float NIDAQThread::getSampleRate(int subProcessorIdx) const
{
	return mNIDAQ->samplerate;
}

float NIDAQThread::getBitVolts(const DataChannel* chan) const
{
	return mNIDAQ->voltageRange.vmax / float(0x7fff);
}



void NIDAQThread::setTriggerMode(bool trigger)
{
    //TODO
}

void NIDAQThread::setAutoRestart(bool restart)
{
	//TODO
}

bool NIDAQThread::updateBuffer()
{
	return true;
}

