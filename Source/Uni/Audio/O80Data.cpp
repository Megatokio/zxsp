// Copyright (c) 2000 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "O80Data.h"
#include "RlesData.h"
#include "TapeFile.h"
#include "TapeFileDataBlock.h"
#include "Z80/Z80.h"
#include "globals.h"
#include "kio/util/count1bits.h"
#include "unix/files.h"


/*  A ZX80 program is stored like this on a real audio tape:

		x seconds    your voice, saying "filename" (optional)
		x seconds    video noise
		5 seconds    silence
		LEN bytes    data, loaded to address $4000, LEN = ($400A)-$4000.
		x seconds    silence / video noise

	Notes:
		ZX80 files do not have filenames
		ZX80 files cannot be autostarted.
		The data is loaded to address $4000++
		The data contains the whole system area, basic program, VARS.
		Video memory is NOT included in ZX80 files.
		the last byte of a (clean) file should be $80 (the last byte of VARS)
		The system area should contain proper data.
		$400A       (2 bytes) defines the data end address (used to calculate the file length).
		$4028++     may be misused for whatever purpose.

		While loading, the data at address $400A/400B is overwritten. After this they contain
		the real data end address of the data loaded and define when loading will stop. :-)

		Files should usually not exceed 16 kBytes.
		The memory detection procedure in both ZX80 and ZX81 stops after 16 kBytes (at $8000),
		and initializes the stack pointer at that address, even if more memory is installed.
		Thus loading files of 16k or more would destroy the stack area,
		unless a separate loader has previously moved the stack area to another location.
		However, most ZXes don't have more than 16k RAM, so bigger files won't load on most computers.

	".o" and ".80" files consists of the raw data as saved by the ZX80 tape saving routine.
	They can only store one program, not a whole tape with multiple programs.

		.80 and .o files:   include only the data, loaded to $4000++
		.o files:           typically there is some garbage at the file end
*/


// ###############################################################################
// constants


// ZX81 character set:
// char(1) to char(10) are block graphics
// char(12) is the £ symbol
static char zx81_charset[] = " XXXXXXXXXX\"_$:?()><=+-*/;,.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";


static const uint32 ccps	= 3250000; // samples per second  ==  cpu clock
static const uint32 zx81lo	= 520;	   // high pulse length
static const uint32 zx81hi	= 530;	   // low pulse length
static const uint32 zx81ooo = 4689;	   // "bit gap" length
// static const uint32 ccbit0  = zx81hi+3*(zx81hi+zx81lo)+zx81ooo;    // cc per bit 0
// static const uint32 ccbit1  = zx81hi+8*(zx81hi+zx81lo)+zx81ooo;    // cc per bit 1


// ###############################################################################
// c'tor etc.


O80Data::O80Data() : TapeData(isa_O80Data), is_zx80(no), is_zx81(no) {}


O80Data::~O80Data() {}


// ###############################################################################
//						convert
// ###############################################################################


/*  convert arbitrary TapeData to O80 data
	set's data_trust_level to indicate success or failure
	note: also used for P81 data
*/
O80Data::O80Data(const TapeData& q) : TapeData(isa_O80Data)
{
	switch (q.isaId())
	{
	case isa_TapData:
		new (this) O80Data();
		trust_level = conversion_failed;
		break;
	case isa_O80Data: new (this) O80Data(O80DataRef(q)); break;
	case isa_TzxData: new (this) O80Data(TzxDataRef(q)); break;
	default: new (this) O80Data(CswBuffer(q, 3250000)); break;
	}
}


/*  copy O80 data to O80 data
 */
O80Data::O80Data(const O80Data& q) : TapeData(q), data(q.data), is_zx80(q.is_zx80), is_zx81(q.is_zx81) {}


/*	construct O80Data from existing data
 */
O80Data::O80Data(Array<uint8> q, bool is_zx81) :
	TapeData(isa_O80Data, original_data),
	data(q),
	is_zx80(!is_zx81),
	is_zx81(is_zx81)
{}


