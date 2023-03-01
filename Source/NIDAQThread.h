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


#ifndef __NIDAQTHREAD_H__
#define __NIDAQTHREAD_H__

#include <DataThreadHeaders.h>
#include <stdio.h>
#include <string.h>
#include <cmath>

#include "nidaq-api/NIDAQmx.h"
#include "NIDAQComponents.h"

class SourceNode;
class NIDAQThread;
class NIDAQEditor;

/**

	Communicates with NI-DAQmx.

	@see DataThread, SourceNode

*/

class NIDAQThread : public DataThread
{

public:

	/** Constructor */
	NIDAQThread(SourceNode* sn);

	/** Destructor */
	~NIDAQThread();

	/** Create the DataThread */
	static DataThread* createDataThread(SourceNode* sn);

	/** Create the custom editor */
	std::unique_ptr<GenericEditor> createEditor(SourceNode* sn);

	/** Not used -- buffer is updated by NIDAQComponents class */
	bool updateBuffer();

	/** Returns true if the data source is connected, false otherwise.*/
	bool foundInputSource();

	/** Returns version and serial number info for hardware and API as XML.*/
	XmlElement getInfoXml();

	/** Called by ProcessorGraph to inform the thread whether the signal chain is loading */
	void initialize(bool signalChainIsLoading) override;

	// Connect to first available device
	int openConnection();

	// Helper method for loading...
	int swapConnection(String productName);

	/** Initializes data transfer.*/
	bool startAcquisition() override;

	/** Stops data transfer.*/
	bool stopAcquisition() override;

	/** Update settings */
	void updateSettings(OwnedArray<ContinuousChannel>* continuousChannels,
		OwnedArray<EventChannel>* eventChannels,
		OwnedArray<SpikeChannel>* spikeChannels,
		OwnedArray<DataStream>* dataStreams,
		OwnedArray<DeviceInfo>* devices,
		OwnedArray<ConfigurationObject>* configurationObjects) override;

	String getProductName() const;

	/** Input channel info */
	int getNumAnalogInputs() const;
	int getNumDigitalInputs() const;

	Array<String> getVoltageRanges();
	int getVoltageRangeIndex();

	Array<float> getSampleRates();
	int getSampleRateIndex();

	void toggleAIChannel(int channelIndex);
	void toggleDIChannel(int channelIndex);

	SOURCE_TYPE getSourceTypeForInput(int index);
	void toggleSourceType(int id);

	int getNumAvailableDevices();
	void selectFromAvailableDevices();

	/** Sets the voltage range of the data source. */
	void setVoltageRange(int rangeIndex);

	/** Sets the sample rate of the data source. */
	void setSampleRate(int rateIndex);

	/** Returns the sample rate of the data source.*/
	float getSampleRate();

	/** Responds to broadcast messages sent during acquisition */
	void handleBroadcastMessage(String msg) override;

	/** Responds to config messages sent while acquisition is stopped */
	String handleConfigMessage(String msg) override;

	CriticalSection* getMutex()
	{
		return &displayMutex;
	}



	friend class AIButton;
	friend class DIButton;
	friend class SourceTypeButton;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NIDAQThread);

private:

	/* Handle to NIDAQ API info */
	NIDAQAPI api;

	NIDAQEditor* editor;

	/* Manages connected NIDAQ devices */
	ScopedPointer<NIDAQmxDeviceManager> dm; 

	/* Flag any available devices */
	bool inputAvailable;

	/* Handle to current NIDAQ device */
	ScopedPointer<NIDAQmx> mNIDAQ;

	/* Array of source streams -- one per connected NIDAQ device */
	OwnedArray<DataStream> sourceStreams;

	/* Handle to input channels */
	Array<AnalogIn> ai;
	Array<DigitalIn> di;

	/* Selectable device properties */
	int sampleRateIndex;
	int voltageRangeIndex;

	bool isRecording;

	CriticalSection displayMutex;

	void closeConnection();

	Array<float> fillPercentage;

};

#endif  // __NIDAQTHREAD_H__
