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

EditorBackground::EditorBackground(int nAI, int nDI) : nAI(nAI), nDI(nDI) {}

void EditorBackground::paint(Graphics& g)
{

	/* Draw AI channels */

	/*
	[ AI0 o |FS| ] [ AI4 o |FS| ]
	[ AI1 o |FS| ] [ AI5 o |FS| ]
	[ AI2 o |FS| ] [ AI6 o |FS| ]
	[ AI3 o |FS| ] [ AI7 o |FS| ]
	*/

	int maxChannelsPerColumn = 4;
	int aiChannelsPerColumn = nAI < maxChannelsPerColumn ? nAI : maxChannelsPerColumn;
	int diChannelsPerColumn = nDI < maxChannelsPerColumn ? nDI : maxChannelsPerColumn;

	float aiChanOffsetX = 15;
	float aiChanOffsetY = 12;
	float aiChanWidth = 70;
	float aiChanHeight = 22;
	float paddingX = 1.07;
	float paddingY = 1.18;

	for (int i = 0; i < nAI; i++)
	{

		int colIndex = i / aiChannelsPerColumn;
		int rowIndex = i % aiChannelsPerColumn;

		g.setColour(Colours::lightgrey);
		g.drawRoundedRectangle(
			aiChanOffsetX + paddingX * colIndex * aiChanWidth,
			aiChanOffsetY + paddingY * rowIndex * aiChanHeight,
			aiChanWidth, aiChanHeight, 4, 3);

		g.setColour(Colours::darkgrey);
		g.drawRoundedRectangle(
			aiChanOffsetX + paddingX * colIndex * aiChanWidth,
			aiChanOffsetY + paddingY * rowIndex * aiChanHeight,
			aiChanWidth, aiChanHeight, 4, 1);

		g.drawRoundedRectangle(
			aiChanOffsetX + colIndex * paddingX * aiChanWidth + aiChanWidth - aiChanWidth / 3,
			16 + paddingY * aiChanHeight * rowIndex,
			aiChanWidth / 3 - 4, 14, 1, 0.4);

		g.setFont(10);
		g.drawText(
			String("AI") + String(i),
			5 + aiChanOffsetX + paddingX * colIndex * aiChanWidth,
			7 + aiChanOffsetY + paddingY * rowIndex * aiChanHeight,
			20, 10, Justification::centredLeft);

		g.drawText(String("FS"),
			51 + aiChanOffsetX + paddingX * colIndex * aiChanWidth,
			7 + aiChanOffsetY + paddingY * rowIndex * aiChanHeight,
			20, 10, Justification::centredLeft);

	}

	/* Draw DI lines */

	/*
	[ DI0 o ] [ DI4 o ]
	[ DI1 o ] [ DI5 o ]
	[ DI2 o ] [ DI6 o ]
	[ DI3 o ] [ DI7 o ]
	*/

	float diChanOffsetX = aiChanOffsetX + nAI / aiChannelsPerColumn * paddingX * aiChanWidth;
	float diChanOffsetY = aiChanOffsetY;
	float diChanWidth = 42;
	float diChanHeight = 22;

	for (int i = 0; i < nDI; i++)
	{

		int colIndex = i / diChannelsPerColumn;
		int rowIndex = i % diChannelsPerColumn;

		g.setColour(Colours::lightgrey);
		g.drawRoundedRectangle(
			diChanOffsetX + paddingX * colIndex * diChanWidth,
			diChanOffsetY + paddingY * rowIndex * diChanHeight,
			diChanWidth, diChanHeight, 4, 3);

		g.setColour(Colours::darkgrey);
		g.drawRoundedRectangle(
			diChanOffsetX + paddingX * colIndex * diChanWidth,
			diChanOffsetY + paddingY * rowIndex * diChanHeight,
			diChanWidth, diChanHeight, 4, 1);

		g.setFont(10);
		g.drawText(
			"DI" + String(i),
			5 + diChanOffsetX + paddingX * colIndex * diChanWidth,
			7 + diChanOffsetY + paddingY * rowIndex * diChanHeight,
			20, 10, Justification::centredLeft);

	}

	//FIFO monitor label
	float settingsOffsetX = 20 + (nAI / aiChannelsPerColumn) * 75 + (nDI / diChannelsPerColumn) * 44;
	g.setFont(8);
	g.drawText(String("0"), settingsOffsetX + 16, 100, 50, 10, Justification::centredLeft);
	g.drawText(String("100"), settingsOffsetX + 16, 60, 50, 10, Justification::centredLeft);
	g.drawText(String("%"), settingsOffsetX + 16, 80, 50, 10, Justification::centredLeft);

	g.setColour(Colours::darkgrey);
	g.setFont(10);
	g.drawText(String("SAMPLE RATE"), settingsOffsetX, 13, 100, 10, Justification::centredLeft);
	g.drawText(String("USAGE"), settingsOffsetX, 48, 100, 10, Justification::centredLeft);

}