/*  convert CswBuffer to O80Data

	--> trust_level:
		no_data				 looks like silence
		conversion_failed	 looks like an unknown data signal
		truncated_data_error started good but stopped prematurely
		checksum_error		 decoding succeeded, but the last byte (of VARS) is not 0x80
		decoded_data		 decoding succeeded, no errors

	--> is_zx80 or is_zx81:
		After successful conversion is_zx80 or is_zx81 is set. The data can be written to a tape file.
		After failed conversion both is_zx80 and is_zx81 are cleared.
*/
O80Data::O80Data(const CswBuffer& bu) : TapeData(isa_O80Data), is_zx80(no), is_zx81(no)
{
	/*	ZX80/ZX81 tape audio layout:
		5s silence (nominal low)
		N bytes:
			8 Bits:		(msb first)
				4 or 9 high pulses à 530cc
				with low pulses à 520cc
				one final x-long low pulse à 4689cc
	*/

	const uint ccpulse	   = (zx81lo + zx81hi) / 2;
	const uint ccpulse_max = ccpulse * 3 / 2;
	const uint cclooo_min  = zx81ooo - 2 * ccpulse;
	const uint cclooo_max  = zx81ooo + 2 * ccpulse;

	bu.seekStart();
	data.purge();
	trust_level = no_data;

	uint gap_a_pulses = 0; // for sanity check

start:
	uint32 cc;
	uint   n;

	n = 0;
	do {
		n++;
		cc = bu.readPulse();
	}
	while (cc && cc <= ccpulse_max);
	if (cc == 0) { return; /* no_data */ }
	gap_a_pulses += n;
	if (cc < cclooo_min || cc > cclooo_max) goto start;
	if (n < 8 - 3 || n > 18) goto start; // too few/too many pulses
	if (n > 8 && n < 18 - 3) goto start; // illegal pulse count
	gap_a_pulses -= n;

	data.grow(bu.getTotalPulses() / (8 * 8) + 2);
	uint zi	  = 0;
	uint byte = 1;
	goto b;

	for (;;)
	{
		n = 0;
		do {
			n++;
			cc = bu.readPulse();
		}
		while (cc && cc <= ccpulse_max);
		if (n != 8 && n != 18) break; // too few/too many pulses
		if (cc < cclooo_min) break;	  // looong pulse too short => error
	b:
		byte += byte + (n > 13);
		if (byte & 0x100)
		{
			data[zi++] = byte;
			byte	   = 1;
		}
		if (cc > cclooo_max) break; // looong pulse too too long => assume end
	}

	// if we come here, decoding has finished due to either:
	// • ill. pulse count		(n!=8 && n!=18)
	// • long pulse too short	(cc<cclooo_min)
	//   this might also be end of data if cc==0
	// • long pulse too long	(typical end of data)
	//	 if byte==1 then stopped at proper byte boundary
	//		then there may be valid looking bits following
	//		test of $400A may be ok / not ok  (ZX80)
	//		last byte may be / may be not $80
	//	 if byte!=1 then breaked in the middle of a byte
	//		but it _might_ be ok if all other tests are ok

	is_zx80 = zi >= 0x29 &&						  // min. length
			  0x4000 + zi >= peek2Z(&data[0x0A]); // E_LINE

	uint namelen = 0;
	while (namelen < zi && data[namelen] < 0x40) { namelen++; } // file name
	is_zx81 = namelen < zi &&									// not stopped at buffer end
			  data[namelen] >= 0x80 && data[namelen] < 0xC0 &&
			  ++namelen < 128 &&					 // last char of file name and name length
			  zi - namelen >= 0x407D + 1 - 0x4009 && // minimum length
			  0x4009 + zi - namelen >= peek2Z(&data[0x4014 - 0x4009 + namelen]); // E_LINE


	if (is_zx80 || is_zx81)
	{
		if (is_zx80 && is_zx81)
		{
			is_zx80 = is_zx81 = no;

			uint32 zx80_len = peek2Z(&data[0x0A]) - 0x4000;
			uint32 zx81_len = peek2Z(&data[0x4014 - 0x4009 + namelen]) - 0x4009 + namelen;

			if (zi == zx81_len) is_zx81 = yes;
			else // Länge passt exakt: das wird's wohl sein
				if (zi == zx80_len) is_zx80 = yes;
				else // Länge passt exakt: das wird's wohl sein
					// sonst Heuristik:
					if (namelen > 31) is_zx80 = yes;
					else // unglaublich langer Name => ZX81 unwahrscheinlich
						if (zi > 4 kB) is_zx81 = yes;
						else // > 4kB => ZX80 ist unwahrscheinlich wg. orig. ZX80 Rampack
							// letzte Idee: wer weniger Bytes zuviel hat:
							if (zx80_len > zx81_len - namelen + 9) is_zx80 = yes;
							else is_zx81 = yes;
		}

		// truncate block at E_LINE:
		if (is_zx80) zi = peek2Z(&data[0x0A]) - 0x4000;
		else zi = peek2Z(&data[0x4014 - 0x4009 + namelen]) - 0x4009 + namelen;
		data.shrink(zi);

		trust_level = data.last() == 0x80 ? decoded_data : checksum_error; // last byte of VARS != 0x80

		return;
	}

	// if we come here, decoding has finished due to either:
	// • ill. pulse count		(n!=8 && n!=18)
	// • long pulse too short	(cc<cclooo_min)
	//   this might also be end of data if cc==0
	// • long pulse too long	(typical end of data)
	//	 if byte==1 then stopped at proper byte boundary or
	//	 if byte!=1 then breaked in the middle of a byte
	//		data sanity test for ZX80 or ZX81 failed
	//		then there may be valid looking bits following

	data.shrink(zi);
	if (byte == 1 && zi >= 32 && cc > cclooo_max)
	{
		trust_level = truncated_data_error;
		return;
	}
	if (zi > 0)
	{
		trust_level = truncated_data_error;
		return;
	}

	data.purge();
	if (zi == 0 && gap_a_pulses > 666)
	{
		trust_level = conversion_failed;
		return;
	}
	trust_level = no_data;
}


