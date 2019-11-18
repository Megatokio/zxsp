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

#include "globals.h"
#include "FloppyDisk.h"
#include "Qt/qt_util.h"
#include "unix/files.h"
#include "kio/peekpoke.h"

static const uint8 DAM  = 0xFB;		// data address mark
static const uint8 DDAM = 0xF8;		// deleted data address mark
static const uint8 IDAM = 0xFE;		// sector ID address mark



/*	create an unformatted new disk:
*/
FloppyDisk::FloppyDisk(uint bytes_per_track)
:
	bytes_per_track(bytes_per_track),
	writeprotected(no),
	modified(no),
	filepath(NULL)
{}


/*	create a formatted new disk:
*/
FloppyDisk::FloppyDisk(uint sides, uint tracks, uint sectors, bool interleaved, uint bytes_per_track)
:
	bytes_per_track(bytes_per_track),
	writeprotected(no),
	modified(no),
	filepath(NULL)
{
	DiskFormatInfo df;
	df.sides    = sides;
	df.tracks   = tracks;
	df.sectors  = sectors;
	df.side0    = 0;
	df.track0   = 0;
	df.sector0  = 1;
	df.sector_sz = 2;	// 512 = 0x80 << 2
	df.databyte = 0xE5;
	df.gap4a    = 80;
	df.gap3     = 54;
	df.interleaved = interleaved;

	formatDisk(df);
	assert(modified);
}


/*	create disk with data from file:
*/
FloppyDisk::FloppyDisk(cstr fpath)
:
	bytes_per_track(6250),
	modified(no),
	filepath(newcopy(fullpath(fpath)))
{
	cstr err;

	try
	{
		FD fd(filepath,'r');
		uint32 sz = fd.file_size();
		uint8* bu = new uint8[sz];
		fd.read_bytes(bu,sz);
		writeprotected = !fd.is_writable();
		fd.close_file(0);

		if(startswith(cstr(bu),"MV - CPC"))      err = read_dsk_file(bu,sz);
		else if(startswith(cstr(bu),"EXTENDED")) err = read_extended_dsk_file(bu,sz);
		else err = "The disc file format is not recognized";
		if(!err) return;
	}
	catch(std::exception& e)
	{
		err = catstr("Failed to read disc file: ",e.what());
	}

	delete[] filepath; filepath = NULL;
	writeprotected = yes;
	showWarning(err);
}


/*	Destructor
*/
FloppyDisk::~FloppyDisk()
{
	if(modified) saveDisk();
	delete[] filepath;
}

void FloppyDisk::saveAs(cstr path) throws
{
	if(path!=filepath) { delete[] filepath; filepath = newcopy(path); }
	saveDisk();
}

void FloppyDisk::saveDisk() throws
{
	if(!filepath) { showAlert("The disc could not be saved because there was no filename assigned with it"); return; }

	FD fd;
	try
	{
		fd.open_file_w(filepath);
		write_extended_disk_file(fd);
		fd.close_file(1);
		modified = no;
	}
	catch(file_error& e)
	{
		showAlert(e.what());
	}
}

bool FloppyDisk::fileIsWritable()
{
	if(!is_file(filepath,1)) return false;
	return is_writable(filepath,1);
}

int FloppyDisk::setWriteProtected(bool wp)
{
	writeprotected = wp;
	if(!filepath) return ok;
	int err = set_file_writable(filepath, wp ? NOBODY : OWNER|GROUP);
	if(!err) return ok;
	writeprotected = !fileIsWritable();
	return err;
}


void FloppyDisk::setFilepath(cstr fpath)
{
	delete[] filepath;
	fpath = fullpath(fpath);
	filepath = newcopy(fpath);
	try { create_file(fpath); }		// touch
	catch(file_error&) {}
	writeprotected = !fileIsWritable();
}


uint8 FloppyDisk::readByte(uint head, uint track, uint bytepos)
{
	uint8* t = getTrack(head,track);
	return t ? t[bytepos] : 0xff;
}

void FloppyDisk::writeByte(uint head, uint track, uint bytepos, uint8 byte)
{
	uint8*& t = getTrack(head,track);
	if(!t)
	{
		t = new uint8[bytes_per_track];
		memset(t,0xff,bytes_per_track);
	}
	t[bytepos] = byte;
	modified = true;
}

void FloppyDisk::makeSingleSided()
{
	while(sides[1].count()) { delete[] sides[1].last(); sides[1].drop(); }
}

