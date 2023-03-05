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

User interface for NIDAQmx devices.

@see SourceNode

*/
class SourceNode;

class NIDAQEditor;
class NIDAQCanvas;
class NIDAQInterface;
class Annotation;
class ColorSelector;

class PopupConfigurationWindow : public Component, public ComboBox::Listener
{

public:
    
    /** Constructor */
    PopupConfigurationWindow(NIDAQEditor* editor);

    /** Destructor */
    ~PopupConfigurationWindow() { }

	void comboBoxChanged(ComboBox*);

private:

	NIDAQEditor* editor;

    ScopedPointer<Label>  analogLabel;
    ScopedPointer<ComboBox> analogChannelCountSelect;

	ScopedPointer<Label>  digitalLabel;
	ScopedPointer<ComboBox> digitalChannelCountSelect;

	ScopedPointer<Label>  digitalReadLabel;
	ScopedPointer<ComboBox> digitalReadSelect;

};

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
	void setEnabled(bool);
	void timerCallback();

	NIDAQThread* thread;

	friend class NIDAQEditor;

private:
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);

	int id;
	bool enabled;

};

class DIButton : public ToggleButton, public Timer
{
public:
	DIButton(int id, NIDAQThread* thread);

	void setId(int id);
	int getId();
	void setEnabled(bool);
	void timerCallback();

	NIDAQThread* thread;

	friend class NIDAQEditor;

private:
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);

	int id;
	bool enabled;

};

class SourceTypeButton : public TextButton, public Timer
{
public:
	SourceTypeButton(int id, NIDAQThread* thread, SOURCE_TYPE source);

	void setId(int id);
	int getId();
	void toggleSourceType();
	void timerCallback();

	void update(SOURCE_TYPE sourceType);

	NIDAQThread* thread;

	friend class NIDAQEditor;

private:

	int id;
	bool enabled;

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

class NIDAQEditor : public GenericEditor, public ComboBox::Listener, public Button::Listener
{
public:

	/** Constructor */
	NIDAQEditor(GenericProcessor* parentNode, NIDAQThread* thread);

	/** Destructor */
	virtual ~NIDAQEditor();

	void draw();

	void update(int analogCount, int digitalCount, int digitalRead);

	void buttonEvent(Button* button);
	void comboBoxChanged(ComboBox*);

	/** Respond to button presses */
	void buttonClicked(Button* button) override;

	void updateSettings(int analogCount, int digitalCount, int digitalRead);

	int getTotalAvailableAnalogInputs() { return thread->getTotalAvailableAnalogInputs(); };
	int getTotalAvailableDigitalInputs() { return thread-> getTotalAvailableDigitalInputs(); };

	int getNumActiveAnalogInputs() { return thread->getNumActiveAnalogInputs(); };
	int getNumActiveDigitalInputs() { return thread->getNumActiveDigitalInputs(); };

	int getDigitalReadSize() { return thread->getDigitalReadSize(); };

	void saveCustomParametersToXml(XmlElement*) override;
	void loadCustomParametersFromXml(XmlElement*) override;

private:

	OwnedArray<AIButton> aiButtons;
	OwnedArray<TextButton> sourceTypeButtons;
	OwnedArray<DIButton> diButtons;

	ScopedPointer<ComboBox> deviceSelectBox;
	ScopedPointer<ComboBox> sampleRateSelectBox;
	ScopedPointer<ComboBox> voltageRangeSelectBox;
	ScopedPointer<FifoMonitor> fifoMonitor;

	ScopedPointer<UtilityButton> configureDeviceButton;

	Array<File> savingDirectories;

	ScopedPointer<BackgroundLoader> uiLoader;
	ScopedPointer<EditorBackground> background;

	PopupConfigurationWindow* currentConfigWindow;

	NIDAQThread* thread;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NIDAQEditor);

};


#endif  // __RHD2000EDITOR_H_2AD3C591__
