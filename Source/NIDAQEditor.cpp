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

EditorBackground::EditorBackground(int nAI, int nDI) {}

void EditorBackground::paint(Graphics& g)
{
	/* Draw AI channels */
	for (int i = 0; i < 4; i++)
	{
		g.setColour(Colours::lightgrey);
		g.drawRoundedRectangle(90 * i + 32, 13, 32, 98, 4, 3);
		g.setColour(Colours::darkgrey);
		g.drawRoundedRectangle(90 * i + 32, 13, 32, 98, 4, 1);

		g.setColour(Colours::darkgrey);
		g.setFont(20);
		g.drawText(String(i + 1), 90 * i + 72, 15, 10, 10, Justification::centred);
		g.setFont(8);
		g.drawText(String("0"), 90 * i + 87, 100, 50, 10, Justification::centredLeft);
		g.drawText(String("100"), 90 * i + 87, 60, 50, 10, Justification::centredLeft);
		g.drawText(String("%"), 90 * i + 87, 80, 50, 10, Justification::centredLeft);

		for (int j = 0; j < 4; j++)
		{
			g.setFont(10);
			g.drawText(String(j + 1), 90 * i + 22, 90 - j * 22, 10, 10, Justification::centredLeft);
		}
	}

	g.setColour(Colours::darkgrey);
	g.setFont(10);
	g.drawText(String("SAMPLE RATE"), 90 * (1)+32, 13, 100, 10, Justification::centredLeft);

}

FifoMonitor::FifoMonitor(int id_, NIDAQThread* thread_) : id(id_), thread(thread_), fillPercentage(0.0)
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

BackgroundLoader::BackgroundLoader(NIDAQThread* thread, NIDAQEditor* editor)
	: Thread("NIDAQ Loader"), np(thread), ed(editor)
{
}

BackgroundLoader::~BackgroundLoader()
{
}

void BackgroundLoader::run()
{
	/* Open the NPX-PXI probe connections in the background to prevent this plugin from blocking the main GUI*/
	np->openConnection();

	/* Let the main GUI know the plugin is done initializing */
	MessageManagerLock mml;
	CoreServices::updateSignalChain(ed);
	CoreServices::sendStatusMessage("NIDAQ plugin ready for acquisition!");

}


NIDAQEditor::NIDAQEditor(GenericProcessor* parentNode, NIDAQThread* t, bool useDefaultParameterEditors)
	: GenericEditor(parentNode, useDefaultParameterEditors)
{

	thread = t;

	for (int i = 0; i < 4; i++)
	{
		int colIndex = i / 4;
		int rowIndex = i % 4 + 1;
		int x_pos = colIndex * 90 + 40;
		int y_pos = 125 - rowIndex * 22;

		AIButton* p = new AIButton(i, thread);
		p->setBounds(x_pos, y_pos, 15, 15);
		p->addListener(this);
		addAndMakeVisible(p);
		aiButtons.add(p);

		//p->setId(? );

	}

	int cols = aiButtons.size() / 4; 

	for (int i = 0; i < cols; i++)
	{
		int x_pos = i * 90 + 70;
		int y_pos = 50;

		UtilityButton* b = new UtilityButton("", Font("Small Text", 13, Font::plain));
		b->setBounds(x_pos, y_pos, 30, 20);
		b->addListener(this);
		addAndMakeVisible(b);
		directoryButtons.add(b);

		savingDirectories.add(File());

		FifoMonitor* f = new FifoMonitor(i, thread);
		f->setBounds(x_pos + 2, 75, 12, 50);
		addAndMakeVisible(f);
		fifoMonitors.add(f);
	}

	sampleRateSelectBox = new ComboBox("SampleRateSelectBox");
	sampleRateSelectBox->setBounds(90 * (1)+32, 39, 38, 20);
	sampleRateSelectBox->addItem(String("1MHz"),1);
	sampleRateSelectBox->setSelectedItemIndex(0, false);
	sampleRateSelectBox->addListener(this);
	addAndMakeVisible(sampleRateSelectBox);

	desiredWidth = 100 * 1 + 120;

	background = new EditorBackground(4,0);
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

		if (directoryButtons.contains((UtilityButton*)button))
		{
			// open file chooser to select the saving location for this basestation
			/*
			int slotIndex = directoryButtons.indexOf((UtilityButton*)button);
			File currentDirectory = thread->getDirectoryForSlot(slotIndex);
			FileChooser fileChooser("Select directory for NPX file.", currentDirectory);
			if (fileChooser.browseForDirectory())
			{
				File result = fileChooser.getResult();
				String pathName = result.getFullPathName();
				thread->setDirectoryForSlot(slotIndex, result);

				savingDirectories.set(slotIndex, result);
				UtilityButton* ub = (UtilityButton*)button;
				ub->setLabel(pathName.substring(0, 3));
			}
			*/

		}

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
