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

#ifndef FLOPPYDISK_H
#define FLOPPYDISK_H

#include "Templates/Array.h"


extern uint16 crc16( const uint8*  q, uint count );

enum
{
	sf_id_crc_wrong		= 1<<0,
	sf_data_crc_wrong	= 1<<1,
	sf_idam_notfound	= 1<<2,
	sf_dam_notfound		= 1<<3,
	sf_ddam				= 1<<4
};

struct SectorFormatInfo
{
	uint8	side_id;		// normally: 0/1 for side 0/1
	uint8	track_id;		// normally: starting at 0
	uint8	sector_id;		// starting at 1 (DOS), 0x41 (CP/M)
	uint8	sector_sz;		// log2(sector_length) - 7
	uint8	gap3;			// gap *before* this sectors; GAP1=50; GAP3=54 (DOS)
	uint8	databyte;		// if data==NULL
	uint8	special;
	uint8*	data;

	uint	totalsize()		{ return gap3+12+10+22+12+4+(0x80<<sector_sz)+2; }
};

typedef Array<SectorFormatInfo> TrackFormatInfo;


struct DiskFormatInfo
{
	uint	sides;
	uint	tracks;
	uint	sectors;
	uint	side0;
	uint	track0;
	uint	sector0;
	uint	sector_sz;		// log2(SZ)-7
	uint	databyte;
	uint	gap4a;			// 80 (FDC765) or 92 (TMS279X)
	uint	gap3;
	uint	interleaved;	// 1 3 5 7 9 2 4 6 8
	//uint	head_skew;
	//uint	track_skew;
};


class FloppyDisk
{
public:
	uint		bytes_per_track;
	bool		writeprotected;
	bool		modified;
	cstr		filepath;
	Array<uint8*> sides[2];

public:
	explicit FloppyDisk(uint bytes_per_track=6250);		// 6250: mfm, 300rpm
	FloppyDisk(uint sides, uint tracks, uint sectors, bool interleaved, uint bytes_per_track=6250);
	explicit FloppyDisk(cstr filepath);
	~FloppyDisk();

	void	saveToFile(FD&) const throws;
	void	loadFromFile(FD&) throws;

	void	makeSingleSided();
	void	truncateTracks(uint n);
	void	saveAs(cstr path) throws;
	void	saveDisk() throws;
	bool	fileIsWritable();
	bool	isModified()					{ return modified; }
	bool	isWriteProtected()				{ return writeprotected; }
	cstr	getFilepath()					{ return filepath; }
	uint	getBytesPerTrack()				{ return bytes_per_track; }
	int		setWriteProtected(bool);
	void	setFilepath(cstr);

	void	formatDisk(const DiskFormatInfo &);
	void	writeTrack(Array<SectorFormatInfo>&, uint8 *&track, uint gap4a);
	uint8*	writeSector(SectorFormatInfo&, uint8*);

	uint8*&	getTrack(uint head, uint track)	{ sides[head].grow(track+1); return sides[head][track]; }	// may be NULL
	uint8	readByte(uint head, uint track, uint bytepos);
	void	writeByte(uint head, uint track, uint bytepos, uint8 byte);

private:
	cstr	read_dsk_file(uint8* bu, uint32 sz);
	cstr	read_extended_dsk_file(uint8* bu, uint32 sz);
	void	parse_disk(Array<Array<SectorFormatInfo>> trackinfo[]);			// !!! ObjArray
	void	parse_track(uint8 *track, TrackFormatInfo &sectorinfo) const;
	void	write_extended_disk_file(FD &fd) const throws;
};


#endif




























