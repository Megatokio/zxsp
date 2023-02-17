#pragma once
// Copyright (c) 2000 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Inspector.h"
#include "Keyboard.h"
#include <QRegion>


namespace gui
{

class KeyboardInspector : public Inspector
{
	Model  model;
	uint8  mousekey;
	Keymap keymap; // currently displayed keyboard state

public:
	KeyboardInspector(QWidget*, MachineController*, volatile IsaObject*);

private:
	virtual QRect keyRect(uint8);
	QRegion		  keyRegion(uint8);
	uint8		  findKeyForPoint(QPoint);

protected:
	void paintEvent(QPaintEvent*) override;
	void mouseDoubleClickEvent(QMouseEvent* e) override { mousePressEvent(e); }
	void mousePressEvent(QMouseEvent*) override;
	void mouseReleaseEvent(QMouseEvent*) override;
	void mouseMoveEvent(QMouseEvent*) override;
	// void		showEvent(QShowEvent*) override;
	bool event(QEvent*) override;

	void fillContextMenu(QMenu*) override;
	void updateWidgets() override;
};


class Tk90xKbdInsp : public KeyboardInspector
{
	QRect keyRect(uint8) override;

public:
	Tk90xKbdInsp(QWidget* p, MachineController* m, volatile IsaObject* i) : KeyboardInspector(p, m, i) {}
};


class Ts1000KbdInsp : public KeyboardInspector
{
	QRect keyRect(uint8) override;

public:
	Ts1000KbdInsp(QWidget* p, MachineController* m, volatile IsaObject* i) : KeyboardInspector(p, m, i) {}
};

class Ts1500KbdInsp : public KeyboardInspector
{
	QRect keyRect(uint8) override;

public:
	Ts1500KbdInsp(QWidget* p, MachineController* m, volatile IsaObject* i) : KeyboardInspector(p, m, i) {}
};

class Tk85KbdInsp : public KeyboardInspector
{
	QRect keyRect(uint8) override;

public:
	Tk85KbdInsp(QWidget* p, MachineController* m, volatile IsaObject* i) : KeyboardInspector(p, m, i) {}
};

class Tk95KbdInsp : public KeyboardInspector
{
	QRect keyRect(uint8) override;

public:
	Tk95KbdInsp(QWidget* p, MachineController* m, volatile IsaObject* i) : KeyboardInspector(p, m, i) {}
};

} // namespace gui
