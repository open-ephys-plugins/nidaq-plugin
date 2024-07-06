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

#include "NIDAQEditor.h"
#include "NIDAQThread.h"

EditorBackground::EditorBackground (int nAI, int nDI) : nAI (nAI), nDI (nDI) {}

void EditorBackground::paint (Graphics& g)
{
    if (nAI > 0 || nDI > 0)
    {
        /* Draw AI channels */
        int maxChannelsPerColumn = 4;
        int aiChannelsPerColumn = nAI > 0 && nAI < maxChannelsPerColumn ? nAI : maxChannelsPerColumn;
        int diChannelsPerColumn = nDI > 0 && nDI < maxChannelsPerColumn ? nDI : maxChannelsPerColumn;

        float aiChanOffsetX = 15; //pixels
        float aiChanOffsetY = 12; //pixels
        float aiChanWidth = 80; //pixels
        float aiChanHeight = 22; //pixels TODO: normalize
        float paddingX = 1.07;
        float paddingY = 1.18;

        for (int i = 0; i < nAI; i++)
        {
            int colIndex = i / aiChannelsPerColumn;
            int rowIndex = i % aiChannelsPerColumn;

            g.setColour (findColour (ThemeColours::outline));
            g.drawRoundedRectangle (
                aiChanOffsetX + paddingX * colIndex * aiChanWidth,
                aiChanOffsetY + paddingY * rowIndex * aiChanHeight,
                aiChanWidth,
                aiChanHeight,
                4,
                1);

            g.setColour (findColour (ThemeColours::outline));

            g.drawRoundedRectangle (
                aiChanOffsetX + paddingX * colIndex * aiChanWidth,
                aiChanOffsetY + paddingY * rowIndex * aiChanHeight,
                aiChanWidth,
                aiChanHeight,
                4,
                1);

            /*
			g.drawRoundedRectangle(
			aiChanOffsetX + colIndex * paddingX * aiChanWidth + aiChanWidth - aiChanWidth / 3,
			16 + paddingY * aiChanHeight * rowIndex,
			aiChanWidth / 3 - 4, 14, 1, 0.4);
			*/

            g.setColour (findColour (ThemeColours::defaultText));
            g.setFont (10);
            g.drawText (
                String ("AI") + String (i),
                5 + aiChanOffsetX + paddingX * colIndex * aiChanWidth,
                7 + aiChanOffsetY + paddingY * rowIndex * aiChanHeight,
                20,
                10,
                Justification::centredLeft);

            /*
			g.drawText(String("FS"),
			51 + aiChanOffsetX + paddingX * colIndex * aiChanWidth,
			7 + aiChanOffsetY + paddingY * rowIndex * aiChanHeight,
			20, 10, Justification::centredLeft);
			*/
        }

        /* Draw DI lines */
        float diChanOffsetX = aiChanOffsetX + ((nAI % maxChannelsPerColumn == 0 ? 0 : 1) + nAI / aiChannelsPerColumn) * paddingX * aiChanWidth;
        float diChanOffsetY = aiChanOffsetY;
        float diChanWidth = 42;
        float diChanHeight = 22;

        for (int i = 0; i < nDI; i++)
        {
            int colIndex = i / diChannelsPerColumn;
            int rowIndex = i % diChannelsPerColumn;

            g.setColour (findColour (ThemeColours::outline));
            g.drawRoundedRectangle (
                diChanOffsetX + paddingX * colIndex * diChanWidth,
                diChanOffsetY + paddingY * rowIndex * diChanHeight,
                diChanWidth,
                diChanHeight,
                4,
                1);

            g.setColour (findColour (ThemeColours::outline));
            g.drawRoundedRectangle (
                diChanOffsetX + paddingX * colIndex * diChanWidth,
                diChanOffsetY + paddingY * rowIndex * diChanHeight,
                diChanWidth,
                diChanHeight,
                4,
                1);

            g.setColour (findColour (ThemeColours::defaultText));
            g.setFont (10);
            if (i >= 10)
                g.setFont (8);
            g.drawText (
                "DI" + String (i),
                5 + diChanOffsetX + paddingX * colIndex * diChanWidth,
                7 + diChanOffsetY + paddingY * rowIndex * diChanHeight,
                20,
                10,
                Justification::centredLeft);
        }

        //FIFO monitor label
        float settingsOffsetX = diChanOffsetX + ((nDI % maxChannelsPerColumn == 0 ? 0 : 1) + nDI / diChannelsPerColumn) * paddingX * diChanWidth + 5;
        g.setColour (findColour (ThemeColours::defaultText));
        g.setFont (10);

        g.drawText (String ("DEVICE"), settingsOffsetX, 13, 100, 10, Justification::centredLeft);
        g.drawText (String ("SAMPLE RATE"), settingsOffsetX, 47, 100, 10, Justification::centredLeft);
        g.drawText (String ("AI VOLTAGE RANGE"), settingsOffsetX, 80, 100, 10, Justification::centredLeft);

        /*
		g.drawText(String("USAGE"), settingsOffsetX, 77, 100, 10, Justification::centredLeft);
		g.setFont(8);
		g.drawText(String("0"), settingsOffsetX, 100, 50, 10, Justification::centredLeft);
		g.drawText(String("100"), settingsOffsetX + 65, 100, 50, 10, Justification::centredLeft);
		g.drawText(String("%"), settingsOffsetX + 33, 100, 50, 10, Justification::centredLeft);
		*/
    }
}

