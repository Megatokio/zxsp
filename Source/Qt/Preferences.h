#pragma once
/*	Copyright  (c)	Günter Woigk 2012 - 2019
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice, this permission notice and the
	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY,
	NOT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
	A PARTICULAR PURPOSE, AND IN NO EVENT SHALL THE COPYRIGHT HOLDER
	BE LIABLE FOR ANY DAMAGES ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
*/

#include <QWidget>
#include <QRadioButton>
#include "kio/kio.h"
#include "ZxInfo.h"


class Preferences : public QWidget
{
	int modelList[num_models];

public:
	explicit Preferences(QWidget* parent = NULL);

private:
	void setSaveAndRestore(bool);
	void setDefaultScreenSize(int);
	void setStartOpenDiskDrive(bool);
	void setStartOpenKeyboard(bool);
	void setStartOpenTaperecorder(bool);
	void setStartOpenMachineImage(bool);
	void setStartEnableAudioIn(bool);
	void setStartStopTape(bool);
	void setFastLoadTape(bool);
	void setNewMachineModel(int);
	void setNewMachineKeyboardMode(int);
	void setNewMachineAy(bool);
	void setNewMachineJoystick(bool);
	void setNewMachineRampack(bool);
	void setNewMachineDivide(bool);
	void setSnapshotIndividualSettings(bool);
	void setSnapshotKeyboardMode(int);
	void setCheckForUpdate(bool);
};














