#pragma once
// Copyright (c) 2013 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Inspector.h"
#include "Ula/MmuTc2068.h"
#include <QRect>
class QPushButton;


namespace gui
{

class TccDockInspector : public Inspector
{
	volatile MmuTc2068* const dock;

	bool		 u;
	QPushButton* button_insert;
	int			 x_overlay;
	int			 y_overlay;
	cstr		 imgdirpath;
	QPixmap		 overlay_zxspemu_ejected;
	QPixmap		 overlay_zxspemu_inserted;
	QPixmap		 overlay_game_ejected;
	QPixmap		 overlay_game_inserted;
	QWidget*	 dock_slot;
	QWidget*	 module_top_inserted;
	QWidget*	 module_top_ejected;
	// QWidget*	module_front_inserted;
	QWidget* module_front_ejected;

	enum CartridgeState { Invalid, NoCartridge, RomEjected, RomInserted };
	CartridgeState cartridge_state;

	cstr	 current_fpath; // current text on front and evtl. graphics on top (1st)
	TccRomId current_id;

public:
	TccDockInspector(QWidget*, MachineController*, volatile MmuTc2068*);
	~TccDockInspector() override;

	cstr getSaveFilename();
	cstr getLoadFilename();

protected:
	void paintEvent(QPaintEvent*) override;
	void mousePressEvent(QMouseEvent*) override;

	void fillContextMenu(QMenu*) override;
	void updateWidgets() override;

private:
	void insert_cartridge(cstr filepath = nullptr);
	void insert_or_eject_cartridge(); // Button "Insert/Eject"
	void eject_cartridge();			  // Context menu
	void save_as();					  // ""
	void remove_cartridge();		  // ""
	void insert_again();			  // ""
};

} // namespace gui
