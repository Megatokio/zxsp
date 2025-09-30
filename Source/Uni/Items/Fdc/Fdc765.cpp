// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#define LOGLEVEL 0
#include "Fdc765.h"
#include "FloppyDiskDrive.h"
#include "Machine/Machine.h"
DEBUG_INIT_MSG


enum Command {
	mRdTr	  = 1 << 2,	   // ReadTrack
	mSpecify  = 1 << 3,	   // Specify				no result phase
	mGetDrvSt = 1 << 4,	   // SenseDriveState		--> SR3
	mWrSec	  = 1 << 5,	   // WriteSector
	mRdSec	  = 1 << 6,	   // ReadSector
	mSeekTr0  = 1 << 7,	   // Recalibrate			no result phase
	mGetIntSt = 1 << 8,	   // SenseInterruptState	--> S0 TP
	mWrDelSec = 1 << 9,	   // WriteDeletedSector
	mRdSecId  = 1 << 0x0A, // ReadSectorId
	mRdDelSec = 1 << 0x0C, // ReadDeletedSector
	mFmtTr	  = 1 << 0x0D, // FormatTrack
	mSeekTr	  = 1 << 0x0F, // SeekTrack			no result phase
	mScanEQ	  = 1 << 0x11, // ScanEqual
	mScanLE	  = 1 << 0x19, // ScanLowOrEqual
	mScanGE	  = 1 << 0x1D, // ScanHighOrEqual
	mScanXX	  = mScanEQ | mScanGE | mScanLE,
	mIllCmd =
		~(mRdTr | mSpecify | mGetDrvSt | mWrSec | mRdSec | mSeekTr0 | mGetIntSt | mWrDelSec | mRdSecId | mRdDelSec |
		  mFmtTr | mSeekTr | mScanXX)
};

enum BitMask {
	// MSR:
	msrFdd0Busy = 1 << 0, // Drive0 Busy:		drive 0 is in seek mode
	msrFdd1Busy = 1 << 1, //		1
	msrFdd2Busy = 1 << 2, //		2
	msrFdd3Busy = 1 << 3, //		3
	msrFdcBusy =
		1 << 4, // Controller Busy: A Read or Write command is in process. FDC will not accept any other command.
				// Dieses Flag wird gesetzt, sobald das erste Byte eines neuen Befehles in's Datenregister geschrieben
				// wurde. Es wird erst mit dem Ende der Auswertungsphase wieder zurückgesetzt.
	msrEXM = 1 << 5,		  // Execution Mode:  Set in Exe Phase in Non-DMA-Mode
	msrDIO = 1 << 6,		  // Data direction:  0 = CPU->FDC
	msrRQM = 1 << 7,		  // Request for Master: FDC is ready to send or read data from CPU
	msrRQR = msrRQM | msrDIO, // request CPU Read
	msrRQW = msrRQM,		  // request CPU write

	// SR0:
	IC			   = 3 << 6, // Interrupt code:
	ICok		   = 0 << 6, // command finished
	ICaborted	   = 1 << 6, // command started, but was not successfully completed
	ICinvalidCmd   = 2 << 6, // invalid command
	ICdriveBusyChg = 3 << 6, // drive became not-ready during command
	SeekEnd		   = 1 << 5, // Seek End: Seek on drive D0D1 ended
	EquipmentCheck = 1 << 4, // Equipment check: Track0 not found or ErrorFF set
	NotReady	   = 1 << 3, // Not Ready: R/W command issued while SEEKin progress or Head1 selected for SS drive
	Head		   = 1 << 2, // Head
	Unit		   = 3 << 0, // Unit select

	// SR1:
	EndOfTrack = 1 << 7, // End of Track: the FDC tried to access a sector beyond the last Sector of a track
	DataError  = 1 << 5, // Data Error:	 CRC error in ID field or data field
	Overrun	   = 1 << 4, // Overrun
	NoData = 1 << 2, // No Data:	READ DATA, READ DELETED DATA, WRITE DATA, WRITE DELETED DATA or SCAN: the FDC cannot
					 // find the Sector specified in the IDR register
					 //			READ ID:		 the FDC cannot read the ID field without an error
					 //			READ DIAGNOSTIC: the starting sector cannot be found
	NotWriteable =
		1 << 1,			// Not writeable:	WRITE DATA, WRITE DELETED DATA or Write ID: Line WProt from FDD was activated
	MissingAM = 1 << 0, // Missing Address Mark:	 the FDC does not detect the IDAM before 2 index pulses.
						//							 or the FDC cannot find the DAM or DDAM after the IDAM is found, then
						//bit MD of ST2 is also set.

	// SR2:
	ControlMark = 1 << 6,	  // READ DATA or SCAN: the FDC encountered a Sector with a DDAM
							  // READ DELETED DATA: the FDC encountered a Sector with a DAM
	DataErrorInData = 1 << 5, // Data error in data field
	WrongTrack = 1 << 4,   // Wrong cylinder: the contents of Cylinder-field in sector on the medium is different from
						   // that stored in the IDR. note: SR1.ND also set.
	ScanEqualHit = 1 << 3, // Scan equal hit: SCAN commands: the condition of the "equal" is satisfied
	ScanFailed = 1 << 2, // Scan not satisfied: SCAN commands: the FDC cannot find a sector on the cylinder which meets
						 // the condition.		TODO: ???
	BadTrack = 1 << 1, // Bad Cylinder: when the contents of Cylinder field on the medium is 0xFF and is different from
					   // that stored in the IDR. note: SR1.ND also set.
	MissingDAM = 1 << 0, // Missing DAM: the FDC cannot find a Data Address Mark or Deleted Data Address Mark

