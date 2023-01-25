#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include <QWidget>
#include <QRadioButton>
#include "kio/kio.h"
#include "ZxInfo.h"


class Preferences : public QWidget
{
	int modelList[num_models];

public:
	explicit Preferences(QWidget* parent = nullptr);

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