void FloppyDisk::truncateTracks(uint n)
{
	while(sides[0].count()>n) { delete[] sides[0].last(); sides[0].drop(); }
	while(sides[1].count()>n) { delete[] sides[1].last(); sides[1].drop(); }
}


// ----------------------------------------------------------------------------------------------

/*	CRC computation for IBM Sytem 34 compatible Floppy Disks (MFM):

from "The floppy user guide"
by Michael Haardt (michael@moria.de) Alain Knaff (alain@linux.lu) David C. Niemi (niemi@tux.org)

	The generator used here is the CCITT polynomial g(x)=1+x5+x12+x16.
	To understand hardware calculation of the CRC value, view the data as a bit stream fed
	into a cyclic 16 bit shift register. At each step, the input bit is xor’ed with bit 15,
	and this result is fed back into various places of the register. Bit 5 gets bit 4 xor feedback,
	bit 12 gets bit 11 xor feedback and bit 0 gets feedback. All other bits simply get rotated
	e.g. bit 1 gets bit 0 on a clock edge. At the beginning, all flip flops of the register
	are set to 1. After the last data bit is processed that way, the register it contains the CRC.
	For checking CRCs, you do the same, but the last you feed to it is the CRC.
	If all is fine, then all bits will be 0 after.
	Since bytes are recorded with their highest bit first on floppies, they are also processed
	by the CRC register that way and the resulting CRC will be written with bit 15 being the first
	and bit 0 being the last to the floppy (big endian).
	The CRC is processed beginning with the first 0xa1 byte which presets the CRC register,
	so the CRC of a typical ID record would be computed as CRC of
		0xa1, 0xa1, 0xa1, 0xfe, 0x0, 0x0, 0x3, 0x2
	and have the value 0xac0d.
	0xac will be the first CRC byte and 0x0d the second.
*/
uint16 crc16( const uint8*  q, uint count )			// kio 2013-08-19
{
	uint crc = 0xffff;

	while(count--)
	{
		uint byte = uint(*q++) << 8;

		for(int i=8; i--;)
		{
			crc = (byte^crc) & 0x8000u ? (crc<<1) ^ 0x1021u : crc<<1;
			byte <<= 1;
		}
	}

	return uint16(crc);
}

// Checker:
static struct TestCrc
{
	TestCrc()
	{
		static uint8 bu[] = { 0xa1, 0xa1, 0xa1, 0xfe, 0x00, 0x00, 0x03, 0x02 };
		uint16 crc = crc16(bu,NELEM(bu));
		assert(crc==0xac0d);
		static uint8 bu2[] = { 0xa1, 0xa1, 0xa1, 0xfe, 0x00, 0x00, 0x03, 0x02, 0xac, 0x0d };
		crc = crc16(bu2,NELEM(bu2));
		assert(crc==0);
	}
} test_crc_dummy;

// ----------------------------------------------------------------------------------------------



/*	Aufbau eines Tracks for IBM Sytem 34 compatible Floppy Disks (MFM):

	Die Aufzeichnung startet 92 Bytes nach Beginn des Index-Pulses. (TMS279X)

	CNT	BYTE	NAME
	-------------------------------------------
	80	4E		GAP4A
	12	00		SYNC
	3	C2		IAM  Byte 0…2
	1	FC		IAM  Byte 3		Index mark

	50	4E		GAP1
	12	00		SYNC
	3	A1		IDAM Byte 0…2

s	1	FE		IDAM Byte 3		ID address mark
s	1	TRACK_ID
s	1	SIDE_ID
s	1	SECTOR_ID
s	1	SECTOR_LENGTH			log2(length_of_data) − 7
s	2	CRC
s
s	22	4E		GAP2			ID gap
s	12	00		SYNC
s	3	A1		DAM  Byte 0…2
s	1	FB/F8	DAM  Byte 3		Data address mark (F8 = delete DAM)
s	nn	DATA
s	2	CRC
s
s	54	4E		GAP3			data gap or format gap; count programmable
s	12	00						no GAP3 after the last sector: then GAP4B follows immediately
s	3	A1

	652	4E		GAP4B			bis Indexsignal aktiv
*/