FifoMonitor::FifoMonitor (NIDAQThread* thread_) : thread (thread_), fillPercentage (0.0)
{
    startTimer (500); // update fill percentage every 0.5 seconds
}

void FifoMonitor::timerCallback()
{
    //TODO:
}

void FifoMonitor::setFillPercentage (float fill_)
{
    fillPercentage = fill_;

    repaint();
}

void FifoMonitor::paint (Graphics& g)
{
    g.setColour (Colours::grey);
    g.fillRoundedRectangle (0, 0, this->getWidth(), this->getHeight(), 4);
    g.setColour (Colours::lightslategrey);
    g.fillRoundedRectangle (2, 2, this->getWidth() - 4, this->getHeight() - 4, 2);

    g.setColour (Colours::yellow);
    float barHeight = (this->getHeight() - 4) * fillPercentage;
    g.fillRoundedRectangle (2, this->getHeight() - 2 - barHeight, this->getWidth() - 4, barHeight, 2);
}

AIButton::AIButton (int id_, NIDAQThread* thread_) : id (id_), thread (thread_), enabled (true)
{
    startTimer (500);
}

void AIButton::setId (int id_)
{
    id = id_;
}

int AIButton::getId()
{
    return id;
}

void AIButton::setEnabled (bool enable)
{
    enabled = enable;
    thread->mNIDAQ->ai[id]->setEnabled (enabled);
}

void AIButton::paintButton (Graphics& g, bool isMouseOver, bool isButtonDown)
{
    if (isMouseOver && enabled)
        g.setColour (Colours::antiquewhite);
    else
        g.setColour (Colours::darkgrey);
    g.fillEllipse (0, 0, 15, 15);

    if (enabled && thread->inputAvailable)
    {
        if (isMouseOver)
            g.setColour (Colours::lightgreen);
        else
            g.setColour (Colours::forestgreen);
    }
    else
    {
        if (isMouseOver)
            g.setColour (Colours::lightgrey);
        else
            g.setColour (Colours::lightgrey);
    }
    g.fillEllipse (3, 3, 9, 9);
}

void AIButton::timerCallback()
{
}

DIButton::DIButton (int id_, NIDAQThread* thread_) : id (id_), thread (thread_), enabled (true)
{
    startTimer (500);
}

void DIButton::setId (int id_)
{
    id = id_;
}

int DIButton::getId()
{
    return id;
}

void DIButton::setEnabled (bool enable)
{
    enabled = enable;
    thread->mNIDAQ->di[id]->setEnabled (enabled);
}

