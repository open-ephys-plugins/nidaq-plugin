/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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

#include <chrono>
#include <math.h>

#include "NIDAQComponents.h"

AnalogInput::AnalogInput (String name, NIDAQ::int32 termCfgs) : InputChannel (name)
{
    sourceTypes.clear();

    if (termCfgs & DAQmx_Val_Bit_TermCfg_RSE)
        sourceTypes.add (SOURCE_TYPE::RSE);

    if (termCfgs & DAQmx_Val_Bit_TermCfg_NRSE)
        sourceTypes.add (SOURCE_TYPE::NRSE);

    if (termCfgs & DAQmx_Val_Bit_TermCfg_Diff)
        sourceTypes.add (SOURCE_TYPE::DIFF);

    if (termCfgs & DAQmx_Val_Bit_TermCfg_PseudoDIFF)
        sourceTypes.add (SOURCE_TYPE::PSEUDO_DIFF);
}

static int32 GetTerminalNameWithDevPrefix (NIDAQ::TaskHandle taskHandle, const char terminalName[], char triggerName[]);

static int32 GetTerminalNameWithDevPrefix (NIDAQ::TaskHandle taskHandle, const char terminalName[], char triggerName[])
{
    NIDAQ::int32 error = 0;
    char device[256];
    NIDAQ::int32 productCategory;
    NIDAQ::uInt32 numDevices, i = 1;

    DAQmxErrChk (NIDAQ::DAQmxGetTaskNumDevices (taskHandle, &numDevices));
    while (i <= numDevices)
    {
        DAQmxErrChk (NIDAQ::DAQmxGetNthTaskDevice (taskHandle, i++, device, 256));
        DAQmxErrChk (NIDAQ::DAQmxGetDevProductCategory (device, &productCategory));
        if (productCategory != DAQmx_Val_CSeriesModule && productCategory != DAQmx_Val_SCXIModule)
        {
            *triggerName++ = '/';
            strcat (strcat (strcpy (triggerName, device), "/"), terminalName);
            break;
        }
    }

Error:
    return error;
}

void NIDAQmxDeviceManager::scanForDevices()
{
    devices.clear();

    char data[2048] = { 0 };
    NIDAQ::DAQmxGetSysDevNames (data, sizeof (data));

    StringArray deviceList;
    deviceList.addTokens (&data[0], ", ", "\"");

    StringArray deviceNames;
    StringArray productList;

    for (int i = 0; i < deviceList.size(); i++)
    {
        if (deviceList[i].length() > 0)
        {
            String deviceName = deviceList[i].toUTF8();

            /* Get product name */
            char pname[2048] = { 0 };
            NIDAQ::DAQmxGetDevProductType (STR2CHR (deviceName), &pname[0], sizeof (pname));
            devices.add (new NIDAQDevice (deviceName));
            devices.getLast()->productName = String (&pname[0]);
        }
    }

    if (! devices.size())
        devices.add (new NIDAQDevice ("Simulated")); // TODO: Make SimulatedClass derived from NIDAQDevice
}

int NIDAQmxDeviceManager::getDeviceIndexFromName (String name)
{
    for (int i = 0; i < devices.size(); i++)
        if (devices[i]->getName() == name)
            return i;

    return -1;
}

NIDAQmx::NIDAQmx (NIDAQDevice* device_)
    : Thread ("NIDAQmx-" + String (device_->getName())), device (device_)
{
    connect();

    digitalReadSize = device->digitalReadSize;

    // Pre-define reasonable sample rates
    float sample_rates[NUM_SAMPLE_RATES] = {
        1000.0f, 1250.0f, 1500.0f, 2000.0f, 2500.0f, 3000.0f, 3330.0f, 4000.0f, 5000.0f, 6250.0f, 8000.0f, 10000.0f, 12500.0f, 15000.0f, 20000.0f, 25000.0f, 30000.0f, 40000.0f
    };

    sampleRates.clear();

    int idx = 0;
    while (sample_rates[idx] <= device->sampleRateRange.max && idx < NUM_SAMPLE_RATES)
        sampleRates.add (sample_rates[idx++]);

    // Default to highest sample rate
    sampleRateIndex = sampleRates.size() - 1;

    // Default to largest voltage range
    voltageRangeIndex = device->voltageRanges.size() - 1;
}

