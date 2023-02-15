#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MemoryInspector.h"
#include "SimpleTerminal.h"
class MyScrollBar;
class QBoxLayout;
class QWidget;
class QPushButton;
class GWidget;


class MemoryGraphInspector : public MemoryInspector
{
	SimpleTerminal* address_view;
	GWidget*		graphics_view;

	int cw; // Character width
	int rh; // Row height

public:
	MemoryGraphInspector(QWidget*, MachineController*, volatile IsaObject*);

protected:
	void resizeEvent(QResizeEvent*) override;
	void showEvent(QShowEvent*) override;
	// void	wheelEvent(QWheelEvent*) override;
	// void	paintEvent(QPaintEvent*) override;
	// void	mousePressEvent(QMouseEvent*) override;
	// bool	event(QEvent*) override;
	// void	keyPressEvent(QKeyEvent*) override;
	// void	keyReleaseEvent(QKeyEvent*) override;
	// void	saveSettings() override;

	void updateScrollbar() override;
	void adjustMaxSizeDuringResize() override; // from ToolWindow
	void adjustSize(QSize&) override;		   // from ToolWindow
	void updateWidgets() override;

private:
	void updateTooltip();
	int	 width_for_bytes(int);
	int	 bytes_for_width(int);
	int	 height_for_rows(int);
	int	 rows_for_height(int);
	void validate_rows();
	void validate_bytes_per_row();
	void validate_scrollposition();

	void slotSet32BytesPerRow();
	// void	setScrollPosition(int) override;	// scrollbar
};