// ###############################################################################


// saved data starts with a program name, 127 char max and last char | 0x80
// saved progname in file is in ZX81 charset!
//
inline int zx81_progname_len(cu8ptr p, int n)
{
	if (n > 127) n = 127;
	for (int i = 0; i < n; i++)
	{
		if (p[i] & 0x80) return i + 1;
	}
	return n;
}

static cstr zx81_progname_str(cu8ptr p, int n)
{
	n	  = zx81_progname_len(p, n);
	str s = tempstr(n);
	for (int i = 0; i < n; i++) { s[i] = zx81_charset[p[i] & 0x3F]; }
	return s;
}


/* returns the program name, if available
 */
cstr O80Data::calcMajorBlockInfo() const noexcept
{
	if (is_zx80) return "ZX80 Programme";
	if (is_zx81)
	{
		cu8ptr p = data.getData();
		uint   n = data.count();
		return n == 0 ? "(no data)" : *p == 0x80 ? "(unnamed)" : zx81_progname_str(p, n);
	}
	return nullptr;
}

/* returns info: block size
 */
cstr O80Data::calcMinorBlockInfo() const noexcept
{
	if (zx81 || zx80)
	{
		uint n = data.count();
		if (zx81) n -= zx81_progname_len(data.getData(), data.count());
		return usingstr("%u bytes", n);
	}
	return nullptr;
}


CswBuffer::CswBuffer(const O80Data& o80data, uint32 ccps) : CswBuffer(ccps, 0, 666)
{
	xlogIn("new CswBuffer(O80Data&)");

	// resize to estimated size:
	uint32 n1 = count1bits(o80data.data.getData(), o80data.data.count());
	uint32 n0 = o80data.data.count() * 8 - n1;
	grow(n1 * 18 + n0 * 8 + 600);

	// initial level := low  &  leading silence required by zx80 rom tape loading routine
	writePulse(5.0, 0);

	// data
	double f	   = (double)::ccps / ccps;
	uint   zx81lo  = uint(::zx81lo * f + 0.5);
	uint   zx81hi  = uint(::zx81hi * f + 0.5);
	uint   zx81ooo = uint(::zx81ooo * f + 0.5);

	for (uint i = 0; i < o80data.data.count(); i++)
	{
		uint c = o80data.data[i];
		for (int i = 7; i >= 0; i--)
		{
			writePulseCc(zx81hi, 1);
			bool b = (c >> i) & 1;
			for (int i = b ? 8 : 3; i; i--)
			{
				writePulseCc(zx81lo);
				writePulseCc(zx81hi);
			}
			writePulseCc(zx81ooo);
		}
	}

	// trailing pause, low level
	writePulse(1.0, 0);

	assert(getPhase0() == 0);
}


/*  read block from file
	.p and .81 files contain only one block => load entire file (limited to 0xC000 bytes)
					 contain no prog name   => store prog name " "
	.p81 file can contain multiple blocks   => parse contained data to find block end
*/
void O80Data::zx81_read_from_file(FD& fd, bool incl_pname) noexcept(false) // bad_alloc,file_error,data_error
{
	xlogIn("P81Data:ReadFromFile");

	assert(data.count() == 0);

	if (incl_pname) // .p81 file
	{
		uint n = min(uint(fd.file_remaining()), 127 + 12u); // 12 = 0x4015-0x4009
		data.grow(n);
		fd.read_bytes(&data[0], n); // prog name + some sysvars + evtl. some prog data

		uint l = zx81_progname_len(data.getData(), n);
		if (n - l < 12) throw FileError(fd, endoffile);

		uint end = peek2Z(data.getData() + l + 0x4014 - 0x4009);
		if (end < 0x4015) throw DataError("data corrupted: ($4014) < $4015");

		data.grow(end - 0x4009 + l);
		fd.read_bytes(&data[n], end - 0x4009 + l - n); // remaining sysvars & program data.  throws at eof
	}
	else // .p or .81 file
	{
		uint n = min(uint(fd.file_size()), 0xc000u);
		data.grow(1 + n);
		data[0] = 0x00 | 0x80; // program name: " "
		fd.read_bytes(&data[1], n);
		fd.seek_endoffile();
	}
}