void NIDAQmx::connect()
{
    String deviceName = device->getName();

    if (deviceName == "Simulated")
    {
        device->isUSBDevice = false;
        device->sampleRateRange = SettingsRange (1000.0f, 30000.0f);
        device->voltageRanges.add (SettingsRange (-10.0f, 10.0f));
        device->productName = String ("No Device Detected");
    }
    else
    {
        /* Get category type */
        NIDAQ::DAQmxGetDevProductCategory (STR2CHR (deviceName), &device->deviceCategory);
        LOGD ("Product Category: ", device->deviceCategory);

        device->isUSBDevice = device->productName.contains ("USB");

        device->digitalReadSize = device->isUSBDevice ? 32 : 8;

        NIDAQ::DAQmxGetDevProductNum (STR2CHR (deviceName), &device->productNum);
        LOGD ("Product Num: ", device->productNum);

        NIDAQ::DAQmxGetDevSerialNum (STR2CHR (deviceName), &device->serialNum);
        LOGD ("Serial Num: ", device->serialNum);

        /* Get simultaneous sampling supported */
        NIDAQ::bool32 supported = false;
        NIDAQ::DAQmxGetDevAISimultaneousSamplingSupported (STR2CHR (deviceName), &supported);
        device->simAISamplingSupported = supported;
        LOGD ("Simultaneous sampling supported: ", supported ? "YES" : "NO");

        /* Get device sample rates */
        NIDAQ::float64 smin;
        NIDAQ::DAQmxGetDevAIMinRate (STR2CHR (deviceName), &smin);
        LOGD ("Min sample rate: ", smin);

        NIDAQ::float64 smaxs;
        NIDAQ::DAQmxGetDevAIMaxSingleChanRate (STR2CHR (deviceName), &smaxs);
        LOGD ("Max single channel sample rate: ", smaxs);

        NIDAQ::float64 smaxm;
        NIDAQ::DAQmxGetDevAIMaxMultiChanRate (STR2CHR (deviceName), &smaxm);
        LOGD ("Max multi channel sample rate: ", smaxm);

        NIDAQ::float64 data[512];
        NIDAQ::DAQmxGetDevAIVoltageRngs (STR2CHR (deviceName), &data[0], sizeof (data));

        // Get available voltage ranges
        device->voltageRanges.clear();
        LOGD ("Detected voltage ranges: \n");
        for (int i = 0; i < 512; i += 2)
        {
            NIDAQ::float64 vmin = data[i];
            NIDAQ::float64 vmax = data[i + 1];
            if (vmin == vmax || abs (vmin) < 1e-10 || vmax < 1e-2)
                break;
            device->voltageRanges.add (SettingsRange (vmin, vmax));
        }

        NIDAQ::int32 error = 0;
        char errBuff[ERR_BUFF_SIZE] = { '\0' };

        char ai_channel_data[2048];
        NIDAQ::DAQmxGetDevAIPhysicalChans (STR2CHR (device->getName()), &ai_channel_data[0], sizeof (ai_channel_data));

        StringArray channel_list;
        channel_list.addTokens (&ai_channel_data[0], ", ", "\"");

        device->numAIChannels = 0;
        ai.clear();

        LOGD ("Detected ", channel_list.size(), " analog input channels");

        for (int i = 0; i < channel_list.size(); i++)
        {
            if (channel_list[i].length() > 0)
            {
                /* Get channel termination */
                NIDAQ::int32 termCfgs;
                NIDAQ::DAQmxGetPhysicalChanAITermCfgs (channel_list[i].toUTF8(), &termCfgs);

                String name = channel_list[i].toRawUTF8();

                String terminalConfigurations = String::toHexString (termCfgs);

                ai.add (new AnalogInput (name, termCfgs));

                if (device->numAIChannels++ <= numActiveAnalogInputs)
                {
                    ai.getLast()->setAvailable (true);
                    ai.getLast()->setEnabled (true);
                }

                LOGD ("Adding analog input channel: ", name, " with terminal config: ", " (", termCfgs, ") enabled: ", ai.getLast()->isEnabled() ? "YES" : "NO");
            }
        }

        // Get ADC resolution for each voltage range (throwing error as is)
        NIDAQ::TaskHandle adcResolutionQuery;

        NIDAQ::DAQmxCreateTask ("ADCResolutionQuery", &adcResolutionQuery);

        SettingsRange vRange;

        for (int i = 0; i < device->voltageRanges.size(); i++)
        {
            vRange = device->voltageRanges[i];

            DAQmxErrChk (NIDAQ::DAQmxCreateAIVoltageChan (
                adcResolutionQuery, // task handle
                STR2CHR (ai[i]->getName()), // NIDAQ physical channel name (e.g. dev1/ai1)
                "", // user-defined channel name (optional)
                DAQmx_Val_Cfg_Default, // input terminal configuration
                vRange.min, // min input voltage
                vRange.max, // max input voltage
                DAQmx_Val_Volts, // voltage units
                NULL));

            NIDAQ::float64 adcResolution;
            DAQmxErrChk (NIDAQ::DAQmxGetAIResolution (adcResolutionQuery, STR2CHR (ai[i]->getName()), &adcResolution));

            device->adcResolutions.add (adcResolution);
        }

        NIDAQ::DAQmxStopTask (adcResolutionQuery);
        NIDAQ::DAQmxClearTask (adcResolutionQuery);

        // Get Digital Input Channels

        char di_channel_data[2048];
        // NIDAQ::DAQmxGetDevTerminals(STR2CHR(deviceName), &data[0], sizeof(data)); //gets all terminals
        // NIDAQ::DAQmxGetDevDIPorts(STR2CHR(deviceName), &data[0], sizeof(data));	//gets line name
        NIDAQ::DAQmxGetDevDILines (STR2CHR (deviceName), &di_channel_data[0], sizeof (di_channel_data)); // gets ports on line
        LOGD ("Found digital inputs: ");

        channel_list.clear();
        channel_list.addTokens (&di_channel_data[0], ", ", "\"");

        device->digitalPortNames.clear();
        device->digitalPortStates.clear();
        device->numDIChannels = 0;
        di.clear();

        for (int i = 0; i < channel_list.size(); i++)
        {
            StringArray channel_type;
            channel_type.addTokens (channel_list[i], "/", "\"");
            if (channel_list[i].length() > 0)
            {
                String fullName = channel_list[i].toRawUTF8();

                String lineName = fullName.fromFirstOccurrenceOf ("/", false, false);
                String portName = fullName.upToLastOccurrenceOf ("/", false, false);

                // Add port to list of ports
                if (! device->digitalPortNames.contains (portName.toRawUTF8()))
                {
                    device->digitalPortNames.add (portName.toRawUTF8());
                    if (device->numDIChannels < numActiveDigitalInputs)
                        device->digitalPortStates.add (true);
                    else
                        device->digitalPortStates.add (false);
                }

                di.add (new InputChannel (fullName));

                di.getLast()->setAvailable (true);
                if (device->numDIChannels < numActiveDigitalInputs)
                    di.getLast()->setEnabled (true);

                device->numDIChannels++;
            }
        }

        // Set sample rate range
        NIDAQ::float64 smax = smaxm;
        if (! device->simAISamplingSupported)
            smax /= numActiveAnalogInputs;

        device->sampleRateRange = SettingsRange (smin, smax);

    Error:

        if (DAQmxFailed (error))
            NIDAQ::DAQmxGetExtendedErrorInfo (errBuff, ERR_BUFF_SIZE);

        if (adcResolutionQuery != 0)
        {
            // DAQmx Stop Code
            NIDAQ::DAQmxStopTask (adcResolutionQuery);
            NIDAQ::DAQmxClearTask (adcResolutionQuery);
        }

        if (DAQmxFailed (error))
            LOGE ("DAQmx Error: ", errBuff);
        fflush (stdout);

        return;
    }
}

