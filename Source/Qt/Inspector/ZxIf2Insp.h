#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Joy/ZxIf2.h"
#include "SinclairJoyInsp.h"
class QLabel;
class QMenu;
class QPushButton;


namespace gui
{

class ZxIf2Insp : public SinclairJoyInsp
{
	volatile ZxIf2* const zxif2;

	QPushButton* button_insert_eject;
	QLabel*		 label_romfilename;
	cstr		 old_romfilepath; // 2nd

public:
	ZxIf2Insp(QWidget*, MachineController*, volatile ZxIf2*);

	void insertRom(cstr filepath);

protected:
	void fillContextMenu(QMenu* menu) override;
	void updateWidgets() override;

private:
	void slotInsertEjectRom();
};

} // namespace gui