void DIButton::paintButton (Graphics& g, bool isMouseOver, bool isButtonDown)
{
    if (isMouseOver && enabled)
        g.setColour (Colours::antiquewhite);
    else
        g.setColour (Colours::darkgrey);
    g.fillRoundedRectangle (0, 0, 15, 15, 2);

    if (enabled && thread->inputAvailable)
    {
        if (isMouseOver)
            g.setColour (Colours::lightgreen);
        else
            g.setColour (Colours::forestgreen);
    }
    else
    {
        if (isMouseOver)
            g.setColour (Colours::lightgrey);
        else
            g.setColour (Colours::lightgrey);
    }
    g.fillRoundedRectangle (3, 3, 9, 9, 2);
}

void DIButton::timerCallback()
{
}

SourceTypeButton::SourceTypeButton (int id_, NIDAQThread* thread_, SOURCE_TYPE source) : id (id_), thread (thread_)
{
    update (source);
}

void SourceTypeButton::setId (int id_)
{
    id = id_;
}

int SourceTypeButton::getId()
{
    return id;
}

void SourceTypeButton::update (SOURCE_TYPE sourceType)
{
    switch (sourceType)
    {
        case SOURCE_TYPE::RSE:
            setButtonText ("RSE");
            return;
        case SOURCE_TYPE::NRSE:
            setButtonText ("NRSE");
            return;
        case SOURCE_TYPE::DIFF:
            setButtonText ("DIFF");
            return;
        case SOURCE_TYPE::PSEUDO_DIFF:
            setButtonText ("PDIF");
            return;
        default:
            break;
    }

    changeWidthToFitText();
}

void SourceTypeButton::timerCallback()
{
}

BackgroundLoader::BackgroundLoader (NIDAQThread* thread, NIDAQEditor* editor)
    : Thread ("NIDAQ Loader"), t (thread), e (editor)
{
}

BackgroundLoader::~BackgroundLoader()
{
}

void BackgroundLoader::run()
{
    /* This process is used to initiate processor loading in the background to prevent this plugin from blocking the main GUI*/

    /* Let the main GUI know the plugin is done initializing */
    MessageManagerLock mml;
    CoreServices::updateSignalChain (e);
    CoreServices::sendStatusMessage ("NIDAQ plugin ready for acquisition!");
}

NIDAQEditor::NIDAQEditor (GenericProcessor* parentNode, NIDAQThread* t)
    : GenericEditor (parentNode), thread (t), currentConfigWindow (nullptr)
{
    draw();
}

