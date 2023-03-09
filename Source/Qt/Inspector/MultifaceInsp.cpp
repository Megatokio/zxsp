// Copyright (c) 2015 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MultifaceInsp.h"
#include "Multiface/Multiface.h"
#include "Templates/NVPtr.h"
#include <QLabel>
#include <QMouseEvent>
#include <QTimer>

namespace gui
{

MultifaceInsp::MultifaceInsp(
	QWidget* p, MachineController* mc, volatile Multiface* o, cstr image, const QRect& btnbox) :
	Inspector(p, mc, o, image),
	mf(o),
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

void MultifaceInsp::mousePressEvent(QMouseEvent* e)
{
	// Mouse click handler:
	// - press the red button

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
	xlogIn("MultifaceInspector::pressRedButton");
	assert(validReference(mf));

	nvptr(mf)->triggerNmi();
}

void MultifaceInsp::updateWidgets()
{
	// timer

	xxlogIn("MultifaceInspector::updateWidgets");
	assert(validReference(mf));

	if (label_nmi_pending->isVisible() != mf->isNmiPending()) label_nmi_pending->setVisible(mf->isNmiPending());
	if (label_paged_in->isVisible() != mf->isPagedIn()) label_paged_in->setVisible(mf->isPagedIn());
}

} // namespace gui


/*
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
*/
