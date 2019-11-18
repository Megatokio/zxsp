#pragma once
/*	Copyright  (c)	Günter Woigk 2013 - 2019
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

#include <QRect>
#include "Inspector.h"
#include "Ula/MmuTc2068.h"
class QPushButton;


class TccDockInspector : public Inspector
{
	bool		u;
	QPushButton*button_insert;
	int			x_overlay;
	int			y_overlay;
	cstr		imgdirpath;
	QPixmap		overlay_zxspemu_ejected;
	QPixmap		overlay_zxspemu_inserted;
	QPixmap		overlay_game_ejected;
	QPixmap		overlay_game_inserted;
	QWidget*	dock_slot;
	QWidget*	module_top_inserted;
	QWidget*	module_top_ejected;
	//QWidget*	module_front_inserted;
	QWidget*	module_front_ejected;

	enum CartridgeState { Invalid, NoCartridge, RomEjected, RomInserted };
	CartridgeState	cartridge_state;

	cstr		current_fpath;	// current text on front and evtl. graphics on top (1st)
	TccRomId	current_id;

public:
	TccDockInspector( QWidget*, MachineController*, volatile IsaObject* );
	~TccDockInspector();

	cstr	getSaveFilename() throws;
	cstr	getLoadFilename() throws;

protected:
	void	paintEvent(QPaintEvent*) override;
	void	mousePressEvent(QMouseEvent*) override;

	void	fillContextMenu(QMenu*) override;
	void	updateWidgets() override;

private:
	void	insert_cartridge(cstr filepath = nullptr);
	void	insert_or_eject_cartridge();	// Button "Insert/Eject"
	void	eject_cartridge();				// Context menu
	void	save_as();						// ""
	void	remove_cartridge();				// ""
	void	insert_again();					// ""
};


















