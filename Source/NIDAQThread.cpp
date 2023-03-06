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

std::unique_ptr<GenericEditor> NIDAQThread::createEditor(SourceNode* sn)
{

	std::unique_ptr<NIDAQEditor> ed = std::make_unique<NIDAQEditor>(sn, this);

	editor = ed.get();

	return ed;

}

NIDAQThread::NIDAQThread(SourceNode* sn) : DataThread(sn), inputAvailable(false)
{

	dm = new NIDAQmxDeviceManager();

	dm->scanForDevices();

	if (dm->getNumAvailableDevices() > 0 && dm->getDeviceAtIndex(0)->getName() != "SimulatedDevice")
		inputAvailable = true;

	openConnection();
}

NIDAQThread::~NIDAQThread()
{
}

void NIDAQThread::initialize(bool signalChainIsLoading)
{
	//Used in Neuropixels to initialize probes in background -- not needed for NIDAQ devices
	//editor->initialize(signalChainIsLoading);

}

String NIDAQThread::handleConfigMessage(String msg)
{
	//TODO:
	return " ";
}

void NIDAQThread::handleBroadcastMessage(String msg)
{
	//TODO
}

void NIDAQThread::updateSettings(OwnedArray<ContinuousChannel>* continuousChannels,
	OwnedArray<EventChannel>* eventChannels,
	OwnedArray<SpikeChannel>* spikeChannels,
	OwnedArray<DataStream>* dataStreams,
	OwnedArray<DeviceInfo>* devices,
	OwnedArray<ConfigurationObject>* configurationObjects)
{

	LOGC("NIDAQThread::Updating settings...");

	if (sourceStreams.size() == 0) // initialize data streams
	{

		DataStream::Settings settings
		{
			getProductName(),
			"Analog input channels from a NIDAQ device",
			"identifier",

			getSampleRate()

		};

		sourceStreams.add(new DataStream(settings));

	}
	else if (sourceStreams[0]->getSampleRate() != getSampleRate())
	{

		sourceStreams.clear();

		DataStream::Settings settings
		{
			getProductName(),
			"Analog input channels from a NIDAQ device",
			"identifier",

			getSampleRate()

		};

		sourceStreams.add(new DataStream(settings));

	}

	dataStreams->clear();
	eventChannels->clear();
	continuousChannels->clear();
	spikeChannels->clear();
	devices->clear();
	configurationObjects->clear();

	for (int i = 0; i < sourceStreams.size(); i++)
	{
		DataStream* currentStream = sourceStreams[i];

		currentStream->setName(getProductName());

		currentStream->clearChannels();

		for (int ch = 0; ch < getNumActiveAnalogInputs(); ch++)
		{

			if (mNIDAQ->ai[ch]->isEnabled())
			{

				float bitVolts = mNIDAQ->getVoltageRange().max / float(0x7fff);

				ContinuousChannel::Settings settings{
					ContinuousChannel::Type::ADC,
					"AI" + String(ch),
					"Analog Input channel from a NIDAQ device",
					"identifier",

					bitVolts,

					currentStream
				};

				continuousChannels->add(new ContinuousChannel(settings));

			}

		}

		EventChannel::Settings settings{
			EventChannel::Type::TTL,
			getProductName() + "Digital Input Line",
			"Digital Line from a NIDAQ device containing " + String(mNIDAQ->di.size()) + " inputs",
			"identifier",
			currentStream,
			mNIDAQ->di.size()
		};

		eventChannels->add(new EventChannel(settings));

		dataStreams->add(new DataStream(*currentStream)); // copy existing stream

	}

}

Array<NIDAQDevice*> NIDAQThread::getDevices()
{
	Array<NIDAQDevice*> deviceList;

	for (int i = 0; i < dm->getNumAvailableDevices(); i++)
		deviceList.add(dm->getDeviceAtIndex(i));

	return deviceList;
}

int NIDAQThread::openConnection()
{

	mNIDAQ = new NIDAQmx(dm->getDeviceAtIndex(0));

	sourceBuffers.add(new DataBuffer(getNumActiveAnalogInputs(), 10000));

	mNIDAQ->aiBuffer = sourceBuffers.getLast();

	sampleRateIndex = mNIDAQ->sampleRates.size() - 1;
	setSampleRate(sampleRateIndex);

	voltageRangeIndex = mNIDAQ->device->voltageRanges.size() - 1;
	setVoltageRange(voltageRangeIndex);

	return 0;

}