uint8* FloppyDisk::writeSector(SectorFormatInfo& f, uint8* p)
{
// Gap1 or Gap3 and ID Address Mark (IDAM):
	memset(p,0x4E,f.gap3); p+=f.gap3;	// 54*	4E		GAP1=50 (fixed size); GAP3=54 (DOS)
	memset(p,0x00,12);     p+=12;		// 12*	00
	memset(p,0xA1,3);      p+=3;		// 3*	A1
	*p++ = IDAM;						//		FE		IDAM  Byte 3	ID address mark
	if(f.special&sf_idam_notfound) *(p-1) += 1;

// ID data and crc:
	*p++ = f.track_id;					//		Track
	*p++ = f.side_id;					//		Head
	*p++ = f.sector_id;					//		Sector No.
	*p++ = f.sector_sz;					//		Sector size:			log2(sz)-7
	poke2X(p,crc16(p-8,8)); p+=2;		// 2*	CRC
	if(f.special&sf_id_crc_wrong) *(p-1) += 1;

// Gap2 and Data Address Mark (DAM):
	memset(p,0x4E,22); p+=22;			// 22*	4E		GAP2			ID gap
	memset(p,0x00,12); p+=12;			// 12*	00		VCO SYNC
	memset(p,0xA1,3);  p+=3;			// 3*	A1		DAM  Byte 0…2
	*p++ = f.special&sf_ddam?DDAM:DAM;	//		FB		DAM  Byte 3
	if(f.special&sf_dam_notfound) *(p-1) += 1;

// Data and crc:
	uint n = 0x80u << f.sector_sz;		// TODO: handling für sector_sz=6 => n=0x2000
	if(f.data) memcpy(p,f.data,n);		//		DATA
	else memset(p,f.databyte,n);  p+=n;	//		DATA
	poke2X(p,crc16(p-(n+4),n+4)); p+=2;	// 2*	CRC
	if(f.special&sf_data_crc_wrong) *(p-1) += 1;

	return p;
}

void FloppyDisk::writeTrack(Array<SectorFormatInfo>& format, uint8*& track, uint gap4a)
{
	uint totalsize = gap4a+12+4;
	for(uint i=0;i<format.count();i++) totalsize += format[i].totalsize();
	if(totalsize > bytes_per_track) { logline("FloppyDisk.writeTrack: track data exceeds track size"); return; }

	if(!track) track = new uint8[bytes_per_track];

// Gap4A and Index Address Mark (IAM):
	uint8* p = track;
	memset(p,0x4E,gap4a); p+=gap4a;			// 80 *	4E		GAP4A
	memset(p,0x00,12); p+=12;				// 12 *	00		VCO SYNC
	memset(p,0xC2,3);  p+=3;				//  3 *	C2		IAM
	*p++ = 0xFC;							//	1 *	FC		IAM

// Sectors:
	for(uint i=0; i<format.count(); i++) { p = writeSector(format[i], p); }

// Gap4B:
	assert(p == track+totalsize);
	memset(p,bytes_per_track-totalsize,0x4E);	// GAP4B
}

void FloppyDisk::formatDisk(const DiskFormatInfo& df)
{
	assert(!writeprotected);

	truncateTracks(0);
	modified = true;

	uint sectors  = df.sectors;		// 8 .. 9
	uint sector0  = df.sector0;		// sector_id for sector 0

	SectorFormatInfo f;
	f.sector_sz   = uint8(df.sector_sz);
	f.gap3		  = uint8(df.gap3);
	f.databyte	  = uint8(df.databyte);
	f.special	  = 0;
	f.data		  = NULL;

	uint8 sids[sectors];
	if(df.interleaved)				// 5 1 6 2 7 3 8 4 9
	{
		uint m = sectors/2;
		for(uint i=0;i<m;  i++) sids[2*i+1] = uint8(sector0     + i);
		for(uint i=0;i<m+1;i++) sids[2*i]   = uint8(sector0 + m + i);
	}
	else
	{
		for(uint i=0;i<sectors;i++) sids[i] = uint8(sector0 + i);
	}

	for(uint h=0; h<df.sides; h++)
	for(uint t=0; t<df.tracks; t++)
	{
		f.side_id   = uint8(df.side0+h);
		f.track_id  = uint8(df.track0+t);

		Array<SectorFormatInfo> sf(sectors);
		for(uint i=0; i<sectors; i++)
		{
			f.sector_id = sids[i];
			sf[i] = f;
		}

		sf[0].gap3 = 50;
		writeTrack(sf,getTrack(h,t),df.gap4a);
	}
}


