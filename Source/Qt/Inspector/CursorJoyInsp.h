#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Joy/CursorJoy.h"
#include "JoyInsp.h"

namespace gui
{

class CursorJoyInsp : public JoyInsp
{
public:
	CursorJoyInsp(QWidget*, MachineController*, volatile CursorJoy*, cstr image);

protected:
	void updateWidgets() override;
};


class ProtekJoyInsp : public CursorJoyInsp
{
public:
	ProtekJoyInsp(QWidget* w, MachineController* mc, volatile ProtekJoy* j) :
		CursorJoyInsp(w, mc, j, "/Images/protek_js_if.jpg")
	{}
};

} // namespace gui
