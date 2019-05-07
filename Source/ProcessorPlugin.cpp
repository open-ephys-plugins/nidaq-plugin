#include "ProcessorPlugin.h"

using namespace ProcessorPluginSpace;

//Change al names for the relevant ones, including "Processor Name"
ProcessorPlugin::ProcessorPlugin() : GenericProcessor("Processor Name")
{

}

ProcessorPlugin::~ProcessorPlugin()
{

}

void ProcessorPlugin::process(AudioSampleBuffer& buffer)
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