enum
{
// NEC µPD765 SR1:
	EndOfTrack		= 1<<7,	// End of Track: the FDC tried to access a sector beyond the programmed last Sector "EOT"
	DataError		= 1<<5,	// Data Error:	 CRC error in ID field or data field
	Overrun			= 1<<4,	// Overrun
	NoData			= 1<<2,	// No Data:	READ DATA, READ DELETED DATA, WRITE DATA, WRITE DELETED DATA or SCAN: the FDC cannot find the Sector specified in the IDR register
							//			READ ID:		 the FDC cannot read the ID field without an error
							//			READ DIAGNOSTIC: the starting sector cannot be found
	NotWriteable	= 1<<1,	// Not writeable:	WRITE DATA, WRITE DELETED DATA or Write ID: Line WProt from FDD was activated
	MissingAM		= 1<<0,	// Missing Address Mark:	 the FDC does not detect the IDAM before 2 index pulses.
							//							 or the FDC cannot find the DAM or DDAM after the IDAM is found, then bit MD of ST2 is also set.

// NEC µPD765 SR2:
	ControlMark		= 1<<6,	// READ DATA or SCAN: the FDC encountered a Sector with a DDAM
							// READ DELETED DATA: the FDC encountered a Sector with a DAM
	DataErrorInData	= 1<<5,	// Data error in data field
	WrongTrack		= 1<<4,	// Wrong cylinder: the contents of Cylinder-field in sector on the medium is different from that stored in the IDR. note: SR1.ND also set.
	ScanEqualHit	= 1<<3,	// Scan equal hit: SCAN commands: the condition of the "equal" is satisfied
	ScanFailed		= 1<<2,	// Scan not satisfied: SCAN commands: the FDC cannot find a sector on the cylinder which meets the condition.		TODO: ???
	BadTrack		= 1<<1,	// Bad Cylinder: when the contents of Cylinder field on the medium is 0xFF and is different from that stored in the IDR. note: SR1.ND also set.
	MissingDAM		= 1<<0	// Missing DAM: the FDC cannot find a Data Address Mark or Deleted Data Address Mark
};