FifoMonitor::FifoMonitor(NIDAQThread* thread_) : thread(thread_), fillPercentage(0.0)
{
	startTimer(500); // update fill percentage every 0.5 seconds
}

void FifoMonitor::timerCallback()
{
	//TODO:
}


void FifoMonitor::setFillPercentage(float fill_)
{
	fillPercentage = fill_;

	repaint();
}

void FifoMonitor::paint(Graphics& g)
{
	g.setColour(Colours::grey);
	g.fillRoundedRectangle(0, 0, this->getWidth(), this->getHeight(), 4);
	g.setColour(Colours::lightslategrey);
	g.fillRoundedRectangle(2, 2, this->getWidth() - 4, this->getHeight() - 4, 2);

	g.setColour(Colours::yellow);
	float barHeight = (this->getHeight() - 4) * fillPercentage;
	g.fillRoundedRectangle(2, this->getHeight() - 2 - barHeight, this->getWidth() - 4, barHeight, 2);
}

AIButton::AIButton(int id_, NIDAQThread* thread_) : id(id_), thread(thread_), status(0), selected(false)
{
	connected = false;

	setRadioGroupId(979);

	startTimer(500);

}

void AIButton::setId(int id_)
{
	id = id_;
}

int AIButton::getId()
{
	return id;
}

void AIButton::setSelectedState(bool state)
{
	selected = state;
}

void AIButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{
	if (isMouseOver && connected)
		g.setColour(Colours::antiquewhite);
	else
		g.setColour(Colours::darkgrey);
	g.fillEllipse(0, 0, 15, 15);

	///g.setGradientFill(ColourGradient(Colours::lightcyan, 0, 0, Colours::lightskyblue, 10,10, true));

	if (status == 1)
	{
		if (selected)
		{
			if (isMouseOver)
				g.setColour(Colours::lightgreen);
			else
				g.setColour(Colours::lightgreen);
		}
		else {
			if (isMouseOver)
				g.setColour(Colours::green);
			else
				g.setColour(Colours::green);
		}
	}
	else if (status == 2)
	{
		if (selected)
		{
			if (isMouseOver)
				g.setColour(Colours::lightsalmon);
			else
				g.setColour(Colours::lightsalmon);
		}
		else {
			if (isMouseOver)
				g.setColour(Colours::orange);
			else
				g.setColour(Colours::orange);
		}
	}
	else {
		g.setColour(Colours::lightgrey);
	}

	g.fillEllipse(2, 2, 11, 11);
}

void AIButton::setInputStatus(int status_)
{
	status = status_;

	repaint();

}

void AIButton::timerCallback()
{

}

DIButton::DIButton(int id_, NIDAQThread* thread_) : id(id_), thread(thread_), status(0), selected(false)
{
	connected = false;

	setRadioGroupId(979);

	startTimer(500);

}

void DIButton::setId(int id_)
{
	id = id_;
}

int DIButton::getId()
{
	return id;
}

void DIButton::setSelectedState(bool state)
{
	selected = state;
}

void DIButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{
	if (isMouseOver && connected)
		g.setColour(Colours::antiquewhite);
	else
		g.setColour(Colours::darkgrey);
	g.fillRoundedRectangle(0, 0, 15, 15, 4);

	///g.setGradientFill(ColourGradient(Colours::lightcyan, 0, 0, Colours::lightskyblue, 10,10, true));

	if (status == 1)
	{
		if (selected)
		{
			if (isMouseOver)
				g.setColour(Colours::lightgreen);
			else
				g.setColour(Colours::lightgreen);
		}
		else {
			if (isMouseOver)
				g.setColour(Colours::green);
			else
				g.setColour(Colours::green);
		}
	}
	else if (status == 2)
	{
		if (selected)
		{
			if (isMouseOver)
				g.setColour(Colours::lightsalmon);
			else
				g.setColour(Colours::lightsalmon);
		}
		else {
			if (isMouseOver)
				g.setColour(Colours::orange);
			else
				g.setColour(Colours::orange);
		}
	}
	else {
		g.setColour(Colours::lightgrey);
	}

	g.fillRoundedRectangle(2, 2, 11, 11, 3);
}

void DIButton::setInputStatus(int status_)
{
	status = status_;

	repaint();

}

void DIButton::timerCallback()
{

}

BackgroundLoader::BackgroundLoader(NIDAQThread* thread, NIDAQEditor* editor)
	: Thread("NIDAQ Loader"), t(thread), e(editor)
{
}

BackgroundLoader::~BackgroundLoader()
{
}

void BackgroundLoader::run()
{
	/* This process is used to initiate processor loading in the background to prevent this plugin from blocking the main GUI*/

	/*Basic steps:*/
	t->openConnection();

	/* Let the main GUI know the plugin is done initializing */
	MessageManagerLock mml;
	CoreServices::updateSignalChain(e);
	CoreServices::sendStatusMessage("NIDAQ plugin ready for acquisition!");

}