	// SR3:
	FddError  = 1 << 7, // ErrorFF from FDD
	FddWprot  = 1 << 6, // WProt from FDD
	FddReady  = 1 << 5, // Ready from FDD
	FddTrack0 = 1 << 4, // Track0 from FDD
	Fdd2Sided = 1 << 3	// DoubleSided from FDD
						//	Head			= 1<<2	// selected head
						//	Unit			= 3<<0	// selected drive
};

static const uint8 DAM	= 0xFB; // data address mark
static const uint8 DDAM = 0xF8; // deleted data address mark
static const uint8 IDAM = 0xFE; // sector ID address mark


// -------------------------------------------------------------------------
//					creator & interface
// -------------------------------------------------------------------------


Fdc765::Fdc765(Machine* m, isa_id id, Internal internal, cstr o_addr, cstr i_addr) :
	Fdc(m, id, internal, o_addr, i_addr)
{
	_init();
}

void Fdc765::_init()
{
	sm_state = 0;
	MSR		 = msrRQW;
	time = when_head_unloaded = timeout = 0;
	SR0 = SR1 = SR2 = 0;
	headloadtime	= 0.008;
	headunloadtime	= 0.08;
	steprate		= 0.002;
	dma_disabled	= no;
	terminal_count	= no;
	memset(track, 0, sizeof(track));
}

void Fdc765::powerOn(/*t=0*/ int32 cc)
{
	_init();
	Fdc::powerOn(cc);
}

void Fdc765::reset(Time t, int32 cc)
{
	_init();
	Fdc::reset(t, cc);
}

uint8 Fdc765::readMainStatusRegister(Time t)
{
	run_statemachine(t);
	time = t;
	return MSR;
}

uint8 Fdc765::readDataRegister(Time t)
{
	run_statemachine(t);
	time = t;
	if ((MSR & (msrRQM | msrDIO)) == msrRQR)
	{
		clear_interrupt();
		MSR ^= msrRQR;
		xxlog("<<%02X ", uint(byte));
		return byte;
	}
	else xlogline("FDC: byte from FDC read, but there was no byte available");
	return 0xff; // hm hm ... input() mask wird gesetzt ...
}

void Fdc765::writeDataRegister(Time t, uint8 b)
{
	run_statemachine(t);
	time = t;
	if ((MSR & (msrRQM | msrDIO)) == msrRQW)
	{
		clear_interrupt();
		MSR ^= msrRQW;
		byte = b;
		xxlog(">>%02X ", uint(b));
	}
	else xlogline("FDC: byte to FDC lost: %i", int(b));
}

void Fdc765::audioBufferEnd(Time t)
{
	if (t > time)
	{
		run_statemachine(t);
		time = t;
	}
	when_head_unloaded -= t;
	timeout -= t;
	time -= t;
	Fdc::audioBufferEnd(t);
}


// -------------------------------------------------------------------------
//							STATE MACHINE
// -------------------------------------------------------------------------


// helper
void Fdc765::update_crc()
{
	uint byte = this->byte << 8;
	uint crc  = this->crc;
	for (uint i = 8; i--; byte <<= 1) { crc = (byte ^ crc) & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1; }
	this->crc = crc;
}

// helper
bool Fdc765::scan_satisfied() { return CMD == mScanEQ ? eq : CMD == mScanGE ? ge : CMD == mScanLE ? le : no; }

// helper
uint8 Fdc765::SR3()
{
	uint8 SR3 = head_unit;			   // b012 = selected drive and head
	if (is_fault()) SR3 |= FddError;   // b7 = Fehler-FF
	if (is_wprot()) SR3 |= FddWprot;   // b6 = write protected
	if (is_ready()) SR3 |= FddReady;   // b5 = drive ready
	if (is_track0()) SR3 |= FddTrack0; // b4 = Track 0
	if (is_2sided()) SR3 |= Fdd2Sided; // b3 = double sided
	return SR3;
}