uint32 NIDAQmx::getActiveDigitalLines()
{
    if (! getNumActiveDigitalInputs())
        return 0;

    uint32 linesEnabled = 0;
    for (int i = 0; i < di.size(); i++)
    {
        if (di[i]->isEnabled())
            linesEnabled += pow (2, i);
    }
    return linesEnabled;
}

void NIDAQmx::run()
{
    /* Derived from NIDAQmx: ANSI C Example program: ContAI-ReadDigChan.c */

    /* Single task to handle all analog inputs */
    NIDAQ::TaskHandle taskHandleAI = 0;

    /* Potentially multiple tasks to handle different digital line properties */
    std::vector<NIDAQ::TaskHandle> taskHandlesDI = std::vector<NIDAQ::TaskHandle>();

    NIDAQ::int32 error = 0;
    char errBuff[ERR_BUFF_SIZE] = { '\0' };

    /**************************************/
    /********CONFIG ANALOG CHANNELS********/
    /**************************************/

    NIDAQ::int32 ai_read = 0;
    static int totalAIRead = 0;

    aiBuffer->clear();
    ai_data.malloc (CHANNEL_BUFFER_SIZE * numActiveAnalogInputs, sizeof (NIDAQ::float64));

    eventCodes.malloc (CHANNEL_BUFFER_SIZE, sizeof (NIDAQ::uInt32));

    /* Create an analog input task */
    if (device->isUSBDevice)
        DAQmxErrChk (NIDAQ::DAQmxCreateTask (STR2CHR ("AITask_USB" + getSerialNumber()), &taskHandleAI));
    else
        DAQmxErrChk (NIDAQ::DAQmxCreateTask (STR2CHR ("AITask_PXI" + getSerialNumber()), &taskHandleAI));

    /* Create a voltage channel for each analog input */
    for (int i = 0; i < numActiveAnalogInputs; i++)
    {
        NIDAQ::int32 termConfig;

        switch (ai[i]->getSourceType())
        {
            case SOURCE_TYPE::RSE:
                termConfig = DAQmx_Val_RSE;
                break;
            case SOURCE_TYPE::NRSE:
                termConfig = DAQmx_Val_NRSE;
                break;
            case SOURCE_TYPE::DIFF:
                termConfig = DAQmx_Val_Diff;
                break;
            case SOURCE_TYPE::PSEUDO_DIFF:
                termConfig = DAQmx_Val_PseudoDiff;
                break;
            default:
                termConfig = DAQmx_Val_Cfg_Default;
                break;
        }

        SettingsRange voltageRange = device->voltageRanges[voltageRangeIndex];

        DAQmxErrChk (NIDAQ::DAQmxCreateAIVoltageChan (
            taskHandleAI, // task handle
            STR2CHR (ai[i]->getName()), // NIDAQ physical channel name (e.g. dev1/ai1)
            "", // user-defined channel name (optional)
            termConfig, // input terminal configuration
            voltageRange.min, // min input voltage
            voltageRange.max, // max input voltage
            DAQmx_Val_Volts, // voltage units
            NULL));
    }

    /* Configure sample clock timing */
    DAQmxErrChk (NIDAQ::DAQmxCfgSampClkTiming (
        taskHandleAI,
        "", // source : NULL means use internal clock
        getSampleRate(), // rate : samples per second per channel
        DAQmx_Val_Rising, // activeEdge : (DAQmc_Val_Rising || DAQmx_Val_Falling)
        DAQmx_Val_ContSamps, // sampleMode : (DAQmx_Val_FiniteSamps || DAQmx_Val_ContSamps || DAQmx_Val_HWTimedSinglePoint)
        numActiveAnalogInputs * CHANNEL_BUFFER_SIZE)); // sampsPerChanToAcquire :
    // If sampleMode == DAQmx_Val_FiniteSamps : # of samples to acquire for each channel
    // Elif sampleMode == DAQmx_Val_ContSamps : circular buffer size

    /* Get handle to analog trigger to sync with digital inputs */
    char trigName[256];
    DAQmxErrChk (GetTerminalNameWithDevPrefix (taskHandleAI, "ai/SampleClock", trigName));

    /************************************/
    /********CONFIG DIGITAL LINES********/
    /************************************/

    NIDAQ::int32 di_read = 0;
    static int totalDIRead = 0;

    if (numActiveDigitalInputs)
    {
        LOGD ("Active digital mask: ", getActiveDigitalLines());

        char ports[2048];
        NIDAQ::DAQmxGetDevDIPorts (STR2CHR (device->getName()), &ports[0], sizeof (ports));

        LOGD ("Detected ports: ", ports);

        StringArray port_list;
        port_list.addTokens (&ports[0], ", ", "\"");

        for (int i = 0; i < device->digitalPortNames.size(); i++)
            LOGD (device->digitalPortNames[i], " : ", device->digitalPortStates[i]);

        int portIdx = 0;
        for (auto& port : port_list)
        {
            if (port.length() && portIdx < di.size() / PORT_SIZE && device->digitalPortStates[portIdx])
            {
                NIDAQ::TaskHandle taskHandleDI = 0;
                /* Create a digital input task using device serial number to gurantee unique task name per device */
                if (device->isUSBDevice)
                    DAQmxErrChk (NIDAQ::DAQmxCreateTask (STR2CHR ("DITask_USB" + getSerialNumber() + "port" + std::to_string (portIdx)), &taskHandleDI));
                else
                    DAQmxErrChk (NIDAQ::DAQmxCreateTask (STR2CHR ("DITask_PXI" + getSerialNumber() + "port" + std::to_string (portIdx)), &taskHandleDI));

                /* Create a channel for each digital input */
                DAQmxErrChk (NIDAQ::DAQmxCreateDIChan (
                    taskHandleDI,
                    STR2CHR (port),
                    "",
                    DAQmx_Val_ChanForAllLines));

                /* In general, only Port0 supports hardware timing */
                if (portIdx == 0)
                {
                    if (numActiveAnalogInputs && numActiveDigitalInputs) // USB devices do not have an internal clock and instead use CPU, so we can't configure the sample clock timing
                        DAQmxErrChk (NIDAQ::DAQmxCfgSampClkTiming (
                            taskHandleDI, // task handle
                            trigName, // source : NULL means use internal clock, we will sync to analog input clock
                            getSampleRate(), // rate : samples per second per channel
                            DAQmx_Val_Rising, // activeEdge : (DAQmc_Val_Rising || DAQmx_Val_Falling)
                            DAQmx_Val_ContSamps, // sampleMode : (DAQmx_Val_FiniteSamps || DAQmx_Val_ContSamps || DAQmx_Val_HWTimedSinglePoint)
                            CHANNEL_BUFFER_SIZE)); // sampsPerChanToAcquire : want to sync with analog samples per channel
                    // If sampleMode == Dmx_Val_FiniteSamps : # of samples to acquire for each channel
                    // Elif sampleMode == DAQAQmx_Val_ContSamps : circular buffer size
                }

                taskHandlesDI.push_back (taskHandleDI);
            }

            if (port.length())
                portIdx++;
        }
    }

    LOGD ("Is USB Device: ", device->isUSBDevice);

    // This order is necessary to get the timing right
    if (numActiveAnalogInputs)
        DAQmxErrChk (NIDAQ::DAQmxTaskControl (taskHandleAI, DAQmx_Val_Task_Commit));
    if (numActiveDigitalInputs)
    {
        for (auto& taskHandleDI : taskHandlesDI)
            DAQmxErrChk (NIDAQ::DAQmxTaskControl (taskHandleDI, DAQmx_Val_Task_Commit));
    }

    if (numActiveDigitalInputs)
    {
        for (auto& taskHandleDI : taskHandlesDI)
            DAQmxErrChk (NIDAQ::DAQmxStartTask (taskHandleDI));
    }
    if (numActiveAnalogInputs)
        DAQmxErrChk (NIDAQ::DAQmxStartTask (taskHandleAI));

    NIDAQ::int32 numSampsPerChan = CHANNEL_BUFFER_SIZE;
    if (device->isUSBDevice)
        numSampsPerChan = 100;

    NIDAQ::int32 arraySizeInSamps = numActiveAnalogInputs * numSampsPerChan;
    NIDAQ::float64 timeout = 5.0;

    uint64 linesEnabled = 0;
    double ts;

    ai_timestamp = 0;
    eventCode = 0;

    while (! threadShouldExit())
    {
        if (numActiveAnalogInputs)
            DAQmxErrChk (NIDAQ::DAQmxReadAnalogF64 (
                taskHandleAI,
                numSampsPerChan,
                timeout,
                DAQmx_Val_GroupByScanNumber, // DAQmx_Val_GroupByScanNumber
                ai_data,
                arraySizeInSamps,
                &ai_read,
                NULL));

        if (getActiveDigitalLines() > 0)
        {
            for (int i = 0; i < CHANNEL_BUFFER_SIZE; i++)
                eventCodes[i] = 0;

            int portIdx = 0;
            for (auto& taskHandleDI : taskHandlesDI)
            {
                if (digitalReadSize == 32)
                {
                    NIDAQ::uInt32 di_data_32_[CHANNEL_BUFFER_SIZE];
                    DAQmxErrChk (NIDAQ::DAQmxReadDigitalU32 (
                        taskHandleDI,
                        numSampsPerChan,
                        timeout,
                        DAQmx_Val_GroupByScanNumber,
                        di_data_32_,
                        numSampsPerChan,
                        &di_read,
                        NULL));
                    for (int i = 0; i < numSampsPerChan; i++)
                        eventCodes[i] |= (di_data_32_[i] << PORT_SIZE * portIdx);
                }
                else if (digitalReadSize == 16)
                {
                    NIDAQ::uInt16 di_data_16_[CHANNEL_BUFFER_SIZE];
                    DAQmxErrChk (NIDAQ::DAQmxReadDigitalU16 (
                        taskHandleDI,
                        numSampsPerChan,
                        timeout,
                        DAQmx_Val_GroupByScanNumber,
                        di_data_16_,
                        numSampsPerChan,
                        &di_read,
                        NULL));
                    for (int i = 0; i < numSampsPerChan; i++)
                        eventCodes[i] |= (di_data_16_[i] << PORT_SIZE * portIdx);
                }
                else if (digitalReadSize == 8)
                {
                    NIDAQ::uInt8 di_data_8_[CHANNEL_BUFFER_SIZE];
                    DAQmxErrChk (NIDAQ::DAQmxReadDigitalU8 (
                        taskHandleDI,
                        numSampsPerChan,
                        timeout,
                        DAQmx_Val_GroupByScanNumber,
                        di_data_8_,
                        numSampsPerChan,
                        &di_read,
                        NULL));
                    for (int i = 0; i < numSampsPerChan; i++)
                        eventCodes[i] |= (di_data_8_[i] << PORT_SIZE * portIdx);
                }

                portIdx++;
            }
        }

        /*
		std::chrono::milliseconds last_time;
		std::chrono::milliseconds t = std::chrono::duration_cast< std::chrono::milliseconds >(
			std::chrono::system_clock::now().time_since_epoch());
		long long t_ms = t.count()*std::chrono::milliseconds::period::num / std::chrono::milliseconds::period::den;
		if (ai_read>0) {
			printf("Read @ %i | ", t_ms);
			printf("Acquired %d AI samples. Total %d | ", (int)ai_read, (int)(totalAIRead += ai_read));
			printf("Acquired %d DI samples. Total %d\n", (int)di_read, (int)(totalDIRead += di_read));
			fflush(stdout);
		}
		*/

        float aiSamples[MAX_NUM_AI_CHANNELS];
        int count = 0;
        for (int i = 0; i < arraySizeInSamps; i++)
        {
            int channel = i % numActiveAnalogInputs;

            aiSamples[channel] = ai[channel]->isEnabled() ? ai_data[i] : 0;

            if (i % numActiveAnalogInputs == 0)
            {
                ai_timestamp++;
                if (getActiveDigitalLines() > 0)
                {
                    eventCode = eventCodes[count++] & getActiveDigitalLines();

                    if (! digitalLineMap.count (eventCode))
                    {
                        digitalLineMap[eventCode] = 1;
                    }
                }
                aiBuffer->addToBuffer (aiSamples, &ai_timestamp, &ts, &eventCode, 1);
            }
        }

        // fflush(stdout);
    }

    /*********************************************/
    // DAQmx Stop Code
    /*********************************************/

    if (numActiveAnalogInputs)
        NIDAQ::DAQmxStopTask (taskHandleAI);
    if (numActiveAnalogInputs)
        NIDAQ::DAQmxClearTask (taskHandleAI);
    if (numActiveDigitalInputs)
    {
        for (auto& taskHandleDI : taskHandlesDI)
        {
            NIDAQ::DAQmxStopTask (taskHandleDI);
            NIDAQ::DAQmxClearTask (taskHandleDI);
        }
    }

    return;

Error:

    if (DAQmxFailed (error))
        NIDAQ::DAQmxGetExtendedErrorInfo (errBuff, ERR_BUFF_SIZE);

    if (taskHandleAI != 0)
    {
        // DAQmx Stop Code
        NIDAQ::DAQmxStopTask (taskHandleAI);
        NIDAQ::DAQmxClearTask (taskHandleAI);
    }

    if (taskHandlesDI.size() > 0)
    {
        for (auto& taskHandleDI : taskHandlesDI)
        {
            // DAQmx Stop Code
            NIDAQ::DAQmxStopTask (taskHandleDI);
            NIDAQ::DAQmxClearTask (taskHandleDI);
        }
    }
    if (DAQmxFailed (error))
        LOGE ("DAQmx Error: ", errBuff);
    fflush (stdout);

    return;
}