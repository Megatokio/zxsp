/*	Copyright  (c)	Günter Woigk 2012 - 2019
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice, this permission notice and the
	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY,
	NOT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
	A PARTICULAR PURPOSE, AND IN NO EVENT SHALL THE COPYRIGHT HOLDER
	BE LIABLE FOR ANY DAMAGES ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
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
