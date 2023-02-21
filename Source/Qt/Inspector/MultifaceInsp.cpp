// Copyright (c) 2015 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MultifaceInsp.h"
#include "Machine.h"
#include "Multiface/Multiface.h"
#include "Templates/NVPtr.h"
#include <QLabel>
#include <QMouseEvent>
#include <QTimer>


namespace gui
{

MultifaceInsp::MultifaceInsp(
	QWidget* p, MachineController* mc, volatile IsaObject* o, cstr image, const QRect& btnbox) :
	Inspector(p, mc, o, image),
	buttonbox(btnbox)
{
	QWidget* button = new QWidget(this);
	button->setGeometry(btnbox);
	button->setCursor(Qt::PointingHandCursor);
	button->setVisible(yes);

	label_nmi_pending = new QLabel("NMI pending", this);
	label_paged_in	  = new QLabel("Paged in", this);

	label_nmi_pending->move(l_x, l_y + 0 * l_d);
	label_paged_in->move(l_x, l_y + 1 * l_d);

	timer->start(1000 / 60); // --> updateWidgets()
}


/*	Mouse click handler:
	- press the red button
*/
void MultifaceInsp::mousePressEvent(QMouseEvent* e)
{
	if (e->button() != Qt::LeftButton)
	{
		Inspector::mousePressEvent(e);
		return;
	}

	xlogline("MultifaceInspector: mouse down at %i,%i", e->x(), e->y());

	if (buttonbox.contains(e->pos())) pressRedButton();
}


void MultifaceInsp::pressRedButton()
{
	auto* mf = dynamic_cast<volatile Multiface*>(object);
	if (mf) NVPtr<Multiface>(mf)->triggerNmi();
}


void MultifaceInsp::updateWidgets()
{
	if (!machine || !object) return;

	if (auto* mf = dynamic_cast<volatile Multiface*>(object))
	{
		if (label_nmi_pending->isVisible() != mf->nmi_pending) label_nmi_pending->setVisible(mf->nmi_pending);
		if (label_paged_in->isVisible() != mf->paged_in) label_paged_in->setVisible(mf->paged_in);
	}
}

} // namespace gui