cstr FloppyDisk::read_dsk_file(uint8* bu, uint32 sz)
{
	sides[0].purge();
	sides[1].purge();

	if(sz<0x34) return "disk file too short (len<0x34)";

/*	The DSK file starts with a 0x100 byte DISK INFORMATION BLOCK:

	00-21 	"MV - CPCEMU Disk-File\r\nDisk-Info\r\n" 	34 bytes
	22-2f 	name of creator								14
	30		number of tracks							1
	31		number of sides								1
	32-33 	size of a track (LSB first)				 	2
	34-ff 	filler (0)									204

	All tracks must be the same size.
	"size of track" includes the 0x100 byte Track Information Block.
	All tracks must have a "Track Information Block"

	Tracks start at 0x100.
	Tracks are ordered 0 .. number_of_tracks - 1
	Tracks on DS disks are interleaved: S0Tr0, S1Tr0, S0Tr1, S1Tr1, ...
*/
	uint num_tracks = bu[0x30];
	uint num_sides  = bu[0x31];
	uint track_size = bu[0x32] + 256 * bu[0x33];

	if(num_sides<1)		return "num sides = 0";
	if(num_sides>2)		return "num sides > 2";
	if(num_tracks>83)	return "num tracks > 83";
	if(num_sides * num_tracks * track_size + 0x100 > sz) return "disk file too short (data exceeds file size)";


	uint8* tp = bu + 256;				// tp -> track data
	for(uint t=0; t<num_tracks; t++)	// tracks
	for(uint s=0; s<num_sides; s++)		// sides
	{
		// Each track starts with a 0x100 byte TRACK INFORMATION BLOCK:
		// 00-0c 	"Track-Info\r\n"		13 bytes
		// 0d-0f 	unused					3
		// 10		track number			1
		// 11		side number				1
		// 12-13 	unused					2
		// 14	 	max. sector size		1
		// 15	 	number of sectors		1
		// 16	 	GAP#3 length			1
		// 17	 	filler byte				1
		// 18-xx 	Sector Information List xx	(space for up to 29 sectors)
		// xx-ff	filler (0)

		if(memcmp(tp,"Track-Info",10)!=0)    return "disk file corrupted (wrong track magic)";
		uint8 track = tp[0x10]; if(track!=t) return "disk file corrupted (wrong track)";
		uint8 side  = tp[0x11]; if(side!=s)  return "disk file corrupted (wrong side)";
		uint8 gap3  = tp[0x16]; if(gap3<20)	 return "disk file corrupted (gap3 too short)";
		uint8 scnt  = tp[0x15]; if(scnt>29)	 return "disk file corrupted (too many sectors)";
		uint  ssize = 0x80u << tp[0x14];
		if(ssize==0x2000) ssize = 0x1800;
		if(ssize*scnt+256 > track_size)		 return "disk file corrupted (sector data exceeds track data)";
		if((gap3+ssize+62)*scnt-gap3 > 6250) return "disk file corrupted (track data exceeds track size)";

		uint8* sip = tp + 0x18;		// sector info pointer
		uint8* sdp = tp + 256;		// sector data pointer

		Array<SectorFormatInfo> format(scnt);
		for(uint i=0;i<scnt;i++)	// sectors
		{
			// SECTOR INFO
			// 00		track		(parameter C in NEC765 commands)	1
			// 01		side		(parameter H in NEC765 commands)	1
			// 02		sector ID	(parameter R in NEC765 commands)	1
			// 03		sector size	(parameter N in NEC765 commands)	1
			// 04		status register 1 (NEC765 ST1 status register)	1
			// 05		status register 2 (NEC765 ST2 status register)	1
			// 06-07 	filler (0)										2

			SectorFormatInfo& f = format[i];

			f.track_id	= sip[0];
			f.side_id	= sip[1];
			f.sector_id	= sip[2];
			f.sector_sz = sip[3];
			f.gap3		= gap3;
			f.data		= sdp;
			f.special	= 0x00;
			if(sip[4]&DataError)   f.special |= sip[5]&DataErrorInData ? sf_data_crc_wrong : sf_id_crc_wrong;
			if(sip[4]&MissingAM)   f.special |= sip[5]&MissingDAM ? sf_dam_notfound : sf_idam_notfound;
			if(sip[5]&ControlMark) f.special |= sf_ddam;
			if((sip[4]&0x04)!=0) showInfo("SR1 in sector data has \"NoData\" bit set");	// wie kann das gesetzt sein?
			logline(" h=%i, t=%i, s=%i, sz=%i", int(f.side_id), int(f.track_id), int(f.sector_id), int(f.sector_sz));
			sdp += ssize;
			sip += 8;
		}

		writeTrack(format,getTrack(s,t),80);
		tp += track_size;
	}
	return NULL;	// ok
}


