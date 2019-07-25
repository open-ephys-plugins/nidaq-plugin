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

/**

	Communicates with NI-DAQmx.

	@see DataThread, SourceNode

*/

class NIDAQThread : public DataThread
{

public:

	NIDAQThread(SourceNode* sn);
	~NIDAQThread();

	bool updateBuffer();

	void updateChannels();

	/** Returns true if the data source is connected, false otherwise.*/
	bool foundInputSource();

	/** Returns version and serial number info for hardware and API as XML.*/
	XmlElement getInfoXml();

	// Connect to first available device
	int openConnection();

	// Helper method for loading...
	int swapConnection(String productName);

	/** Initializes data transfer.*/
	bool startAcquisition() override;

	/** Stops data transfer.*/
	bool stopAcquisition() override;

	String getProductName() const;

	/** Input channel info */
	int getNumAnalogInputs() const;
	int getNumDigitalInputs() const;

	Array<String> getVoltageRanges();
	int getVoltageRangeIndex();

	Array<String> getSampleRates();
	int getSampleRateIndex();

	void toggleAIChannel(int channelIndex);
	void toggleDIChannel(int channelIndex);

	SOURCE_TYPE getSourceTypeForInput(int index);
	void toggleSourceType(int id);

	int getNumAvailableDevices();
	void selectFromAvailableDevices();

	void selectDevice();

	/** Returns the number of virtual subprocessors this source can generate */
	unsigned int getNumSubProcessors() const override;

	/** Returns the number of continuous headstage channels the data source can provide.*/
	int getNumDataOutputs(DataChannel::DataChannelTypes type, int subProcessorIdx) const override;

	/** Returns the number of TTL channels that each subprocessor generates*/
	int getNumTTLOutputs(int subProcessorIdx) const override;

	/** Sets the voltage range of the data source. */
	void setVoltageRange(int rangeIndex);

	/** Sets the sample rate of the data source. */
	void setSampleRate(int rateIndex);

	/** Returns the sample rate of the data source.*/
	float getSampleRate(int subProcessorIdx) const override;

	/** Returns the volts per bit of the data source.*/
	float getBitVolts(const DataChannel* chan) const override;

	float getAdcBitVolts();

	/** Used to set default channel names.*/
	void setDefaultChannelNames() override;

	/** Used to override default channel names.*/
	bool usesCustomNames() const override;

	/** Toggles between internal and external triggering. */
	void setTriggerMode(bool trigger);

	/** Toggles between auto-restart setting. */
	void setAutoRestart(bool restart);

	/** Sets the currently selected input */
	void setSelectedInput();

	/** Starts recording.*/
	void startRecording();

	/** Stops recording.*/
	void stopRecording();

	/** Set directory for input */
	void setDirectoryForInput(int id, File directory);

	/** Get directory for input */
	File getDirectoryForInput(int id);

	CriticalSection* getMutex()
	{
		return &displayMutex;
	}

	static DataThread* createDataThread(SourceNode* sn);

	GenericEditor* createEditor(SourceNode* sn);

	friend class AIButton;
	friend class DIButton;
	friend class SourceTypeButton;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NIDAQThread);

private:

	/* Handle to NIDAQ API info */
	NIDAQAPI api;

	/* Manages connected NIDAQ devices */
	ScopedPointer<NIDAQmxDeviceManager> dm; 

	/* Flag any available devices */
	bool inputAvailable;

	/* Handle to chosen NIDAQ device */
	ScopedPointer<NIDAQmx> mNIDAQ;

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