void NIDAQEditor::draw()
{
    NIDAQThread* t = thread;

    int nAI;
    int nDI;

    if (t->getDeviceName() == "Simulated Device")
    {
        nAI = 8;
        nDI = 8;
    }
    else
    {
        nAI = t->getNumActiveAnalogInputs();
        nDI = t->getNumActiveDigitalInputs();
    }

    int maxChannelsPerColumn = 4;
    int aiChannelsPerColumn = nAI > 0 && nAI < maxChannelsPerColumn ? nAI : maxChannelsPerColumn;
    int diChannelsPerColumn = nDI > 0 && nDI < maxChannelsPerColumn ? nDI : maxChannelsPerColumn;

    aiButtons.clear();
    sourceTypeButtons.clear();

    int xOffset = 0;

    // Draw analog inputs
    for (int i = 0; i < nAI; i++)
    {
        int colIndex = i / aiChannelsPerColumn;
        int rowIndex = i % aiChannelsPerColumn + 1;
        xOffset = colIndex * 86 + 40;
        int y_pos = 4 + rowIndex * 26;

        AIButton* a = new AIButton (i, thread);
        a->setBounds (xOffset, y_pos, 15, 15);
        a->addListener (this);
        addAndMakeVisible (a);
        aiButtons.add (a);

        SOURCE_TYPE sourceType = SOURCE_TYPE::RSE;
        if (thread->foundInputSource())
            sourceType = thread->getSourceTypeForInput (i);

        SourceTypeButton* b = new SourceTypeButton (i, thread, sourceType);
        b->setBounds (xOffset + 17, y_pos - 1, 32, 17);
        b->changeWidthToFitText();
        b->addListener (this);
        b->setEnabled (thread->foundInputSource());
        addAndMakeVisible (b);
        sourceTypeButtons.add (b);
    }

    diButtons.clear();

    // Draw digital inputs
    for (int i = 0; i < nDI; i++)
    {
        int colIndex = i / diChannelsPerColumn;
        int rowIndex = i % diChannelsPerColumn + 1;
        xOffset = ((nAI % maxChannelsPerColumn == 0 ? 0 : 1) + nAI / aiChannelsPerColumn) * 86 + 38 + colIndex * 45;
        int y_pos = 5 + rowIndex * 26;

        DIButton* b = new DIButton (i, thread);
        b->setBounds (xOffset, y_pos, 15, 15);
        b->addListener (this);
        //b->setEnabled(thread->foundInputSource());
        addAndMakeVisible (b);
        diButtons.add (b);
    }

    xOffset = xOffset + 25 + 30 * (nDI == 0);

    deviceSelectBox = new ComboBox ("DeviceSelectBox");
    deviceSelectBox->setBounds (xOffset, 39, 85, 20);
    Array<NIDAQDevice*> devices = t->getDevices();
    for (int i = 0; i < t->getNumAvailableDevices(); i++)
    {
        deviceSelectBox->addItem (devices[i]->productName, i + 1);
    }
    deviceSelectBox->setSelectedItemIndex (t->getDeviceIndex(), false);
    deviceSelectBox->addListener (this);
    addAndMakeVisible (deviceSelectBox);

    if (t->getNumAvailableDevices() == 1) // disable device selection if only one device is available
        deviceSelectBox->setEnabled (false);

    sampleRateSelectBox = new ComboBox ("SampleRateSelectBox");
    sampleRateSelectBox->setBounds (xOffset, 72, 85, 20);
    Array<NIDAQ::float64> sampleRates = t->getSampleRates();
    for (int i = 0; i < sampleRates.size(); i++)
    {
        sampleRateSelectBox->addItem (String (sampleRates[i]) + " S/s", i + 1);
    }
    sampleRateSelectBox->setSelectedItemIndex (t->getSampleRateIndex(), false);
    sampleRateSelectBox->addListener (this);
    addAndMakeVisible (sampleRateSelectBox);

    voltageRangeSelectBox = new ComboBox ("VoltageRangeSelectBox");
    voltageRangeSelectBox->setBounds (xOffset, 105, 85, 20);
    Array<SettingsRange> voltageRanges = t->getVoltageRanges();
    for (int i = 0; i < voltageRanges.size(); i++)
    {
        //String rangeString = String(voltageRanges[i].min) + " - " + String(voltageRanges[i].max) + " V";
        String rangeString = String (voltageRanges[i].min) + " to " + String (voltageRanges[i].max) + " V";
        voltageRangeSelectBox->addItem (rangeString, i + 1);
    }
    voltageRangeSelectBox->setSelectedItemIndex (t->getVoltageRangeIndex(), false);
    voltageRangeSelectBox->addListener (this);
    addAndMakeVisible (voltageRangeSelectBox);

    fifoMonitor = new FifoMonitor (thread);
    fifoMonitor->setBounds (xOffset + 2, 105, 70, 12);
    //addAndMakeVisible(fifoMonitor);

    configureDeviceButton = new UtilityButton ("...");
    configureDeviceButton->setFont (FontOptions ((12.0f)));
    configureDeviceButton->setBounds (xOffset + 60, 25, 24, 12);
    configureDeviceButton->addListener (this);
    configureDeviceButton->setAlpha (0.5f);
    addAndMakeVisible (configureDeviceButton);

    desiredWidth = xOffset + 100;

    background = new EditorBackground (nAI, nDI);
    background->setBounds (0, 15, 1000, 150);
    addAndMakeVisible (background);
    background->toBack();
    background->repaint();

    //TODO: Why this line casuses crash in editor->update in v6?
    //setDisplayName("NIDAQmx-(" + t->getProductName() + ")");
}