cstr FloppyDisk::read_extended_dsk_file(uint8* bu, uint32 sz)
{
	sides[0].purge();
	sides[1].purge();

/*	The DSK file starts with a 0x100 byte DISK INFORMATION BLOCK:

	00-21 	"EXTENDED CPC DSK File\r\nDisk-Info\r\n" 	34 bytes
	22-2f 	name of creator								14
	30		number of tracks							1
	31		number of sides								1
	32-33 	unused									 	2
	34-ff 	track size table 	number of tracks*number of sides

	"size of track" includes the 0x100 byte Track Information Block.
	All tracks must have a "Track Information Block"

	Tracks start at 0x100.
	Tracks are ordered 0 .. number_of_tracks - 1
	Tracks on DS disks are interleaved: S0Tr0, S1Tr0, S0Tr1, S1Tr1, ...
*/
	if(sz<0x100)		return "disk file too short (len<0x100)";
	uint num_tracks = bu[0x30];
	uint num_sides  = bu[0x31];
	if(num_sides<1)		return "num sides = 0";
	if(num_sides>2)		return "num sides > 2";
	if(num_tracks>83)	return "num tracks > 83";

	uint disk_size = 0x100;
	for(uint i=0;i<num_tracks*num_sides;i++) disk_size += bu[0x34+i] << 8;
	if(disk_size > sz)	return "disk file too short (data exceeds file size)";


	uint8* tip = bu+0x34;				// tip -> track size info
	uint8* tp = bu + 256;				// tp -> track data
	for(uint t=0; t<num_tracks; t++)	// tracks
	for(uint s=0; s<num_sides; s++)		// sides
	{
		uint track_size = *tip++ * 256;	// actual length of track data (incl. Track Information Block)
		if(track_size==0) continue;		// unformatted track

		// Each track starts with a 0x100 byte TRACK INFORMATION BLOCK:
		// 00-0c 	"Track-Info\r\n"		13 bytes
		// 0d-0f 	unused					3
		// 10		track number			1
		// 11		side number				1
		// 12-13 	unused					2
		// 14	 	max. sector size		1
		// 15	 	number of sectors		1
		// 16	 	GAP#3 length			1
		// 17	 	filler byte				1
		// 18-xx 	Sector Information List xx	(space for up to 29 sectors)
		// xx-ff	filler (0)

		if(memcmp(tp,"Track-Info",10)!=0)    return "disk file corrupted (wrong track magic)";
		uint8 track = tp[0x10]; if(track!=t) return "disk file corrupted (wrong track)";
		uint8 side  = tp[0x11]; if(side!=s)  return "disk file corrupted (wrong side)";
		uint8 gap3  = tp[0x16]; if(gap3<20)	 return "disk file corrupted (gap3 too short)";
		uint8 scnt  = tp[0x15]; if(scnt>29)	 return "disk file corrupted (too many sectors)";
		uint  ssize = 0x80u << (tp[0x14]&7);
//		if(ssize*scnt+256 > track_size)		 return "disk file corrupted (sector data exceeds track data)";
//		if((gap3+ssize+62)*scnt-gap3 > 6250) return "disk file corrupted (track data exceeds track size)";

		uint8* sip = tp + 0x18;		// sector info pointer
		uint8* sdp = tp + 256;		// sector data pointer

		Array<SectorFormatInfo> format(scnt);
		for(uint i=0;i<scnt;i++)	// sectors
		{
			// SECTOR INFO
			// 00		track		(parameter C in NEC765 commands)	1
			// 01		side		(parameter H in NEC765 commands)	1
			// 02		sector ID	(parameter R in NEC765 commands)	1
			// 03		sector size	(parameter N in NEC765 commands)	1
			// 04		status register 1 (NEC765 ST1 status register)	1
			// 05		status register 2 (NEC765 ST2 status register)	1
			// 06-07 	actual data length in bytes	(LSB first)			2

			uint actual_data_length = peek2Z(sip+6);
			if(actual_data_length!=ssize)
			{
				TODO();
			}

			SectorFormatInfo& f = format[i];

			f.track_id	= sip[0];
			f.side_id	= sip[1];
			f.sector_id	= sip[2];
			f.sector_sz = sip[3];
			if(sip[4]&DataError) f.special |= sip[5]&DataErrorInData ? sf_data_crc_wrong : sf_id_crc_wrong;
			if(sip[4]&MissingAM) f.special |= sip[5]&MissingDAM ? sf_dam_notfound : sf_idam_notfound;
			if(sip[5]&ControlMark) f.special |= sf_ddam;
			if((sip[4]&0x04)!=0) showInfo("SR1 in sector data has \"NoData\" bit set");	// wie kann das gesetzt sein?
			f.gap3		= gap3;
			f.data		= sdp;
			logline(" h=%i, t=%i, s=%i, sz=%i", int(f.side_id), int(f.track_id), int(f.sector_id), int(f.sector_sz));
			f.special	= 0x00;
			sdp += actual_data_length;
			sip += 8;
		}

		writeTrack(format,getTrack(s,t),80);
		tp += track_size;
	}
	return NULL;	// ok
}



void FloppyDisk::parse_track(uint8* track, TrackFormatInfo& trackinfo) const
{
	uint i=0; uint n;

	while(i<bytes_per_track)
	{
		SectorFormatInfo si;
		memset(&si,0,sizeof(si));

	// ID:
		while(i<bytes_per_track && track[i]!=0x4e) { i++; }				// garbage
		n=0; while(i<bytes_per_track && track[i]==0x4e) { i++; n++; }	// GAP1 or GAP3
		if(n<16) continue;
		si.gap3 = uint8(n);

		n=0; while(i<bytes_per_track && track[i]==0x00) { i++; n++; }	// VCO SYNC
		if(n<10) continue;

		if(i+10>bytes_per_track) break;									// sizeof ID

		n=0; while(n<3 && track[i]==0xA1) { i++; n++; }					// IDAM
		if(n!=3) continue;
		if(track[i]!=IDAM) continue; else i++;

		si.track_id  = track[i++];
		si.side_id   = track[i++];
		si.sector_id = track[i++];
		si.sector_sz = track[i++];
		uint sz = 0x80u << (si.sector_sz&7);

		i+=2; if(crc16(track+i-10,10)!=0) continue;						// ID CRC wrong

	// DATA:
		n=0; while(i<bytes_per_track && track[i]==0x4e) { i++; n++; }	// GAP2
		if(n<16) continue;

		n=0; while(i<bytes_per_track && track[i]==0x00) { i++; n++; }	// VCO SYNC
		if(n<10) continue;

		if(i+4>bytes_per_track) break;									// sizeof DAM

		n=0; while(n<3 && track[i]==0xA1) { i++; n++; }					// DAM / DDAM
		if(n!=3) continue;
		if(track[i]!=DAM && track[i]!=DDAM) continue;
		if(track[i]==DDAM) si.special |= sf_ddam;
		i++;

		if(i+sz+2>bytes_per_track) break;								// sizeof data + crc

		if(crc16(track+i-4,4+sz+2)!=0) si.special |= sf_data_crc_wrong;
		si.data = track + i;
		i += sz + 2;

		trackinfo.append(si);
	}
}

