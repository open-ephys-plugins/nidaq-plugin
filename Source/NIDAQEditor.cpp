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

	if (nAI > 0 || nDI > 0)
	{

		/* Draw AI channels */
		int maxChannelsPerColumn = 4;
		int aiChannelsPerColumn = nAI > 0 && nAI < maxChannelsPerColumn ? nAI : maxChannelsPerColumn;
		int diChannelsPerColumn = nDI > 0 && nDI < maxChannelsPerColumn ? nDI : maxChannelsPerColumn;

		float aiChanOffsetX = 15; //pixels
		float aiChanOffsetY = 12; //pixels
		float aiChanWidth = 70;   //pixels
		float aiChanHeight = 22;  //pixels TODO: normalize
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

			/*
			g.drawRoundedRectangle(
			aiChanOffsetX + colIndex * paddingX * aiChanWidth + aiChanWidth - aiChanWidth / 3,
			16 + paddingY * aiChanHeight * rowIndex,
			aiChanWidth / 3 - 4, 14, 1, 0.4);
			*/

			g.setFont(10);
			g.drawText(
				String("AI") + String(i),
				5 + aiChanOffsetX + paddingX * colIndex * aiChanWidth,
				7 + aiChanOffsetY + paddingY * rowIndex * aiChanHeight,
				20, 10, Justification::centredLeft);

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
			if (i >= 10)
				g.setFont(8);
			g.drawText(
				"DI" + String(i),
				5 + diChanOffsetX + paddingX * colIndex * diChanWidth,
				7 + diChanOffsetY + paddingY * rowIndex * diChanHeight,
				20, 10, Justification::centredLeft);

		}

		//FIFO monitor label
		float settingsOffsetX = diChanOffsetX + ((nDI % maxChannelsPerColumn == 0 ? 0 : 1) + nDI / diChannelsPerColumn) * paddingX * diChanWidth + 5;
		g.setColour(Colours::darkgrey);
		g.setFont(10);
		g.drawText(String("SAMPLE RATE"), settingsOffsetX, 13, 100, 10, Justification::centredLeft);
		g.drawText(String("AI VOLTAGE RANGE"), settingsOffsetX, 45, 100, 10, Justification::centredLeft);

		/*
		g.drawText(String("USAGE"), settingsOffsetX, 77, 100, 10, Justification::centredLeft);
		g.setFont(8);
		g.drawText(String("0"), settingsOffsetX, 100, 50, 10, Justification::centredLeft);
		g.drawText(String("100"), settingsOffsetX + 65, 100, 50, 10, Justification::centredLeft);
		g.drawText(String("%"), settingsOffsetX + 33, 100, 50, 10, Justification::centredLeft);
		*/


	}

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

