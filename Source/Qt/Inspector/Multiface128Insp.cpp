/*	Copyright  (c)	Günter Woigk 2012 - 2018
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

#include "Multiface128Insp.h"
#include <QLabel>
#include "Multiface/Multiface128.h"


Multiface128Insp::Multiface128Insp(QWidget*w, MachineController* mc, volatile IsaObject *i )
:
    MultifaceInsp(w,mc,i,"Images/multiface128.jpg",QRect(220,20,30,30))	// red button		x y w h
{
	label_visibility = new QLabel("Visible",this);

	label_visibility->move(l_x,l_y+0*l_d);
	label_nmi_pending->move(l_x,l_y+1*l_d);
	label_paged_in->move(l_x,l_y+2*l_d);
}


void Multiface128Insp::updateWidgets()
{
	MultifaceInsp::updateWidgets();

	if(label_visibility->isVisible() != multiface128()->mf_enabled)
	{
		label_visibility->setVisible(multiface128()->mf_enabled);
	}
}