void NIDAQThread::selectFromAvailableDevices()
{

	PopupMenu deviceSelect;
	StringArray productNames;

	for (int i = 0; i < getNumAvailableDevices(); i++)
	{
		String productName = dm->getDeviceAtIndex(i)->productName;
		if (!(productName == getProductName()))
		{
			productNames.add(productName);
			deviceSelect.addItem(productNames.size(), "Swap to " + productName);
		}
	}
	int selectedDeviceIndex = deviceSelect.show();

	if (selectedDeviceIndex == 0) //user clicked outside of popup window
		return;

	String selectedProduct = productNames[selectedDeviceIndex - 1];

	for (auto& dev : getDevices())
	{
		if (dev->productName == selectedProduct)
		{
			swapConnection(dev->getName());
			break;
		}
	}

}

int NIDAQThread::swapConnection(String deviceName)
{

	int deviceIdx = -1;

	for (auto& dev : getDevices())
	{

		deviceIdx++;

		if (dev->getName() == deviceName)
		{
			mNIDAQ = new NIDAQmx(dev);

			sourceBuffers.removeLast();
			sourceBuffers.add(new DataBuffer(getNumActiveAnalogInputs(), 10000));
			mNIDAQ->aiBuffer = sourceBuffers.getLast();

			deviceIndex = deviceIdx;
			setDeviceIndex(deviceIndex);

			sampleRateIndex = mNIDAQ->sampleRates.size() - 1;
			setSampleRate(sampleRateIndex);

			voltageRangeIndex = mNIDAQ->device->voltageRanges.size() - 1;
			setVoltageRange(voltageRangeIndex);

			sourceStreams.clear();

			break;
		}
	}

	setDeviceIndex(deviceIdx);

	return deviceIdx;

}

void NIDAQThread::toggleSourceType(int id)
{
	mNIDAQ->toggleSourceType(id);
}

SOURCE_TYPE NIDAQThread::getSourceTypeForInput(int index)
{
	return mNIDAQ->ai[index]->getSourceType();
}

void NIDAQThread::closeConnection()
{
}

bool NIDAQThread::toggleAIChannel(int index)
{
	mNIDAQ->ai[index]->setEnabled(!mNIDAQ->ai[index]->isEnabled());
	return mNIDAQ->ai[index]->isEnabled();
}

bool NIDAQThread::toggleDIChannel(int index)
{
	mNIDAQ->di[index]->setEnabled(!mNIDAQ->di[index]->isEnabled());
	return mNIDAQ->di[index]->isEnabled();
}

void NIDAQThread::setDeviceIndex(int index)
{
	deviceIndex = index;
}

void NIDAQThread::setVoltageRange(int rangeIndex)
{
	voltageRangeIndex = rangeIndex;
	mNIDAQ->setVoltageRange(rangeIndex);
}

void NIDAQThread::setSampleRate(int rateIndex)
{
	sampleRateIndex = rateIndex;
	mNIDAQ->setSampleRate(rateIndex);
}

float NIDAQThread::getSampleRate()
{
	return mNIDAQ->getSampleRate();
}

Array<SettingsRange> NIDAQThread::getVoltageRanges()
{
	return mNIDAQ->device->voltageRanges;
}

Array<NIDAQ::float64> NIDAQThread::getSampleRates()
{
	return mNIDAQ->sampleRates;
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
	
	mNIDAQ->startThread();

    return true;
}

/** Stops data transfer.*/
bool NIDAQThread::stopAcquisition()
{

	if (mNIDAQ->isThreadRunning())
	{
		mNIDAQ->signalThreadShouldExit();
	}
    return true;
}

/* DEPRECATED 

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

// Returns the number of virtual subprocessors this source can generate 
unsigned int NIDAQThread::getNumSubProcessors() const
{
	return 1;
}

// Returns the number of continuous headstage channels the data source can provide.
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

// Returns the number of TTL channels that each subprocessor generates
int NIDAQThread::getNumTTLOutputs(int subProcessorIdx) const
{
	return getNumDigitalInputs();
}


// Returns the sample rate of the data source.
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
*/

bool NIDAQThread::updateBuffer()
{
	return true;
}