NIDAQEditor::NIDAQEditor(GenericProcessor* parentNode, NIDAQThread* t, bool useDefaultParameterEditors)
	: GenericEditor(parentNode, useDefaultParameterEditors)
{

	thread = t;

	t->openConnection();

	int nAI = t->getNumAnalogInputs();
	int nDI = t->getNumDigitalInputs();

	int aiChannelsPerColumn = nAI < 4 ? nAI : 4;
	int diChannelsPerColumn = nDI < 4 ? nDI : 4;

	for (int i = 0; i < nAI; i++)
	{

		int colIndex = i / aiChannelsPerColumn;
		int rowIndex = i % aiChannelsPerColumn + 1;
		int x_pos = colIndex * 75 + 40;
		int y_pos = 5 + rowIndex * 26;

		AIButton* b = new AIButton(i, thread);
		b->setBounds(x_pos, y_pos, 15, 15);
		b->addListener(this);
		addAndMakeVisible(b);
		aiButtons.add(b);

		//p->setId(? );

	}

	for (int i = 0; i < nDI; i++)
	{

		int colIndex = i / diChannelsPerColumn;
		int rowIndex = i % diChannelsPerColumn + 1;
		int x_pos = (nAI / aiChannelsPerColumn) * 75 + 38 + colIndex * 44;
		int y_pos = 5 + rowIndex * 26;

		DIButton* b = new DIButton(i, thread);
		b->setBounds(x_pos, y_pos, 15, 15);
		b->addListener(this);
		addAndMakeVisible(b);
		diButtons.add(b);

		//p->setId(? );

	}

	float xOffset = 20 + (nAI / aiChannelsPerColumn) * 75 + (nDI / diChannelsPerColumn) * 44;

	sampleRateSelectBox = new ComboBox("SampleRateSelectBox");
	sampleRateSelectBox->setBounds(xOffset, 39, 64, 20);
	sampleRateSelectBox->addItem(String("30kHz"), 1);
	sampleRateSelectBox->setSelectedItemIndex(0, false);
	sampleRateSelectBox->addListener(this);
	addAndMakeVisible(sampleRateSelectBox);
	
	fifoMonitor = new FifoMonitor(thread);
	fifoMonitor->setBounds(xOffset + 2, 75, 12, 50);
	addAndMakeVisible(fifoMonitor);

	desiredWidth = 15 + 75 * (nAI / aiChannelsPerColumn) + 45 * (nDI / diChannelsPerColumn) + (nAI + nDI > 0 ? 90 : 0);

	background = new EditorBackground(nAI, nDI);
	background->setBounds(0, 15, 500, 150);
	addAndMakeVisible(background);
	background->toBack();
	background->repaint();

	uiLoader = new BackgroundLoader(t, this);
	uiLoader->startThread();

}

NIDAQEditor::~NIDAQEditor()
{

}

void NIDAQEditor::comboBoxChanged(ComboBox* comboBox)
{

	if (comboBox == sampleRateSelectBox)
	{
		//TODO:
	}
}

void NIDAQEditor::buttonEvent(Button* button)
{

	if (aiButtons.contains((AIButton*)button))
	{
		for (auto button : aiButtons)
		{
			button->setSelectedState(false);
		}
		AIButton* button = (AIButton*)button;
		button->setSelectedState(true);
		//thread->setSelectedProbe(probe->slot, probe->port);

		repaint();
	}

	if (!acquisitionIsActive)
	{

	}
}


void NIDAQEditor::saveEditorParameters(XmlElement* xml)
{
	/*
	std::cout << "Saving NI-DAQ editor." << std::endl;

	XmlElement* xmlNode = xml->createNewChildElement("NIDAQ_EDITOR");

	for (int slot = 0; slot < thread->getNumBasestations(); slot++)
	{
		String directory_name = savingDirectories[slot].getFullPathName();
		if (directory_name.length() == 2)
			directory_name += "\\\\";
		xmlNode->setAttribute("Slot" + String(slot) + "Directory", directory_name);
	}
	*/

}

void NIDAQEditor::loadEditorParameters(XmlElement* xml)
{
	/*
	forEachXmlChildElement(*xml, xmlNode)
	{
		if (xmlNode->hasTagName("NIDAQ_EDITOR"))
		{
			std::cout << "Found parameters for Neuropixels editor" << std::endl;

			for (int slot = 0; slot < thread->getNumBasestations(); slot++)
			{
				File directory = File(xmlNode->getStringAttribute("Slot" + String(slot) + "Directory"));
				std::cout << "Setting thread directory for slot " << slot << std::endl;
				thread->setDirectoryForSlot(slot, directory);
				directoryButtons[slot]->setLabel(directory.getFullPathName().substring(0, 2));
				savingDirectories.set(slot, directory);
			}
		}
	}
	*/
}
