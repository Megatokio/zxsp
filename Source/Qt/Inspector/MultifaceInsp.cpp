/*	Copyright  (c)	Günter Woigk 2015 - 2018
					mailto:kio@little-bat.de

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Permission to use, copy, modify, distribute, and sell this software and
	its documentation for any purpose is hereby granted without fee, provided
	that the above copyright notice appear in all copies and that both that
	copyright notice and this permission notice appear in supporting
	documentation, and that the name of the copyright holder not be used
	in advertising or publicity pertaining to distribution of the software
	without specific, written prior permission.  The copyright holder makes no
	representations about the suitability of this software for any purpose.
	It is provided "as is" without express or implied warranty.

	THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
	EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
	CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
	DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
	PERFORMANCE OF THIS SOFTWARE.
*/

#include "MultifaceInsp.h"
#include <QMouseEvent>
#include <QLabel>
#include <QTimer>
#include "Machine.h"
#include "Multiface/Multiface.h"
#include "Templates/NVPtr.h"


MultifaceInsp::MultifaceInsp(QWidget* p, MachineController* mc, volatile IsaObject* o, cstr image, const QRect& btnbox)
:
    Inspector(p,mc,o,image),
    buttonbox(btnbox)
{
	QWidget* button = new QWidget(this);
	button->setGeometry(btnbox);
	button->setCursor(Qt::PointingHandCursor);
	button->setVisible(yes);

	label_nmi_pending = new QLabel("NMI pending",this);
	label_paged_in    = new QLabel("Paged in",this);

	label_nmi_pending->move(l_x,l_y+0*l_d);
	label_paged_in->move(l_x,l_y+1*l_d);

	timer->start(1000/60);		// --> updateWidgets()
}







/*	Mouse click handler:
	- press the red button
*/
void MultifaceInsp::mousePressEvent(QMouseEvent* e)
{
	if(e->button()!=Qt::LeftButton) { Inspector::mousePressEvent(e); return; }

	xlogline("MultifaceInspector: mouse down at %i,%i",e->x(),e->y());

	if(buttonbox.contains(e->pos())) pressRedButton();
}


void MultifaceInsp::pressRedButton()
{
	NVPtr<Multiface>(multiface())->triggerNmi();
}


void MultifaceInsp::updateWidgets()
{
	if(label_nmi_pending->isVisible() != multiface()->nmi_pending)
	{
		label_nmi_pending->setVisible(multiface()->nmi_pending);
	}

	if(label_paged_in->isVisible() != multiface()->paged_in)
	{
		label_paged_in->setVisible(multiface()->paged_in);
	}
}