void NIDAQEditor::update (int numAnalog, int numDigital, int digitalReadSize)
{
    if (numAnalog != thread->getNumActiveAnalogInputs())
    {
        thread->setNumActiveAnalogChannels (numAnalog);
        thread->updateAnalogChannels();

        CoreServices::updateSignalChain (this);

        ((CallOutBox*) currentConfigWindow->getParentComponent())->dismiss();
    }

    if (numDigital != thread->getNumActiveDigitalInputs())
    {
        thread->setNumActiveDigitalChannels (numDigital);
        thread->updateDigitalChannels();

        ((CallOutBox*) currentConfigWindow->getParentComponent())->dismiss();
    }

    if (digitalReadSize != thread->getDigitalReadSize())
    {
        thread->setDigitalReadSize (digitalReadSize);
    }

    draw();
}

NIDAQEditor::~NIDAQEditor()
{
}

void NIDAQEditor::startAcquisition()
{
    //Disable all source type buttons
    for (auto& button : sourceTypeButtons)
        button->setEnabled (false);

    //Disable all combo boxes
    deviceSelectBox->setEnabled (false);
    sampleRateSelectBox->setEnabled (false);
    voltageRangeSelectBox->setEnabled (false);

    //Disable device config button
    configureDeviceButton->setEnabled (false);
}

void NIDAQEditor::stopAcquisition()
{
    //Enable all source type buttons
    for (auto& button : sourceTypeButtons)
        button->setEnabled (true);

    //Enable all combo boxes
    deviceSelectBox->setEnabled (true);
    sampleRateSelectBox->setEnabled (true);
    voltageRangeSelectBox->setEnabled (true);

    //Enable device config button
    configureDeviceButton->setEnabled (true);
}

/** Respond to button presses */
void NIDAQEditor::buttonClicked (Button* button)
{
    // Ignore any button presses if there is no input source
    if (thread->foundInputSource())
        buttonEvent (button);
}

void NIDAQEditor::comboBoxChanged (ComboBox* comboBox)
{
    if (comboBox == deviceSelectBox)
    {
        if (! thread->isThreadRunning())
        {
            if (comboBox->getSelectedId() - 1 != thread->getDeviceIndex())
            {
                thread->swapConnection (thread->getDevices()[comboBox->getSelectedId() - 1]->getName());
                draw();
            }
        }
        else
        {
            comboBox->setSelectedItemIndex (thread->getDeviceIndex());
        }
    }
    else if (comboBox == sampleRateSelectBox)
    {
        if (! thread->isThreadRunning())
        {
            thread->setSampleRate (comboBox->getSelectedId() - 1);
            CoreServices::updateSignalChain (this);
        }
        else
        {
            comboBox->setSelectedItemIndex (thread->getSampleRateIndex());
        }
    }
    else // (comboBox == voltageRangeSelectBox)
    {
        if (! thread->isThreadRunning())
        {
            thread->setVoltageRange (comboBox->getSelectedId() - 1);
            CoreServices::updateSignalChain (this);
        }
        else
        {
            comboBox->setSelectedItemIndex (thread->getVoltageRangeIndex());
        }
    }
}

void NIDAQEditor::buttonEvent (Button* button)
{
    if (aiButtons.contains ((AIButton*) button))
    {
        ((AIButton*) button)->setEnabled (thread->toggleAIChannel (((AIButton*) button)->getId()));
        repaint();
    }
    else if (diButtons.contains ((DIButton*) button))
    {
        ((DIButton*) button)->setEnabled (thread->toggleDIChannel (((DIButton*) button)->getId()));
        repaint();
    }
    else if (sourceTypeButtons.contains ((SourceTypeButton*) button))
    {
        int currentButtonId = ((SourceTypeButton*) button)->getId();
        thread->toggleSourceType (currentButtonId);
        SOURCE_TYPE next = thread->getSourceTypeForInput (currentButtonId);
        ((SourceTypeButton*) button)->update (next);
        repaint();
    }
    else if (button == configureDeviceButton)
    {
        if (! thread->isThreadRunning())
        {
            currentConfigWindow = new PopupConfigurationWindow (this);

            CallOutBox& myBox = CallOutBox::launchAsynchronously (std::unique_ptr<Component> (currentConfigWindow),
                                                                  button->getScreenBounds(),
                                                                  nullptr);

            myBox.setDismissalMouseClicksAreAlwaysConsumed (true);

            return;
        }
    }
}

