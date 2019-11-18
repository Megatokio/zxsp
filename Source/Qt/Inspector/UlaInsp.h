/*	Copyright  (c)	Günter Woigk 2009 - 2019
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

#ifndef ULAINSP_H
#define ULAINSP_H

#include <QTimer>
#include <QObject>
#include "Inspector.h"

class QTimer;
class QPushButton;
class QCheckBox;
class QLineEdit;
class QLabel;


class UlaInsp : public Inspector
{
	volatile Mmu* mmu;

	QPushButton* btn_restore_defaults;

	struct
	{
	// clock settings.:
		QLineEdit*  cpu_clock_overdrive;
		QLineEdit*  ula_clock;
		QLineEdit*  cpu_clock_predivider;
		QLineEdit*  cpu_clock;

	// screen settings.:
		QLineEdit*  top_rows;
		QLineEdit*  screen_rows;
		QLineEdit*  bottom_rows;
		QLineEdit*  screen_columns;
		QLineEdit*  bytes_per_row;
		QLineEdit*  cpu_cycles_per_row;
		QLineEdit*  frames_per_second;
		QLineEdit*  cpu_cycles_per_frame;

	// contended ram:
		QCheckBox*  checkbox_enable_cpu_waitcycles;
		QLineEdit*  waitmap_offset;
		QLineEdit*  waitmap;

	// misc. screen bits:
		QLineEdit*  border_color;
		QLineEdit*  mic_bit;
		QLineEdit*  ear_bit;
		QLineEdit*  frames_hit;

	// mmu port 7ffd & 1ffd:
		QLineEdit*  port_7ffd;
		QLineEdit*  port_1ffd;
		QLineEdit*  page_c000;
		QLineEdit*  page_8000;
		QLineEdit*  page_4000;
		QLineEdit*  page_0000;
		QLineEdit*  video_page;
		QCheckBox*  checkbox_mmu_locked;
		QCheckBox*  checkbox_ram_only;
		QLineEdit*  disc_motor;
		QLineEdit*  printer_strobe;
		QLabel*     label_rom0000;

	} inputs;

	struct
	{
		//float	cpu_clock_overdrive;
		//uint32 ula_clock;
		//uint	cpu_clock_predivider;
		uint32  cpu_clock;
		uint    top_rows;
		uint    screen_rows;
		uint    bottom_rows;
		uint    screen_columns;
		uint    bytes_per_row;
		uint    cpu_cycles_per_row;
		float   frames_per_second;
		uint32  cpu_cycles_per_frame;
		bool    checkbox_enable_cpu_waitcycles;
		uint    waitmap_offset;
		uint8   waitmap;
		uint8   border_color;
		uint8   mic_bit;
		uint8   ear_bit;
		float   frames_hit;
		uint8   port_7ffd;
		uint8   port_1ffd;
		uint    page_c000;
		uint    page_8000;
		uint    page_4000;
		uint    page_0000;
		uint    video_page;
		bool    checkbox_mmu_locked;
		bool    checkbox_ram_only;
		bool    disc_motor;
		bool    printer_strobe;
	} values;

public:
	UlaInsp( QWidget*, MachineController*, volatile Machine* );

protected:
	 void	updateWidgets() override;
};

#endif


























