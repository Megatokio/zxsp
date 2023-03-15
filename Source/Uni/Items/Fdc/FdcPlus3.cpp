// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "FdcPlus3.h"
#include "Fdc.h"
#include "Fdc765.h"
#include "FloppyDiskDrive.h"


/*
http://www.worldofspectrum.org/faq/reference/ports.htm
http://www.cpctech.org.uk/docs/upd765a/necfdc.htm
http://k1.spdns.de/Vintage/Schneider%20CPC/Das%20Schneider%20CPC%20Systembuch/z193.htm#A

	Port: 0011.----.----.--0-	+3 FDC Data
	Port: 0010.----.----.--0-	+3 FDC Main status register

0x1ffd: Memory control register:
		Bit 3: 0=motors off; 1=motors on
0x2ffd: Reading from this port will return the main status register of the 765A.
0x3ffd: out: write data to FDC, in: read data from FDC


Anschluss:

	RESET <- System Reset
	/RD   <- Gate Array
	/WR   <- Gate Array
	/CS	  <- GND			der FDC ist immer selektiert
	A0    <- CPU.A12
	D0…7  <->

	DRQ   -> x				DMA request: nicht angeschlossen
	/DK   <- Vcc			DMA ACK
	TC    <- GND			DMA Terminal Count
	INT   -> SED9420.DRQ	keine Interrupts!
	MFM   -> x				FDD VCO's operation mode: MFM/FM
	PS0   -> x				FDD Write precompensation
	PS1   -> x				FDD Write precompensation

	HDLD    -> x			FDD Head Load
	US0     -> FDD.DRIVE_1	FDD Unit Select
	US1     -> x			FDD Unit Select
	HDSEL   -> FDD.HDSEL	Side select

	RW/SEEK -> misc. gates	read/write mode or seek mode
	FR/STEP -> FDD.STEP		Fault reset / step
							Beschaltung: seek mode: Step; read/write mode: immer false
	LC/DIR  -> FDD.DIR		Low Current / Dir	LC: Track ≥ 42; DIR: step in / step out
							Beschaltung: DIR in any mode
	FT/T0   <- FDD.TRACK0	read/write mode: FDD fault; seek mode: Track 0
							Beschaltung => seek mode: Track 0; read/write mode: immer false
	WP/TS   <- FDD.WR_PROT	Write protected / Two sides
							Beschaltung => WPROT in any mode

	IDX   <- FDD.INDEX		FDD sector index pulse
	RDY   <- FDD.READY		FDD ready to receive data

	WE    -> FDD.WR_GATE	FDD Write Enable
	WR_D  -> FDD.WR_DATA	FDD Write Data

	RD_D  <- SED9420.DATA	Read Data & Clock
	RDW   <- SED9420.WND	Read Data Window
	WCLK  <- SED9420.WCK
	CLK   <- SED9420.CK1	Input for SED's clock
	VCO   -> SED9420.DRQ	VCO Sync

Summary:

	no DMA			send_byte_to_dma() and read_byte_from_dma() are not reimplemented and do nothing
	no interrupts	raise_interrupt() and clear_interrupt() just set this.interrupt for Illegal Command handling.
	no FM			MFM is always assumed.

	mode:			RW		SEEK
	–––––––––––––––––––––––––––––––––
	FLT_RES/STEP	0		STEP	(fault reset always on)						TODO
	LC / DIR		DIR		DIR		(step direction statt low current signal)	EGAL
	FAULT/T0		0		TRACK0	(fault signal wird nicht ausgewertet)
	WPROT/2S		WPROT	WPROT	(2sides-signal wird ignoriert)
*/


static cstr i_addr = "001- ---- ---- --0-";
static cstr o_addr = "0011 ---- ---- --0-";


FdcPlus3::FdcPlus3(Machine* m) : Fdc765(m, isa_FdcPlus3, internal, o_addr, i_addr)
{
	attachDiskDrive(0, FloppyDiskDrive::newFloppyDiskDrive(m, Drive3, 1, 42, 0.006, 6250));
}

void FdcPlus3::input(Time t, int32 /*cc*/, uint16 addr, uint8& byte, uint8& mask)
{
	byte &= addr & 0x1000 ? readDataRegister(t) : readMainStatusRegister(t);
	mask = 0xff;
}

void FdcPlus3::output(Time t, int32 /*cc*/, uint16 /*addr*/, uint8 byte) { writeDataRegister(t, byte); }

void FdcPlus3::attachDiskDrive(uint n, std::shared_ptr<FloppyDiskDrive> dd)
{
	Fdc765::attachDiskDrive(n, dd);
	Fdc765::attachDiskDrive(n ^ 2, dd); // mirrored position
}

void FdcPlus3::removeDiskDrive(uint n)
{
	Fdc765::removeDiskDrive(n);
	Fdc765::removeDiskDrive(n ^ 2); // mirrored position
}
