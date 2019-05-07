#include "NI_DAQmx.h"

using namespace NIDAQ;

//Change al names for the relevant ones, including "Processor Name"
NI_DAQmx::NI_DAQmx() : GenericProcessor("NI_DAQmx")
{

}

NI_DAQmx::~NI_DAQmx()
{

}

void NI_DAQmx::process(AudioSampleBuffer& buffer)
{
	/** 
	If the processor needs to handle events, this method must be called onyle once per process call
	If spike processing is also needing, set the argument to true
	*/
	//checkForEvents(false);
	int numChannels = getNumOutputs();

	for (int chan = 0; chan < numChannels; chan++)
	{
		int numSamples = getNumSamples(chan);
		int64 timestamp = getTimestamp(chan);

		//Do whatever processing needed
	}
	 
}