void FloppyDisk::parse_disk(Array<TrackFormatInfo> trackinfo[2])
{
	trackinfo[0].purge();
	trackinfo[1].purge();

	for(uint s=0;s<2;s++)
	for(uint t=0;t<sides[s].count();t++)
	{
		parse_track(sides[s][t],trackinfo[s][t]);
	}
}


void FloppyDisk::write_extended_disk_file(FD& fd) const throws
{
	Array<TrackFormatInfo> trackinfo[2];

	for(uint s=0;s<2;s++)
	{
		for(uint t=0;t<sides[s].count();t++)
		{
			trackinfo[s].append(TrackFormatInfo());
			parse_track(sides[s][t],trackinfo[s][t]);
		}
	}

// DISK INFORMATION BLOCK:

//		00-21 	"EXTENDED CPC DSK File\r\nDisk-Info\r\n" 	34 bytes
//		22-2f 	name of creator								14
//		30		number of tracks							1
//		31		number of sides								1
//		32-33 	unused									 	2
//		34-ff 	track size table							num_tracks*num_sides
//
//		"size of track" includes the 0x100 byte Track Information Block.
//		All tracks must have a "Track Information Block"
//
//		Tracks start at file offset 0x100.
//		Tracks are ordered 0 .. number_of_tracks - 1
//		Tracks on DS disks are interleaved: S0Tr0, S1Tr0, S0Tr1, S1Tr1, ...

	uint8 diskinfo[0x100]; memset(diskinfo,0,0x100);
	uint8* dip = diskinfo;

	uint num_sides  = trackinfo[1].count()==0 ? 1 : 2;
	uint num_tracks = max(trackinfo[0].count(),trackinfo[1].count());

	strcpy(ptr(dip),"EXTENDED CPC DSK File\r\nDisk-Info\r\n");		dip += 34;
	strcpy(ptr(dip),catstr(appl_name," ",appl_version_str));		dip += 14;
	*dip++ = uint8(num_tracks);
	*dip++ = uint8(num_sides);
	dip += 2;				// unused

	// tracksize table

	for(uint s=0;s<num_sides;s++)
	{
		for(uint t=0;t<num_tracks;t++)
		{
			if(trackinfo[s].count()<=t) { *dip++ = 0; continue; }

			Array<SectorFormatInfo>& si = trackinfo[s][t];

			uint max_sectorsize = 0;
			for(uint i=0;i<si.count();i++) max_sectorsize = max(max_sectorsize, uint(si[i].sector_sz));
			assert(max_sectorsize>=2);
			assert(max_sectorsize<=7);
			max_sectorsize = 0x80u << max_sectorsize; if(max_sectorsize==0x2000) max_sectorsize = 0x1800;
			uint tracksize = 0x100 + max_sectorsize * si.count();
			*dip++ = uint8(tracksize >> 8);
		}
	}
	fd.write_bytes(diskinfo,0x100);

// TRACKS:

	dip = diskinfo + 0x34;

	for(uint t=0; t<num_tracks; t++)	// tracks
	for(uint s=0; s<num_sides; s++)		// sides
	{
		uint track_size = *dip++ * 256;	// actual length of track data (incl. Track Information Block)
		if(track_size==0) continue;		// unformatted track

		// Each track starts with a 0x100 byte TRACK INFORMATION BLOCK:
		// 00-0c 	"Track-Info\r\n"		13 bytes
		// 0d-0f 	unused					3
		// 10		track number			1
		// 11		side number				1
		// 12-13 	unused					2
		// 14	 	max. sector size		1
		// 15	 	number of sectors		1
		// 16	 	GAP#3 length			1
		// 17	 	filler byte				1
		// 18-xx 	Sector Information List xx	(space for up to 29 sectors)
		// xx-ff	filler (0)

		Array<SectorFormatInfo>& si = trackinfo[s][t];

		uint8 trackinfo[256]; memset(trackinfo,0,256);
		uint8* tip = trackinfo;

		memcpy(tip,"Track-Info",13); tip += 16;
		*tip++ = uint8(t);		//	track#
		*tip++ = uint8(s);		//	side#
		tip += 2;				//	unused

		uint max_sectorsize = 0;
		for(uint i=0;i<si.count();i++) max_sectorsize = max(max_sectorsize, uint(si[i].sector_sz));
		*tip++ = uint8(max_sectorsize);	//	log2(max_sector_size)
		*tip++ = uint8(si.count());		//	num sectors
		*tip++ = 54;					//	GAP3 len		((TODO))
		tip++;							//	filler

	// SECTOR INFORMATION LIST:

		// SECTOR INFO
		// 00		track		(parameter C in NEC765 commands)	1
		// 01		side		(parameter H in NEC765 commands)	1
		// 02		sector ID	(parameter R in NEC765 commands)	1
		// 03		sector size	(parameter N in NEC765 commands)	1
		// 04		status register 1 (NEC765 ST1 status register)	1
		// 05		status register 2 (NEC765 ST2 status register)	1
		// 06-07 	actual data length in bytes	(LSB first)			2

		for(uint i=0;i<si.count();i++)
		{
			*tip++ = si[i].track_id;
			*tip++ = si[i].side_id;
			*tip++ = si[i].sector_id;
			*tip++ = si[i].sector_sz;
			*tip++ = 0;					// SR1
			*tip++ = 0;					// SR2
//			if(sip[4]&DataError) f.special |= sip[5]&DataErrorInData ? sf_data_crc_wrong : sf_id_crc_wrong;
//			if(sip[4]&MissingAM) f.special |= sip[5]&MissingDAM ? sf_dam_notfound : sf_idam_notfound;
//			if(sip[5]&ControlMark) f.special |= sf_ddam;
//			if((sip[4]&0b00000100)!=0) showInfo("SR1 in sector data has \"NoData\" bit set");	// wie kann das gesetzt sein?
			uint ssize = 0x80u << si[i].sector_sz; if(ssize==0x2000u) ssize=0x1800u;
			poke2Z(tip,uint16(ssize)); tip+=2;
		}
		fd.write_bytes(trackinfo,256);

	// SECTOR DATA:

		for(uint i=0;i<si.count();i++)
		{
			uint ssize = 0x80u << si[i].sector_sz; if(ssize==0x2000u) ssize=0x1800u;
			fd.write_bytes(si[i].data, ssize);
		}
	}
}