AIButton::AIButton(int id_, NIDAQThread* thread_) : id(id_), thread(thread_), enabled(true)
{
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

void AIButton::setEnabled(bool enable)
{
	enabled = enable;
	thread->ai[id].setEnabled(enabled);
}

void AIButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{
	if (isMouseOver && enabled)
		g.setColour(Colours::antiquewhite);
	else
		g.setColour(Colours::darkgrey);
	g.fillEllipse(0, 0, 15, 15);

	if (enabled)
	{
		if (isMouseOver)
			g.setColour(Colours::lightgreen);
		else
			g.setColour(Colours::forestgreen);
	}
	else
	{
		if (isMouseOver)
			g.setColour(Colours::lightgrey);
		else
			g.setColour(Colours::lightgrey);
	}
	g.fillEllipse(3, 3, 9, 9);
}

void AIButton::timerCallback()
{

}

DIButton::DIButton(int id_, NIDAQThread* thread_) : id(id_), thread(thread_), enabled(true)
{
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

void DIButton::setEnabled(bool enable)
{
	enabled = enable;
	thread->ai[id].setEnabled(enabled);
}

void DIButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{

	if (isMouseOver && enabled)
		g.setColour(Colours::antiquewhite);
	else
		g.setColour(Colours::darkgrey);
	g.fillRoundedRectangle(0, 0, 15, 15, 2);

	if (enabled)
	{
		if (isMouseOver)
			g.setColour(Colours::lightgreen);
		else
			g.setColour(Colours::forestgreen);
	} 
	else 
	{
		if (isMouseOver)
			g.setColour(Colours::lightgrey);
		else
			g.setColour(Colours::lightgrey);
	}
	g.fillRoundedRectangle(3, 3, 9, 9, 2);
}


void DIButton::timerCallback()
{

}

SourceTypeButton::SourceTypeButton(int id_, NIDAQThread* thread_, SOURCE_TYPE source) : id(id_), thread(thread_)
{

	update(source);

}

void SourceTypeButton::setId(int id_)
{
	id = id_;
}

int SourceTypeButton::getId()
{
	return id;
}

void SourceTypeButton::update(SOURCE_TYPE sourceType)
{
	switch (sourceType) {
	case SOURCE_TYPE::RSE:
		setButtonText("RSE"); return;
	case SOURCE_TYPE::NRSE:
		setButtonText("NRSE"); return;
	case SOURCE_TYPE::DIFF:
		setButtonText("DIFF"); return;
	case SOURCE_TYPE::PSEUDO_DIFF:
		setButtonText("PDIF"); return;
	default:
		break;
	}
}

void SourceTypeButton::timerCallback()
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


	/* Let the main GUI know the plugin is done initializing */
	MessageManagerLock mml;
	CoreServices::updateSignalChain(e);
	CoreServices::sendStatusMessage("NIDAQ plugin ready for acquisition!");

}


NIDAQEditor::NIDAQEditor(GenericProcessor* parentNode, NIDAQThread* t, bool useDefaultParameterEditors)
	: GenericEditor(parentNode, useDefaultParameterEditors), thread(t)
{

	draw();

}

void NIDAQEditor::draw()
{

	NIDAQThread* t = thread; 

	int nAI = t->getNumAnalogInputs();
	int nDI = t->getNumDigitalInputs();

	int maxChannelsPerColumn = 4;
	int aiChannelsPerColumn = nAI > 0 && nAI < maxChannelsPerColumn ? nAI : maxChannelsPerColumn;
	int diChannelsPerColumn = nDI > 0 && nDI < maxChannelsPerColumn ? nDI : maxChannelsPerColumn;

	aiButtons.clear();
	sourceTypeButtons.clear();

	for (int i = 0; i < nAI; i++)
	{

		int colIndex = i / aiChannelsPerColumn;
		int rowIndex = i % aiChannelsPerColumn + 1;
		int xOffset = colIndex * 75 + 40;
		int y_pos = 5 + rowIndex * 26;

		AIButton* a = new AIButton(i, thread);
		a->setBounds(xOffset, y_pos, 15, 15);
		a->addListener(this);
		addAndMakeVisible(a);
		aiButtons.add(a);

		SOURCE_TYPE sourceType = thread->getSourceTypeForInput(i);
		printf("Got source type for input %d: %d\n", i, sourceType);

		SourceTypeButton* b = new SourceTypeButton(i, thread, sourceType);
		b->setBounds(xOffset+18, y_pos-2, 26, 17);
		b->addListener(this);
		addAndMakeVisible(b);
		sourceTypeButtons.add(b);

	}

	diButtons.clear();

	int xOffset;
	for (int i = 0; i < nDI; i++)
	{

		int colIndex = i / diChannelsPerColumn;
		int rowIndex = i % diChannelsPerColumn + 1;
		xOffset = ((nAI % maxChannelsPerColumn == 0 ? 0 : 1) + nAI / aiChannelsPerColumn) * 75 + 38 + colIndex * 45;
		int y_pos = 5 + rowIndex * 26;

		DIButton* b = new DIButton(i, thread);
		b->setBounds(xOffset, y_pos, 15, 15);
		b->addListener(this);
		addAndMakeVisible(b);
		diButtons.add(b);

	}

	xOffset = xOffset + 25;

	sampleRateSelectBox = new ComboBox("SampleRateSelectBox");
	sampleRateSelectBox->setBounds(xOffset, 39, 85, 20);
	Array<String> sampleRates = t->getSampleRates();
	for (int i = 0; i < sampleRates.size(); i++)
	{
		sampleRateSelectBox->addItem(sampleRates[i], i + 1);
	}
	sampleRateSelectBox->setSelectedItemIndex(t->getSampleRateIndex(), false);
	sampleRateSelectBox->addListener(this);
	addAndMakeVisible(sampleRateSelectBox);

	voltageRangeSelectBox = new ComboBox("VoltageRangeSelectBox");
	voltageRangeSelectBox->setBounds(xOffset, 70, 85, 20);
	Array<String> voltageRanges = t->getVoltageRanges();
	for (int i = 0; i < voltageRanges.size(); i++)
	{
		voltageRangeSelectBox->addItem(voltageRanges[i], i + 1);
	}
	voltageRangeSelectBox->setSelectedItemIndex(t->getVoltageRangeIndex(), false);
	voltageRangeSelectBox->addListener(this);
	addAndMakeVisible(voltageRangeSelectBox);

	fifoMonitor = new FifoMonitor(thread);
	fifoMonitor->setBounds(xOffset + 2, 105, 70, 12);
	//addAndMakeVisible(fifoMonitor);

	if (t->getNumAvailableDevices() > 1)
	{
		swapDeviceButton = new UtilityButton("...", Font("Small Text", 15, Font::plain));
		swapDeviceButton->setBounds(xOffset + 60, 5, 25, 15);
		swapDeviceButton->addListener(this);
		swapDeviceButton->setAlpha(0.5f);
		addAndMakeVisible(swapDeviceButton);
	}
	
	desiredWidth = xOffset + 100;

	background = new EditorBackground(nAI, nDI);
	background->setBounds(0, 15, 500, 150);
	addAndMakeVisible(background);
	background->toBack();
	background->repaint();

	setDisplayName("NIDAQmx-(" + t->getProductName() + ")");

}

NIDAQEditor::~NIDAQEditor()
{

}

void NIDAQEditor::comboBoxChanged(ComboBox* comboBox)
{

	if (comboBox == sampleRateSelectBox)
	{
		if (!thread->isThreadRunning())
		{
			thread->setSampleRate(comboBox->getSelectedId() - 1);
			CoreServices::updateSignalChain(this);
		}
		else
		{
			comboBox->setSelectedItemIndex(thread->getSampleRateIndex());
		}
	}
	else // (comboBox == voltageRangeSelectBox)
	{
		if (!thread->isThreadRunning())
		{
			thread->setVoltageRange(comboBox->getSelectedId() - 1);
			CoreServices::updateSignalChain(this);
		}
		else
		{
			comboBox->setSelectedItemIndex(thread->getVoltageRangeIndex());
		}
	}

} 

void NIDAQEditor::buttonEvent(Button* button)
{

	if (aiButtons.contains((AIButton*)button))
	{
		((AIButton*)button)->setEnabled(!((AIButton*)button)->enabled);
		thread->toggleAIChannel(((AIButton*)button)->getId());
		repaint();
	}
	else if (diButtons.contains((DIButton*)button))
	{
		((DIButton*)button)->setEnabled(!((DIButton*)button)->enabled);
		thread->toggleDIChannel(((DIButton*)button)->getId());
		repaint();
	}
	else if (sourceTypeButtons.contains((SourceTypeButton*)button))
	{
		thread->toggleSourceType(((SourceTypeButton*)button)->getId());
		((SourceTypeButton*)button)->update(thread->getSourceTypeForInput(((SourceTypeButton*)button)->getId()));
	}
	else if (button == swapDeviceButton)
	{
		if (!thread->isThreadRunning())
		{
			thread->selectFromAvailableDevices();
			setDisplayName(thread->getProductName());
			draw();
		}
	}
}


void NIDAQEditor::saveCustomParameters(XmlElement* xml)
{
	xml->setAttribute("productName", thread->getProductName());
}


void NIDAQEditor::loadCustomParameters(XmlElement* xml)
{
	String productName = xml->getStringAttribute("productName", "NIDAQmx");
	if (!thread->swapConnection(productName));
		draw();
}