void NIDAQEditor::saveCustomParametersToXml (XmlElement* xml)
{
    xml->setAttribute ("deviceName", thread->getDeviceName());
    xml->setAttribute ("sampleRate", thread->getSampleRate());
    xml->setAttribute ("voltageRange", thread->getVoltageRangeIndex());

    xml->setAttribute ("numAnalog", thread->getNumActiveAnalogInputs());
    xml->setAttribute ("numDigital", thread->getNumActiveDigitalInputs());
    xml->setAttribute ("digitalReadSize", thread->getDigitalReadSize());

    String digitalPortStates = "";
    for (int i = 0; i < thread->getNumPorts(); i++)
        digitalPortStates += thread->getPortState (i) ? "1" : "0";
    xml->setAttribute ("digitalPortStates", digitalPortStates);
}

void NIDAQEditor::loadCustomParametersFromXml (XmlElement* xml)
{
    String deviceToLoad = xml->getStringAttribute ("deviceName", "NIDAQmx");

    // Load device
    if (! deviceToLoad.equalsIgnoreCase ("NIDAQmx"))
    {
        int deviceIdx = thread->swapConnection (deviceToLoad);
        if (deviceIdx >= 0)
        {
            thread->setDeviceIndex (deviceIdx);
            deviceSelectBox->setSelectedItemIndex (thread->getDeviceIndex(), false);
            draw();
        }
    }

    float sampleRate = xml->getStringAttribute ("sampleRate", "0.0").getFloatValue();

    // Load sample rate
    if (sampleRate > 0.0f)
    {
        int idx = 0;
        for (auto& sr : thread->getSampleRates())
        {
            if (sr == sampleRate)
            {
                LOGD ("Setting saved sample rate: " + String (sampleRate) + " (" + String (idx) + ")");
                thread->setSampleRate (idx);
                sampleRateSelectBox->setSelectedItemIndex (thread->getSampleRateIndex(), false);
                break;
            }
            idx++;
        }
    }

    // Load voltage range
    int voltageRangeIndex = xml->getStringAttribute ("voltageRange", "-1").getIntValue();

    if (voltageRangeIndex >= 0)
    {
        thread->setVoltageRange (voltageRangeIndex);
        voltageRangeSelectBox->setSelectedItemIndex (thread->getVoltageRangeIndex(), false);
    }

    // Load number of active analog channels
    int numAnalog = xml->getStringAttribute ("numAnalog", "0").getIntValue();

    if (numAnalog >= 0)
    {
        thread->setNumActiveAnalogChannels (numAnalog);
        thread->updateAnalogChannels();
    }

    // Load number of active digital channels
    int numDigital = xml->getStringAttribute ("numDigital", "0").getIntValue();

    if (numDigital >= 0)
    {
        thread->setNumActiveDigitalChannels (numDigital);
        thread->updateDigitalChannels();
    }

    // Load digital read size
    int digitalReadSize = xml->getStringAttribute ("digitalReadSize", "0").getIntValue();

    if (digitalReadSize >= 0)
    {
        thread->setDigitalReadSize (digitalReadSize);
    }

    String digitalPortStates = xml->getStringAttribute ("digitalPortStates", "000");

    for (int i = 0; i < digitalPortStates.length(); i++)
        thread->setPortState (i, digitalPortStates[i] == '1');

    draw();
}

