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

#ifndef __NIDAQEDITOR_H__
#define __NIDAQEDITOR_H__

#include <ProcessorHeaders.h>
#include <EditorHeaders.h>
#include "NIDAQThread.h"

class UtilityButton;
/**

User interface for the BNC-2110 terminal block.

@see SourceNode

*/
class SourceNode;

class NIDAQEditor;
class NIDAQCanvas;
class NIDAQInterface;
class Annotation;
class ColorSelector;

class EditorBackground : public Component
{
public:
	EditorBackground(int nAI, int nDI);

private:
	void paint(Graphics& g);
	int nAI;
	int nDI;

};

class AIButton : public ToggleButton, public Timer
{
public:
	AIButton(int id, NIDAQThread* thread);

	void setId(int id);
	int getId(); 
	void setSelectedState(bool);

	void setInputStatus(int status);
	void timerCallback();

	bool connected;
	NIDAQThread* thread;

private:
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);

	int id;
	int status;
	bool selected;
};

class DIButton : public ToggleButton, public Timer
{
public:
	DIButton(int id, NIDAQThread* thread);

	void setId(int id);
	int getId();
	void setSelectedState(bool);

	void setInputStatus(int status);
	void timerCallback();

	bool connected;
	NIDAQThread* thread;

private:
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);

	int id;
	int status;
	bool selected;
};

class FifoMonitor : public Component, public Timer
{
public:
	FifoMonitor(NIDAQThread* thread);

	void setFillPercentage(float percentage);

	void timerCallback();

private:
	void paint(Graphics& g);

	float fillPercentage;
	NIDAQThread* thread;
	int id;
};

class BackgroundLoader : public Thread
{
public:
	BackgroundLoader(NIDAQThread* t, NIDAQEditor* e);
	~BackgroundLoader();
	void run();
private:
	NIDAQThread* t;
	NIDAQEditor* e;
};

class NIDAQEditor : public GenericEditor, public ComboBox::Listener
{
public:
	NIDAQEditor(GenericProcessor* parentNode, NIDAQThread* thread, bool useDefaultParameterEditors);
	virtual ~NIDAQEditor();

	void buttonEvent(Button* button);
	void comboBoxChanged(ComboBox*);

	void saveEditorParameters(XmlElement*);
	void loadEditorParameters(XmlElement*);

private:

	OwnedArray<AIButton> aiButtons;
	OwnedArray<DIButton> diButtons;

	ScopedPointer<ComboBox> sampleRateSelectBox;
	ScopedPointer<UtilityButton> directoryButton;
	ScopedPointer<FifoMonitor> fifoMonitor;

	Array<File> savingDirectories;

	ScopedPointer<BackgroundLoader> uiLoader;
	ScopedPointer<EditorBackground> background;

	NIDAQThread* thread;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NIDAQEditor);

};


#endif  // __RHD2000EDITOR_H_2AD3C591__
