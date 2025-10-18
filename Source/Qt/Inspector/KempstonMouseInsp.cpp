// Copyright (c) 2006 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "KempstonMouseInsp.h"
#include "Mouse.h"
#include "Templates/RCPtr.h"
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QtGui>


namespace gui
{

KempstonMouseInsp::KempstonMouseInsp(QWidget* w, MachineController* mc, volatile KempstonMouse* mif) :
	Inspector(w, mc, mif, "/Images/kempston_mouse_if.jpg"),
	mif(mif),
	display_x(newLineEdit("0", 32)),
	display_y(newLineEdit("0", 32)),
	display_buttons(newLineEdit("%------11", 100)),
	combobox_scale(new QComboBox()),
	button_grab_mouse(new QPushButton("Grab mouse", this)),
	old_x(0),
	old_y(0),
	old_buttons(0xff),
	old_grabbed(no)
{
	xlogIn("new KempstonMouseInsp");
	assert(machine && object);
	assert(object->isA(isa_KempstonMouse));

	display_buttons->setMaximumWidth(100);

	combobox_scale->setFocusPolicy(Qt::NoFocus);
	combobox_scale->addItems(
		QStringList() << "1:1"
					  << "1:2"
					  << "1:3"
					  << "1:4");
	combobox_scale->setMinimumWidth(80);
	connect(
		combobox_scale, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
		[this](int index) {
			assert(validReference(this->mif));
			nvptr(this->mif)->setScale(index + 1);
		});

	button_grab_mouse->setMinimumWidth(100);
	connect(button_grab_mouse, &QPushButton::clicked, this, [this] {
		mouse.grab(this);
		timer->start(1000 / 20);
	});

	QGridLayout* g = new QGridLayout(this);
	g->setContentsMargins(20, 10, 20, 5);
	g->setVerticalSpacing(4);
	g->setRowStretch(0, 100);
	//	g->setColumnStretch(0,0);	// X=
	//	g->setColumnStretch(1,0);	// [X]
	//	g->setColumnStretch(2,0);	// Y=
	//	g->setColumnStretch(3,0);	// [Y]
	g->setColumnStretch(4, 100); /* spacer */
	//	g->setColumnStretch(5,0);	// (btn)

	g->addWidget(new QLabel("X ="), 1, 0);
	g->addWidget(display_x, 1, 1);
	g->addWidget(new QLabel("Y ="), 1, 2);
	g->addWidget(display_y, 1, 3);

	g->addWidget(new QLabel("Buttons:"), 2, 0, 1, 4, Qt::AlignLeft);
	g->addWidget(display_buttons, 2, 0, 1, 4, Qt::AlignRight);

	g->addWidget(new QLabel(""), 1, 4); // Spalte muss belegt werden, sonst wird sie vom Layout ignoriert

	g->addWidget(combobox_scale, 1, 5);
	g->addWidget(button_grab_mouse, 2, 5);

	clearFocus();
	KempstonMouseInsp::updateWidgets(); // once, assumes class is final
}

void KempstonMouseInsp::hideEvent(QHideEvent* e)
{
	xlogline("KempstonMouseInsp::hideEvent");
	mouse.ungrab();
	Inspector::hideEvent(e);
}

KempstonMouseInsp::~KempstonMouseInsp()
{
	xlogline("~KempstonMouseInsp");
	mouse.ungrab();
}

void KempstonMouseInsp::updateWidgets()
{
	xxlogIn("KempstonMouseInsp:updateWidgets");
	assert(validReference(mif));

	if (QApplication::keyboardModifiers() & Qt::CTRL) { mouse.ungrab(); }

	uint8 newx, newy;
	uint  newbuttons;

	{
		auto mouse = nvptr(mif);
		newx	   = mouse->getXPos();
		newy	   = mouse->getYPos();
		newbuttons = mouse->getButtons();
	}

	if (old_x != newx)
	{
		old_x = newx;
		display_x->setText(tostr(newx));
	}

	if (old_y != newy)
	{
		old_y = newy;
		display_y->setText(tostr(newy));
	}

	if (old_buttons != newbuttons)
	{
		old_buttons = newbuttons;
		display_buttons->setText(binstr(newbuttons, "%------LR", "%------11"));
	}

	bool newgrabbed = mouse.isGrabbed();
	if (old_grabbed != newgrabbed)
	{
		old_grabbed = newgrabbed;
		button_grab_mouse->setText(newgrabbed ? "CMD to exit" : "Grab mouse");
		if (!newgrabbed) timer->stop();
	}

	if (combobox_scale->currentIndex() != mif->getScale() - 1)
	{
		combobox_scale->setCurrentIndex(mif->getScale() - 1); //
	}
}

} // namespace gui