static const uint8 FDD_MAGIC[4] = { 182,32,34,201 };
static const uint8 TRACK_MAGIC  = 213;
enum { MF,MFM,HD,ED };

void FloppyDisk::saveToFile(FD& fd) const throws
{
	fd.write_bytes(FDD_MAGIC,4);
	fd.write_uint8(0);					// comments array size					To be defined
	fd.write_uint8(MFM);				// Disk format
	fd.write_uint8(0);					// Track format exceptions table size	To be defined
	fd.write_uint8(0);					// Sector format exceptions table size	To be defined
	fd.write_uint8(0);					// weak bytes table size				To be defined
	fd.write_uint8(0);					// unwritable (FF) bytes table size		To be defined

	for(uint s=0;s<2;s++)				// 2 sides
	{
		fd.write_uint8(sides[s].count());		// num tracks
		for(uint t=0;t<sides[s].count();t++)	// N tracks
		{
			fd.write_uint8(TRACK_MAGIC);
			fd.write_uint8(254);				// uint16 flag (see: write_nstr() in "unix/fd_throw.cpp")
			fd.write_uint16_z(bytes_per_track);
			fd.write_bytes(sides[s][t],bytes_per_track);	TODO();
		}
	}
}

void FloppyDisk::loadFromFile(FD& fd) throws
{
	(void)fd;
	TODO();
}























