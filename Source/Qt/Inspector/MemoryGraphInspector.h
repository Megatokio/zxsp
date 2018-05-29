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


#ifndef MEMORYGRAPHINSPECTOR_H
#define MEMORYGRAPHINSPECTOR_H

#include "MemoryInspector.h"
#include "SimpleTerminal.h"
class MyScrollBar;
class QBoxLayout;
class QWidget;
class QPushButton;
class GWidget;


class MemoryGraphInspector : public MemoryInspector
{
	SimpleTerminal*	address_view;
	GWidget*		graphics_view;

	int				cw;			// Character width
	int				rh;			// Row height

public:
	MemoryGraphInspector(QWidget*, MachineController*, volatile IsaObject* );

protected:
	void	resizeEvent(QResizeEvent*) override;
	void	showEvent(QShowEvent*) override;
	//void	wheelEvent(QWheelEvent*) override;
	//void	paintEvent(QPaintEvent*) override;
	//void	mousePressEvent(QMouseEvent*) override;
	//bool	event(QEvent*) override;
	//void	keyPressEvent(QKeyEvent*) override;
	//void	keyReleaseEvent(QKeyEvent*) override;
	//void	saveSettings() override;

	void	updateScrollbar() override;
	void	adjustMaxSizeDuringResize() override;	// from ToolWindow
	void	adjustSize(QSize&) override;			// from ToolWindow
	void	updateWidgets() override;

private:
	void	updateTooltip();
	int		width_for_bytes(int);
	int		bytes_for_width(int);
	int		height_for_rows(int);
	int		rows_for_height(int);
	void	validate_rows();
	void	validate_bytes_per_row();
	void	validate_scrollposition();

	void	slotSet32BytesPerRow();
	//void	setScrollPosition(int) override;	// scrollbar
};

#endif