/*  write ZX81 block to .p81 file
	.p81 blocks are checked for integrity, because a corrupted block makes the remaining file unreadable:
		sysvar $4014 E_LINE must be contained and it must match actual block size
		if block is larger than required, then the data is silently truncated to match E_LINE
	this is the minimum requirement for not to corrupt the .p81 file, NOT for a valid program!
*/
void O80Data::write_block_to_p81_file(FD& fd) const noexcept(false) // file_error,data_error
{
	assert(is_zx81);

	const uint8* p = data.getData();
	uint		 n = data.count();
	uint		 l = zx81_progname_len(p, n);

	if (n - l < 0x4015 - 0x4009) throw DataError("data corrupted: data does not include sysvar $4014");

	uint end = peek2Z(p + l + 0x4014 - 0x4009);
	if (n - l < end - 0x4009) throw DataError("data corrupted: data size does not match sysvar $4014");

	fd.write_bytes(p, end - 0x4009 + l);
}


/*static*/
void O80Data::readFile(cstr fpath, TapeFile& data) noexcept(false) // file_error,data_error,bad_alloc
{
	xlogIn("O80Data::readFile(%s)", fpath);

	FD	 fd(fpath, 'r'); // throw file_error
	cstr ext = lowerstr(extension_from_path(fpath));

	if (eq(ext, ".o") || eq(ext, ".80"))
	{
		O80Data* o80data = new O80Data();
		uint	 len	 = min(uint(fd.file_size()), 0xc000u);
		o80data->data.grow(len);
		fd.read_bytes(&o80data->data[0], len);
		o80data->trust_level = original_data;
		o80data->is_zx80	 = yes;
		data.append(o80data);
	}
	else
	{
		bool incl_pname = eq(ext, ".p81");

		while (fd.file_remaining())
		{
			O80Data* o80data = new O80Data();
			o80data->zx81_read_from_file(fd, incl_pname);
			o80data->trust_level = original_data;
			o80data->is_zx81	 = yes;
			data.append(o80data);
		}
	}
}


/*	write file:
	is_zx80: ".o" or ".80"
	is_zx81: ".p", ".81" or ".p81"
	writes what can be written.
	logical errors are only logged.
	caller must validate file type before calling e.g. with TapeFile::canBeSavedAs(..)
*/
/*static*/
void O80Data::writeFile(cstr fpath, TapeFile& data) noexcept(false) // file_error,data_error,bad_alloc
{
	xlogIn("O80Data::writeFile(%s)", fpath);

	cstr ext = lowerstr(extension_from_path(fpath));

	bool o80 = eq(ext, ".o") || eq(ext, ".80");
	bool p81 = eq(ext, ".p81");

	assert(o80 || p81 || eq(ext, ".p") || eq(ext, ".81"));


	FD fd(fpath, 'w'); // throw file_error


	if (p81) // .p81 file allows multiple blocks in a single file:
	{
		for (uint i = 0; i < data.count(); i++)
		{
			TapeFileDataBlock* datablock = data[i];
			datablock->getO80Data();
			O80Data* td = datablock->getO80Data();
			if (td && td->is_zx81) td->write_block_to_p81_file(fd);
		}
		if (fd.file_position() == 0) { xlogline("error: no suitable block in TapeFile"); }
	}
	else // .o or .p file can store only 1 block:
	{
		O80Data* bestdata = nullptr;

		for (uint i = 0; i < data.count(); i++)
		{
			TapeFileDataBlock* datablock = data[i];
			datablock->getO80Data();
			O80Data* td = datablock->getO80Data();
			if (td == nullptr || td->trust_level < TrustLevel::conversion_success) continue; // no o80 block
			if (o80 ? td->is_zx81 : td->is_zx80) continue;									 // wrong model for file
			if (bestdata == nullptr)
			{
				bestdata = td;
				continue;
			} // store first candidate
			xlogline("error: more than 1 suitable block in TapeFile");
			if (td->trust_level > bestdata->trust_level) bestdata = td; // pick best candidate
		}

		if (bestdata)
		{
			u8ptr p = bestdata->getData();
			uint  n = bestdata->count();
			if (!o80)
			{
				uint l = zx81_progname_len(p, n);
				p += l;
				n -= l;
			}
			fd.write_bytes(p, n);
		}
		else xlogline("error: no suitable block in TapeFile");
	}
}