PopupConfigurationWindow::PopupConfigurationWindow (NIDAQEditor* editor_)
    : editor (editor_)
{
    //tableHeader.reset(new TableHeaderComponent());

    analogLabel = new Label ("Analog", "Analog Inputs: ");
    analogLabel->setFont (Font (16.0f, Font::bold));
    analogLabel->setColour (Label::textColourId, Colours::white);
    analogLabel->setBounds (2, 8, 110, 20);
    addAndMakeVisible (analogLabel);

    int activeAnalogCount = editor->getNumActiveAnalogInputs();
    analogChannelCountSelect = new ComboBox ("Analog Count Selector");
    for (int i = 4; i <= editor->getTotalAvailableAnalogInputs(); i += 4)
    {
        analogChannelCountSelect->addItem (String (i), i / 4);
        if (i == activeAnalogCount)
            analogChannelCountSelect->setSelectedId (i / 4, dontSendNotification);
    }
    analogChannelCountSelect->setBounds (115, 8, 60, 20);
    analogChannelCountSelect->addListener (this);
    addAndMakeVisible (analogChannelCountSelect);

    digitalLabel = new Label ("Digital", "Digital Inputs: ");
    digitalLabel->setColour (Label::textColourId, Colours::white);
    digitalLabel->setBounds (2, 33, 110, 20);
    addAndMakeVisible (digitalLabel);

    int activeDigitalCount = editor->getNumActiveDigitalInputs();
    digitalChannelCountSelect = new ComboBox ("Digital Count Selector");
    for (int i = 0; i <= editor->getTotalAvailableDigitalInputs(); i += 4)
    {
        digitalChannelCountSelect->addItem (String (i), i / 4 + 1);
        if (i == activeDigitalCount)
            digitalChannelCountSelect->setSelectedId (i / 4 + 1, dontSendNotification);
    }
    digitalChannelCountSelect->setBounds (115, 33, 60, 20);
    digitalChannelCountSelect->addListener (this);
    addAndMakeVisible (digitalChannelCountSelect);

    digitalReadLabel = new Label ("Digital Read", "Digital Read: ");
    digitalReadLabel->setColour (Label::textColourId, Colours::white);
    digitalReadLabel->setBounds (2, 58, 110, 20);
    addAndMakeVisible (digitalReadLabel);

    digitalReadSelect = new ComboBox ("Digital Read Selector");
    Array<int> digitalReadOptions = { 8, 16, 32 };
    for (int i = 0; i < digitalReadOptions.size(); i++)
    {
        digitalReadSelect->addItem (String (digitalReadOptions[i]) + " bits", i + 1);
        if (digitalReadOptions[i] == editor->getDigitalReadSize())
            digitalReadSelect->setSelectedId (i + 1, dontSendNotification);
    }
    digitalReadSelect->setBounds (115, 58, 60, 20);
    digitalReadSelect->addListener (this);
    addAndMakeVisible (digitalReadSelect);

    for (int i = 0; i < editor->getNumPorts(); i++)
    {
        ToggleButton* button = new ToggleButton ("P" + String (i));
        button->setBounds (i * 60 + 5, 85, 58, 20);
        button->addListener (this);
        button->setToggleState (editor->getPortState (i), juce::dontSendNotification);
        addAndMakeVisible (button);
        digitalPortButtons.add (button);
    }

    setSize (180, 110);
}

void PopupConfigurationWindow::comboBoxChanged (ComboBox* comboBox)
{
    int numAnalogInputs = int (analogChannelCountSelect->getItemText (analogChannelCountSelect->getSelectedId() - 1).getFloatValue());
    int numDigitalInputs = int (digitalChannelCountSelect->getItemText (digitalChannelCountSelect->getSelectedId() - 1).getFloatValue());
    int digitalRead = int (digitalReadSelect->getItemText (digitalReadSelect->getSelectedId() - 1).getFloatValue());

    editor->update (numAnalogInputs, numDigitalInputs, digitalRead);
}

void PopupConfigurationWindow::paint (juce::Graphics& g)
{
    // Set the background color of toggle buttons based on their state
    for (int i = 0; i < digitalPortButtons.size(); ++i)
    {
        ToggleButton* button = digitalPortButtons[i];
        g.setColour (button->getToggleState() ? juce::Colours::green : juce::Colours::red);
        g.fillRect (button->getBounds());
    }
}

void PopupConfigurationWindow::buttonClicked (juce::Button* button)
{
    int portIdx = button->getName().getLastCharacter() - '0';
    editor->setPortState (portIdx, button->getToggleState());
    repaint();
}