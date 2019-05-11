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

DataThread* NIDAQThread::createDataThread(SourceNode *sn)
{
	return new NIDAQThread(sn);
}

GenericEditor* NIDAQThread::createEditor(SourceNode* sn)
{
    return new NIDAQEditor(sn, this, true);
}

NIDAQThread::NIDAQThread(SourceNode* sn) : DataThread(sn), recordingTimer(this)
{
	progressBar = new ProgressBar(initializationProgress);
}

NIDAQThread::~NIDAQThread()
{
    closeConnection();
}

void NIDAQThread::openConnection()
{
	if (false)
	{
		//Simulate connection
		for (int i = 0; i < getNumAnalogInputs(); i++)
		{
			ai.add(AnalogIn(i));
		}

		for (int i = 0; i < getNumDigitalInputs(); i++)
		{
			di.add(DigitalIn(i));
		}
	}

}

void NIDAQThread::closeConnection()
{

}

int NIDAQThread::getNumAnalogInputs()
{
	return 10;
}

int NIDAQThread::getNumDigitalInputs()
{
	return 8;
}

/** Returns true if the data source is connected, false otherwise.*/
bool NIDAQThread::foundInputSource()
{
    return inputAvailable;
}

XmlElement NIDAQThread::getInfoXml()
{

	XmlElement nidaq_info("NI-DAQmx");

	XmlElement* api_info = new XmlElement("API");
	//api_info->setAttribute("version", api.version);
	nidaq_info.addChildElement(api_info);

	return nidaq_info;

}


String NIDAQThread::getInfoString()
{

	String infoString;

	infoString += "TEST";

	return infoString;

}

/** Initializes data transfer.*/
bool NIDAQThread::startAcquisition()
{
	//TODO:
    return false;
}

void NIDAQThread::timerCallback()
{
	//TODO:
}

void NIDAQThread::startRecording()
{
	//TODO:
}

void NIDAQThread::stopRecording()
{
	//TODO:
}

/** Stops data transfer.*/
bool NIDAQThread::stopAcquisition()
{
	//TODO:
    return false;
}

void NIDAQThread::setSelectedInput()
{

}


void NIDAQThread::setDefaultChannelNames()
{
	//TODO:
}

bool NIDAQThread::usesCustomNames() const
{
	return false;
}


/** Returns the number of virtual subprocessors this source can generate */
unsigned int NIDAQThread::getNumSubProcessors() const
{
	//TODO?
	return 1;
}

/** Returns the number of continuous headstage channels the data source can provide.*/
int NIDAQThread::getNumDataOutputs(DataChannel::DataChannelTypes type, int subProcessorIdx) const
{
	//TODO?
	int numChans;

	if (type == DataChannel::DataChannelTypes::AUX_CHANNEL)
		numChans = 1;
	else
		numChans = 0;

	return numChans;
}

/** Returns the number of TTL channels that each subprocessor generates*/
int NIDAQThread::getNumTTLOutputs(int subProcessorIdx) const
{
	//TODO
	return 0;
}

/** Returns the sample rate of the data source.*/
float NIDAQThread::getSampleRate(int subProcessorIdx) const
{
	//TODO
	return 30000.0f;
}

/** Returns the volts per bit of the data source.*/
float NIDAQThread::getBitVolts(const DataChannel* chan) const
{
	//TODO
	return 0.1950000f;
}

void NIDAQThread::setTriggerMode(bool trigger)
{
    //TODO
}

void NIDAQThread::setRecordMode(bool record)
{
    //TODO
}

void NIDAQThread::setAutoRestart(bool restart)
{
	//TODO
}

void NIDAQThread::setDirectoryForInput(int id, File directory)
{
	//TODO
}

File NIDAQThread::getDirectoryForInput(int id)
{
	//TODO
	return File::getCurrentWorkingDirectory();
}

float NIDAQThread::getFillPercentage(int id)
{
	//TODO
	return 0.0f;
}

bool NIDAQThread::updateBuffer()
{
	//TODO
    return false;
}


RecordingTimer::RecordingTimer(NIDAQThread* t_)
{
	thread = t_;
}

void RecordingTimer::timerCallback()
{
	thread->startRecording();
	stopTimer();
}
