/*	Copyright  (c)	Günter Woigk 2013 - 2018
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


#ifndef DOCKINSPECTOR_H
#define DOCKINSPECTOR_H

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

#endif
















