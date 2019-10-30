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

#include "Multiface3Insp.h"
#include <QLabel>
#include "Multiface/Multiface3.h"


Multiface3Insp::Multiface3Insp(QWidget*w, MachineController* mc, volatile IsaObject *i )
:
    MultifaceInsp(w,mc,i,"Images/multiface3.jpg",QRect(234,22,30,30))	// red button		x y w h
{
	label_visible = new QLabel("Visible",this);
	label_ramonly = new QLabel("Ram at $0000",this);


	label_visible->move(l_x,l_y+0*l_d);
	label_nmi_pending->move(l_x,l_y+1*l_d);
	label_paged_in->move(l_x,l_y+2*l_d);
	label_ramonly->move(l_x,l_y+2*l_d);		// same position as paged_in
}


void Multiface3Insp::updateWidgets()
{
	MultifaceInsp::updateWidgets();

	if(label_visible->isVisible() != multiface3()->mf_enabled)
	{
		label_visible->setVisible(multiface3()->mf_enabled);
	}

	if(label_ramonly->isVisible() != multiface3()->all_ram)
	{
		label_ramonly->setVisible(multiface3()->all_ram);
	}
}