//	Tricky co-routine macros:
#define BEGIN       \
  switch (sm_state) \
  {                 \
  default: IERR();  \
  case 0:

#define WAIT             \
  do {                   \
	sm_state = __LINE__; \
	return;              \
  case __LINE__:;        \
  }                      \
  while (0)

#define FINISH }

#define GOSUB(PROC)               \
  do {                            \
	sm_stack[sm_sp++] = __LINE__; \
	goto PROC;                    \
  case __LINE__:;                 \
  }                               \
  while (0)

#define RETURN goto ret

#define send_a_byte_to_cpu(BYTE) \
  do {                           \
	MSR	 = msrFdcBusy | msrRQR;  \
	byte = BYTE;                 \
	while (MSR & msrRQM) WAIT;   \
  }                              \
  while (0)

#define read_a_byte_from_cpu(BYTE) \
  do {                             \
	MSR = msrFdcBusy | msrRQW;     \
	while (MSR & msrRQM) WAIT;     \
	BYTE = byte;                   \
  }                                \
  while (0)

#define read_byte_from_cpu()   \
  do {                         \
	MSR = msrFdcBusy | msrRQW; \
	while (MSR & msrRQM) WAIT; \
  }                            \
  while (0)


void Fdc765::run_statemachine(Time t)
{
aa:
	BEGIN;

	head_unit = 0b000;
	SR0		  = 0x00;
	head	  = 0;
	unit	  = 0;
	drive	  = fdd[0];
	ready	  = no;

	for (;;)
	{
		sm_sp  = 0;	 // reset stack			(if prev. command was aborted)
		crc_on = no; // disable crc update	(if prev. command was aborted)

		// Get Command:

		MSR = msrRQW;
		while (MSR)
		{
			if (ready != drive->is_ready)
			{
				ready ^= 1;
				SR0 |= ICdriveBusyChg | head_unit;
				raise_interrupt();
			}
			WAIT;
		}

#if XLOG
		static cstr names[32] = {
			"IllCmd 00",  "IllCmd 01",	 "ReadTrack",	"Specify",		"GetDriveState", "WriteSector", "ReadSector",
			"SeekTrack0", "GetIntState", "WriteDelSec", "ReadSectorID", "IllCmd 0Bh",	 "ReadDelSec",	"FormatTrack",
			"IllCmd 0Eh", "SeekTrack",	 "IllCmd 10h",	"ScanEQ",		"IllCmd 12h",	 "IllCmd 13h",	"IllCmd 14h",
			"IllCmd 15h", "IllCmd 16h",	 "IllCmd 17h",	"IllCmd 18h",	"ScanLE",		 "IllCmd 1Ah",	"IllCmd 1Bh",
			"IllCmd 1Ch", "ScanGE",		 "IllCmd 1Eh",	"IllCmd 1Fh"};
		logline("FDC765: %s", names[byte & 0x1f]);
#endif

		multitrack = (byte >> 7) & 1;
		mfm		   = (byte >> 6) & 1;
		skip_bit   = (byte >> 5) & 1;
		CMD		   = 1 << (byte & 0x1F); // CMD = Mask!
		MSR		   = msrFdcBusy;

		// nach SEEK und RECALIBRATE, die mit einem Interrupt enden, muss SR0 abgefragt werden
		// andere Befehle werden mit "Illegal Command" beendet.
		//		if(interrupt)
		if (SR0)
		{
			assert((SR0 & SeekEnd) || (SR0 & IC) == ICdriveBusyChg);
			//			clear_interrupt();
			assert(!interrupt);

			if (CMD == mGetIntSt) // 08 -- S0 TP			// Statusregister 0 abfragen
			{
				xlogline("Fdc765: -> SR0 = %02x", uint(SR0));
				send_a_byte_to_cpu(SR0);
				SR0 = 0;
				send_a_byte_to_cpu(uint8(track[unit]));
			}
			else // IllCmd -- S0
			{
				xlogline("Fdc765: -> SR0 = Ill. Command (GetIntSt expected)");
				send_a_byte_to_cpu(ICinvalidCmd /*SR0*/);
				SR0 = 0;
			}
			continue;
		}

		// Get Most Common Arguments:

		// Get Drive+Head:
		if (CMD & (mRdTr | mWrSec | mRdSec | mWrDelSec | mRdDelSec | mScanXX | mGetDrvSt | mRdSecId | mSeekTr0 |
				   mSeekTr | mFmtTr))
		{
			read_byte_from_cpu();
			head_unit = byte & 7;
			head	  = head_unit >> 2;
			unit	  = head_unit & 3;
			drive	  = fdd[unit];
		}

		// Get TrackID, HeadID, 1st SectorID
		if (CMD & (mRdTr | mWrSec | mRdSec | mWrDelSec | mRdDelSec | mScanXX))
		{
			read_a_byte_from_cpu(track_id);
			read_a_byte_from_cpu(head_id);
			read_a_byte_from_cpu(sector_id);
			xlogline("Fdc765:    Hd=%u, Tr=%u, Sec=%u", uint(head_id), uint(track_id), uint(sector_id));
		}

		// Get Log2SectorSize, LastSectorID|NumSectors, Gap3Len, FillByte|SectorSize|ScanStep
		if (CMD & (mRdTr | mWrSec | mRdSec | mWrDelSec | mRdDelSec | mScanXX | mFmtTr))
		{
			read_a_byte_from_cpu(log2sectorsize);
			log2sectorsize &= 7;								// Sector size: log2(size)-7
			read_a_byte_from_cpu(last_sector_id = num_sectors); // last sector id "EOT" or number of sectors
			read_a_byte_from_cpu(gap3len);
			read_a_byte_from_cpu(sectorsize = fillbyte); // sector length if SZ=0 | FillByte | ScanStep
		}

		// Kommandos auswerten:

		SR0 = ICok;
		SR1 = 0x00;
		SR2 = 0x00;
		drive->update(t);

		if (CMD & (mIllCmd | mGetIntSt)) // xx -- S0		// note: S0 = 0x80 (angeblich. to be verified)
		{								 // note: +3DOS behaves as if unexpected GetIntSt are illCmds
			xlogline("Fdc765: -> SR0 = Ill. Command");
			send_a_byte_to_cpu(ICinvalidCmd /*SR0*/);
			continue;
		}

		if (CMD == mSpecify) // 03 XX YY -				// Laufwerksdaten festlegen (SPECIFY)
		{
			read_byte_from_cpu();
			headunloadtime = (byte & 0x0F) * 0.032;	   // b0…3: headunload = n*32ms
			steprate	   = (16 - byte / 16) * 0.002; // b4…7: steprate   = (16-n)*2ms

			read_byte_from_cpu();
			dma_disabled = byte & 1;			   // b0: DMA_disable
			headloadtime = (byte / 2 + 1) * 0.004; // b1…7: headload = (n+1)*4ms

			xlogline("Fdc765: -> head load time   = %.6g ms", headloadtime * 1000.0);
			xlogline("Fdc765: -> head unload time = %.6g ms", headunloadtime * 1000.0);
			xlogline("Fdc765: -> step time        = %.6g ms", steprate * 1000.0);
			continue;
		}

		if (CMD == mGetDrvSt) // 04 HU - S3				// SENSE DRIVE STATE
		{
			send_a_byte_to_cpu(SR3());
			continue;
		}

		if (CMD == mSeekTr) // 0F HU TP -				// Seek Track
		{					// TODO: multiple drives
			read_a_byte_from_cpu(requested_track[unit]);
			MSR		= 1 << unit; // FDDx busy
			timeout = t;
			xlogline("Fdc765:    Track %u", requested_track[unit]);

			while (track[unit] < requested_track[unit])
			{
				drive->step(timeout, +1);
				timeout += steprate;
				while (t < timeout) WAIT;
				drive->update(timeout);
				track[unit] += 1;
			}

			while (track[unit] > requested_track[unit])
			{
				drive->step(timeout, -1);
				timeout += steprate;
				while (t < timeout) WAIT;
				drive->update(timeout);
				track[unit] -= 1;
			}

			SR0 = ICok | SeekEnd | head_unit;
			raise_interrupt();
			continue;
		}

		if (CMD == mSeekTr0)	 // 07 HU -					// Spur 0 suchen
		{						 // TODO: multiple drives
			MSR		= 1 << unit; // FDDx busy
			timeout = t;

			for (n = 77; n && !is_track0(); n--)
			{
				drive->step(timeout, -1);
				timeout += steprate;
				while (t < timeout) WAIT;
				drive->update(timeout);
			}
			track[unit] = 0;

			SR0 = ICok | SeekEnd | head_unit;
			if (!is_track0())
			{
				SR0 |= EquipmentCheck;
				logline("EQUIPMENT CHECK!");
			}
			raise_interrupt();
			continue;
		}

		// Ab hier Kommandos, die eine Standard-Resultphase haben:

		if (!is_ready())
		{
			// The loss of a READY signal at the beginning of a command execution phase causes SR0 to be set to
			// ICaborted.
			xlogline("Fdc765: Drive not ready");
			SR0 = ICaborted | NotReady | head_unit;
		}
		else if ((head || multitrack) && !is_2sided())
		{
			xlogline("Fdc765: Side 2 on SS drive selected");
			SR0 = ICaborted | NotReady | head_unit;
		}
		else if ((CMD & (mWrSec | mWrDelSec | mFmtTr)) && is_wprot())
		{
			xlogline("Fdc765: write command to write-protected disk");
			SR0 = ICaborted | head_unit;
			SR1 = NotWriteable;
		}
		//		if((CMD&(mWrSec|mWrDelSec|mFmtTr)) && mfm && log2sectorsize==0)
		//		{
		//			write:  ssize won't match
		//			format: uses ssize from individual sector data		TODO: TO BE TESTED!
		//		}
		else
		{
			if (t >= when_head_unloaded)
			{
				xlogline("Fdc765: Waiting for head loaded");
				timeout = t + headloadtime;
				while (t < timeout) WAIT;
				when_head_unloaded = t + headunloadtime;
				drive->update(t);
			}

			SR0			   = ICok | head_unit;
			terminal_count = off;
			bytepos		   = drive->bytePosition();

			if (CMD & (mRdSec | mRdDelSec | mWrSec | mWrDelSec | mScanXX))
			{
				/*	READ_SECTORS		06+t+m+s	HU TR HD SC SZ LS GP SL  <R>  S0 S1 S2 TR HD LS SZ
					READ_DELETED_SEC:	0C+t+m+s	HU TR HD SC SZ LS GP SL  <R>  S0 S1 S2 TR HD LS SZ
					WRITE_SECTORS:		05+t+m		HU TR HD SC SZ LS GP SL  <W>  S0 S1 S2 TR HD LS SZ
					WRITE_DELETED_SEC:	09+t+m		HU TR HD SC SZ LS GP SL  <W>  S0 S1 S2 TR HD LS SZ
					SCAN_EQUAL:			11+t+m+s	HU TR HD SC SZ LS GP SL  <W>  S0 S1 S2 TR HD LS SZ
					SCAN_LOW_OR_EQUAL:	19+t+m+s	HU TR HD SC SZ LS GP SL  <W>  S0 S1 S2 TR HD LS SZ
					SCAN_HIGH_OR_EQUAL:	1D+t+m+s	HU TR HD SC SZ LS GP SL	 <W>  S0 S1 S2 TR HD LS SZ
				*/

				step = CMD & mScanXX ? ((sectorsize - 1) & 1) + 1 : 1;

				for (;;) // schleife über seiten
				{
					for (;;) // schleife über sektorn
					{
						GOSUB(io_sector);
						if ((SR0 & IC) != ICok || terminal_count || scan_satisfied()) break;
						sector_id += step;
						if (sector_id == last_sector_id + step) break;
					}

					if (!multitrack || head == 1 || (SR0 & IC) != ICok || terminal_count || scan_satisfied()) break;

					head ^= 1;
					head_id ^= 1;
					sector_id = 1;
				}

				if (sector_id == last_sector_id + step)
				{
					if (multitrack && head == 0)
					{
						head ^= 1;
						head_id ^= 1;
					}
					else { track_id += 1; }
					sector_id = 1;	   /*SR0 = ICaborted | head_unit;*/
					SR1 |= EndOfTrack; // ICaborted funktioniert nicht mit +3DOS
				}

				if ((SR0 & IC) == ICok && (CMD & mScanXX))
				{
					if (eq) SR2 |= ScanEqualHit;
					else if (!scan_satisfied()) SR2 |= ScanFailed;
				}
			}
			else if (CMD == mRdTr) // READ_TRACK:		02+m+s HU TR HD ?? SZ NM GP SL  <R>  S0 S1 S2 TR HD NM SZ
			{
				SR1 |= NoData;

				GOSUB(wait_for_indexpulse);
				timeout += drive->timePerByte(); // when_timeout for 1st byte from drive
				bytepos		= 0;
				indexpulses = 0;
				idams		= 0;

				while (num_sectors && !indexpulses)
				{
					// GAP3:
					for (n = 0; !indexpulses; n++)
					{
						GOSUB(copy_byte_from_fdd_to_host);
						if (byte != 0x4E) break;
					}
					if (n < gap3len) continue; // gap too short

					// VCO SYNC:
					if (byte != 0x00) continue;
					for (n = 1; !indexpulses; n++)
					{
						GOSUB(copy_byte_from_fdd_to_host);
						if (byte != 0x00) break;
					}
					if (n < 10) continue; // SYNC too short

					// IDAM:
					if (byte != 0xA1) continue;
					start_crc();
					update_crc();
					for (n = 1; !indexpulses; n++) // IDAM byte 1=0xA1, 2=0xA1, 3=0xA1, 4=0xFE
					{
						GOSUB(copy_byte_from_fdd_to_host);
						update_crc();
						if (byte != 0xA1) break;
					}
					if (n != 4 || byte != 0xFE)
					{
						cancel_crc();
						continue;
					} // not an IDAM or corrupted or terminated by 2nd sync pulse

					// Sector Info:
					GOSUB(copy_byte_from_fdd_to_host);
					f = byte == track_id;
					GOSUB(copy_byte_from_fdd_to_host);
					f &= byte == head_id;
					GOSUB(copy_byte_from_fdd_to_host);
					f &= byte == sector_id;
					GOSUB(copy_byte_from_fdd_to_host);
					f &= byte == log2sectorsize;
					crc_on = false;
					GOSUB(copy_byte_from_fdd_to_host);
					if (byte != (crc >> 8)) continue; // crc error in IDAM
					GOSUB(copy_byte_from_fdd_to_host);
					if (byte != uint8(crc)) continue; // crc error in IDAM

					// valid sector IDAM found: inspect and count it:
					num_sectors--;
					idams++;
					if (f) SR1 &= ~NoData;
				}

				if (num_sectors == 0) // Maximalzahl Sektoren gelesen:
				{
					//					SR0 |= ICaborted;
					SR1 |= EndOfTrack;
				}

				if (!idams) // no sector id found:
				{
					SR0 |= ICaborted;
					SR1 |= MissingAM;
				}
			}
			else if (CMD == mRdSecId) // READ_SECTOR_ID:	0A+m HU -- S0 S1 S2 TR HD LS SZ
			{
				gap3len = 20;
				GOSUB(read_sector_id);
			}
			else if (CMD == mFmtTr) // FORMAT_TRACK:	0D+m   HU SZ NM GP FB <W> S0 S1 S2 TR HD LS SZ
			{
				GOSUB(wait_for_indexpulse);
				bytepos		= 0;
				indexpulses = 0;

				// IAM (Index Address Mark):

				n = 80;
				GOSUB(write_gap);	   // GAP4A
				GOSUB(write_vco_sync); // VCO SYNC
				byte = 0xC2;
				for (n = 0; n < 3; n++) { GOSUB(write_byte_to_fdd); } // IAM
				byte = 0xFC;
				GOSUB(write_byte_to_fdd); // IAM

				n = 50;								// Gap1 len
				while (num_sectors && !indexpulses) // sectors
				{
					// Sector IDAM (ID Address Mark):

					GOSUB(write_gap);
					if (indexpulses) break; // GAP1 o. Gap3
					GOSUB(write_vco_sync);
					if (indexpulses) break; // VCO SYNC
					start_crc();
					byte = 0xA1;
					for (n = 0; n < 3; n++) { GOSUB(write_byte_to_fdd); } // IDAM
					byte = IDAM;
					GOSUB(write_byte_to_fdd); // IDAM

					timeout = t - drive->timeSinceIndex() + (bytepos + 1) * drive->timePerByte();
					GOSUB(copy_byte_from_host_to_fdd);
					track_id = byte;
					GOSUB(copy_byte_from_host_to_fdd);
					head_id = byte;
					GOSUB(copy_byte_from_host_to_fdd);
					sector_id = byte;
					GOSUB(copy_byte_from_host_to_fdd);
					log2sectorsize = byte;
					if (mfm && byte == 0) log2sectorsize = 1;
					GOSUB(write_crc);
					if (indexpulses) break;

					// Sector Data:

					n = 22;
					GOSUB(write_gap);
					if (indexpulses) break; // GAP2
					GOSUB(write_vco_sync);
					if (indexpulses) break; // VCO SYNC
					start_crc();
					byte = 0xA1;
					for (n = 0; n < 3; n++) { GOSUB(write_byte_to_fdd); } // DAM
					byte = DAM;
					GOSUB(write_byte_to_fdd); // DAM

					byte = fillbyte;
					n	 = 0x80 << log2sectorsize;
					while (n-- && !indexpulses) { GOSUB(write_byte_to_fdd); } // Sector Data
					if (!indexpulses)
					{
						GOSUB(write_crc);
						sector_id++;
						num_sectors--;
					}

					n = gap3len; // Gap3 len
				}

				byte = 0x4E;
				while (!indexpulses) { GOSUB(write_byte_to_fdd); } // Gap4B up to track end
			}
			else IERR();
		}

		// standard result phase:
	std_result_phase:
		when_head_unloaded = t + headunloadtime;

#ifdef XXLOG
		if ((SR0 & IC) != ICok) { logline("FDC765: -> SR0 = %02Xh", uint(SR0)); }
		if (SR1) { logline("FDC765: -> SR1 = %02Xh", uint(SR1)); }
		if (SR2) { logline("FDC765: -> SR1 = %02Xh", uint(SR2)); }
		logline("FDC765: -> h=%i, t=%i, s=%i, sz=%i", int(head_id), int(track_id), int(sector_id), int(log2sectorsize));
#endif

		raise_interrupt();
		send_a_byte_to_cpu(SR0);
		SR0 = 0;
		send_a_byte_to_cpu(SR1);
		send_a_byte_to_cpu(SR2);
		send_a_byte_to_cpu(track_id);
		send_a_byte_to_cpu(head_id);
		send_a_byte_to_cpu(sector_id);
		send_a_byte_to_cpu(log2sectorsize);
	}

	IERR(); // security only. dead code


/*	READ_SECTOR_ID:	0A+m HU -- S0 S1 S2 TR HD LS SZ

	RdSecId:  lese die nächste Sector-ID von der Floppy
	Rd/WrSec: lese solange Sector-IDs bis die gesuchte gefunden wurd

	Abort on Error (SR0=ICaborted) wenn
		- index_pulses ≥ 2
		- wrong track (TrackID from CPU != TrackID from FDD)
		- crc error in ID (außer CMD==mRdSecID)
	Return Ok (SR0=ICok) wenn
		- Sektor-ID korrekt gelesen und
			- TrackID, HeadID, SectorID und Log2SectorSize übereinstimmen
			- oder CMD==mRdSecId
*/
read_sector_id: // ( -- SR0 )
{
	for (indexpulses = 0; indexpulses < 2;)
	{
		crc_on = no;
		GOSUB(read_gap); // GAP3
		if (byte != 0x00 || n < gap3len || indexpulses >= 2) continue;

		GOSUB(read_vco_sync);
		n++; // VCO SYNC
		if (byte != 0xA1 || n < 10 || indexpulses >= 2) continue;

		start_crc();
		update_crc(); // IDAM  =  0xA1, 0xA1, 0xA1, 0xFE
		GOSUB(read_byte_from_fdd);
		if (byte != 0xA1) continue;
		GOSUB(read_byte_from_fdd);
		if (byte != 0xA1) continue;
		GOSUB(read_byte_from_fdd);
		if (byte != 0xFE) continue;

		GOSUB(read_byte_from_fdd);
		if (CMD == mRdSecId) track_id = byte;
		if (byte != track_id)
		{
			SR0 |= ICaborted;
			SR1 |= NoData;
			SR2 |= WrongTrack;
			if (byte == 0xff) SR2 |= BadTrack;
			goto std_result_phase; // abort command
		}
		GOSUB(read_byte_from_fdd);
		if (CMD == mRdSecId) head_id = byte;
		f = byte == head_id;
		GOSUB(read_byte_from_fdd);
		if (CMD == mRdSecId) sector_id = byte;
		f &= byte == sector_id;
		GOSUB(read_byte_from_fdd);
		if (CMD == mRdSecId) log2sectorsize = byte;
		f &= byte == log2sectorsize || (log2sectorsize == 0 && (CMD & (mRdSec | mRdDelSec)));
		actual_log2ssize = byte;
		//		crc_on = no;
		GOSUB(read_byte_from_fdd); // n = byte<<8;
		GOSUB(read_byte_from_fdd); // n += byte;
		crc_on = no;
		if (crc) // crc error
		{
			if (CMD == mRdSecId) continue;
			SR0 |= ICaborted;
			SR1 |= DataError;
			goto std_result_phase; // abort command
		}
		if (f) RETURN; // crc stimmt => raus, wenn es der gesuchte Sektor ist
	}

	// IDAM not found:
	SR0 |= ICaborted;
	//	SR1 |= NoData;			// <- +3 hängt dann ewig und bricht mit error "no data" ab
	SR1 |= MissingAM;
	goto std_result_phase; // abort command
}


/*	read or write sector as defined by TrackID .. Log2SectorSize
 */
io_sector: // ( -- )
{
	if (CMD & (mRdSec | mRdDelSec | mScanXX)) // Read Sector Data:
	{
		GOSUB(read_sector_id);

		GOSUB(read_gap); // 22 * 0x4E	GAP2
		if (byte != 0x00 || n < 10) goto iox;

		GOSUB(read_vco_sync);
		n++; // 12 * 0x00	VCO SYNC
		if (byte != 0xA1 || n < 10) goto iox;

		if (terminal_count) RETURN;

		start_crc();
		update_crc();
		GOSUB(read_byte_from_fdd);
		if (byte != 0xA1) goto iox;
		GOSUB(read_byte_from_fdd);
		if (byte != 0xA1) goto iox;
		GOSUB(read_byte_from_fdd);

		if (byte != DAM && byte != DDAM)
		{
		iox:
			SR0 |= ICaborted;
			SR1 |= NoData;
			SR1 |= MissingAM;
			SR2 |= MissingDAM;
			goto std_result_phase; // abort
		}

		skip  = (byte == DDAM) != (CMD == mRdDelSec);
		ssize = 0x80u << actual_log2ssize; // sector size
		size  = skip ? 0 : log2sectorsize || (CMD & mScanXX) ? ssize : min(ssize, uint(sectorsize));

		// read and send data bytes:
		timeout = t - drive->timeSinceIndex() + (bytepos + 1) * drive->timePerByte();
		eq = le = ge = true;
		for (n = 0; n < size; n++)
		{
			if (CMD & mScanXX) GOSUB(compare_byte_from_host_and_fdd);
			else GOSUB(copy_byte_from_fdd_to_host);
		}

		// read and skip unwanted data bytes:
		for (; n < ssize; n++) { GOSUB(read_byte_from_fdd); }

		// read and test crc:
		crc_on = no;
		GOSUB(read_byte_from_fdd);
		n = byte << 8;
		GOSUB(read_byte_from_fdd);
		n += byte;
		if (n != crc) // crc error
		{
			SR0 |= ICaborted;
			SR1 |= DataError;
			SR2 |= DataErrorInData;
			goto std_result_phase; // abort
		}

		// exit if wrong DAM / DDAM:
		if (skip && !skip_bit)
		{
			SR0 |= ICaborted;
			SR2 |= ControlMark;
			goto std_result_phase; // abort
		}
	}
	else // write Sector Data:
	{
		GOSUB(read_sector_id);

		if (terminal_count) RETURN;

		n = 22;
		GOSUB(write_gap);	   // Gap2
		GOSUB(write_vco_sync); // VCO SYNC

		start_crc();
		byte = 0xA1;
		GOSUB(write_byte_to_fdd);
		GOSUB(write_byte_to_fdd);
		GOSUB(write_byte_to_fdd);
		byte = CMD == mWrDelSec ? DDAM : DAM;
		GOSUB(write_byte_to_fdd);

		// get and write data bytes:
		n = 0x80 << log2sectorsize; // sector size

		timeout = t - drive->timeSinceIndex() + (bytepos + 1) * drive->timePerByte();
		while (n)
		{
			GOSUB(copy_byte_from_host_to_fdd);
			n--;
		}

		// write dummy bytes if aborted due to terminal_count:
		byte = 0x00;
		while (n)
		{
			GOSUB(write_byte_to_fdd);
			n--;
		}

		// write crc:
		GOSUB(write_crc);
	}
	RETURN;
}


	// -------------------------------------------------------------------------------------
	// FDD, CPU and DMA read and write procs:

copy_byte_from_host_to_fdd:
	GOSUB(read_byte_from_host);
	if (t > timeout)
	{
		SR0 |= ICaborted;
		SR1 |= Overrun;
		goto std_result_phase;
	}
	timeout += drive->timePerByte();
	goto write_byte_to_fdd;

compare_byte_from_host_and_fdd:
	GOSUB(read_byte_from_host);
	if (t > timeout)
	{
		SR0 |= ICaborted;
		SR1 |= Overrun;
		goto std_result_phase;
	}
	timeout += drive->timePerByte();
	z = byte;
	GOSUB(read_byte_from_fdd);
	eq = eq && (z == byte);
	le = le && (z <= byte);
	ge = ge && (z >= byte);
	RETURN;

copy_byte_from_fdd_to_host:
	if (t > timeout)
	{
		SR0 |= ICaborted;
		SR1 |= Overrun;
		goto std_result_phase;
	}
	timeout += drive->timePerByte();
	GOSUB(read_byte_from_fdd);
	goto send_byte_to_host;

write_byte_to_fdd: // ( byte -- )
	drive->writeByte(head, bytepos, byte);
	goto wait_fdd;
	//	while(drive->bytePosition(t)==bytepos) WAIT;
	//	if(++bytepos>=drive->bytesPerTrack()) { indexpulses++; bytepos=0; }
	//	if(crc_on) update_crc();
	//	RETURN;

read_byte_from_fdd: // ( -- byte )
	byte = drive->readByte(head, bytepos);
wait_fdd:
	while (drive->bytePosition(t) == bytepos) WAIT;
	if (++bytepos >= drive->bytesPerTrack())
	{
		indexpulses++;
		bytepos = 0;
	}
	if (crc_on) update_crc();
	RETURN;

read_byte_from_host: // ( -- byte )
	if (dma_disabled)
	{
		MSR = msrFdcBusy | msrEXM | msrRQW;
		raise_interrupt();
		while (MSR & msrRQM) WAIT;
	}
	else { byte = read_byte_from_dma(); }
	RETURN;

send_byte_to_host: // ( byte -- )
	if (dma_disabled)
	{
		MSR = msrFdcBusy | msrEXM | msrRQR;
		raise_interrupt();
		while (MSR & msrRQM) WAIT;
	}
	else { send_byte_to_dma(byte); }
	RETURN;


	// convenience write procs:

write_vco_sync: // ( -- )
	for (byte = n = 0; n < 12; n++) { GOSUB(write_byte_to_fdd); }
	RETURN;

write_gap: // ( n -- )
	for (byte = 0x4E; n--;) { GOSUB(write_byte_to_fdd); }
	RETURN;

write_crc: // ( crc -- )
	crc_on = no;
	byte   = crc >> 8;
	GOSUB(write_byte_to_fdd);
	byte = crc;
	GOSUB(write_byte_to_fdd);
	RETURN;


// convenience read procs:

/*	Read and skip Gap bytes
	return:	n = number of Gap bytes skipped
			byte = first Non-Gap byte
*/
read_gap: // ( -- byte n )
	n = -1;
	do {
		n++;
		GOSUB(read_byte_from_fdd);
	}
	while (byte == 0x4E && indexpulses < 2);
	RETURN;

/*	Read and skip VCO Sync bytes
	RETURN:	n = number of Sync bytes skipped
			byte = first Non-Sync byte
*/
read_vco_sync: // ( -- byte n )
	n = -1;
	do {
		n++;
		GOSUB(read_byte_from_fdd);
	}
	while (byte == 0x00 && indexpulses < 2);
	RETURN;

wait_for_indexpulse:
	timeout = t - drive->timeSinceIndex() + drive->timePerTrack();
	while (t < timeout) { WAIT; }
	drive->update(t);
	RETURN;


/* state machine's 'RETURN' handler:
 */
ret:
	sm_state = sm_stack[--sm_sp];
	goto aa;

	FINISH;
}


/*	do a best-effort initialization for use in snapshots
 */
void Fdc765::initForSnapshot(int32 cc)
{
	powerOn(cc);

	dma_disabled   = yes;
	headloadtime   = 0.008;
	headunloadtime = 0.48;
	steprate	   = 0.012;

	for (uint i = 0; i < NELEM(fdd); i++) { track[i] = fdd[i]->track; }

	//	motor_on=0;				init()
	//	interrupt=0;			init()
	//	sm_state=0;				init()
	//	time=0;					init()
	//	SR1 = SR2	= 0;		init()
	//	terminal_count = false;	init()
	//	when_head_unloaded = 0;	init()
	//	timeout = time;			init()
	//	memset(track,0);		init()

	//	head_unit = 0b000;		run_statemachine.BEGIN
	//	SR0      = 0x00;		run_statemachine.BEGIN
	//	head	 = 0;			run_statemachine.BEGIN
	//	unit	 = 0;			run_statemachine.BEGIN
	//	drive	 = fdd[0];		run_statemachine.BEGIN
	//	ready	 = no;			run_statemachine.BEGIN
	//	sm_sp = 0;				run_statemachine.for(;;)
	//	crc_on = no;			run_statemachine.for(;;)
	//	MSR = msrRQW;			run_statemachine.for(;;)

	//	CMD		   = 1 << (byte & 0x1F);	next command
	//	multitrack = (byte>>7) & 1;			next command
	//	mfm        = (byte>>6) & 1;			next command
	//	skip_bit   = (byte>>5) & 1;			next command

	// FDC command and arguments:
	//	track_id=head_id=sector_id=last_sector_id=fillbyte=0;
	//	sectorsize = log2sectorsize = 512>>7;
	//	gap3len = 50;
	//	num_sectors = 1;
	//	step = 1;
	//	memset(requested_track,0,sizeof(requested_track));

	// misc. state machine variables:
	//	uint	idams, bytepos, n, z, size, ssize, indexpulses;
	//	uint16	crc;
	//	uint8	byte, actual_log2ssize; // byte_for_cpuxxxxx, byte_from_cpuxxxxx;
	//	bool	f,eq,le,ge,skip;
}
