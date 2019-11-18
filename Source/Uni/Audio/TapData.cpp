/*	Copyright  (c)	Günter Woigk 1994 - 2018
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

#include "unix/FD.h"
#include "Templates/Array.h"
#include "TapeFile.h"
#include "TapeFileDataBlock.h"
#include "CswBuffer.h"
#include "TapeData.h"
#include "TapData.h"
#include "RlesData.h"
#include "Z80/Z80.h"
#include "Z80/Z80opcodes.h"
#include "ZxInfo.h"
#include "globals.h"
#include "kio/util/count1bits.h"
#include "globals.h"
#include "TzxData.h"


/*	Timing				ZX Spectrum		Jupiter Ace:
	–––––––––––––––––––	–––––––––––––––	–––––––––––––––––––––
	Cpu clock:			3.5 MHz			3.25 MHz
	cpu cycles for pulses:	(cc)
		pilot:			2168			2011
		bit 0:			855				798	 (1:795,  0:801)
		bit 1:			1710			1588 (1:1585, 0:1591)
		sync1:			667				601
		sync2:			738				791
	pause after header:	1s				2ms	 (1:903,  0:4187)
	pause after data:   2s				2s
	pilot pulses header 8064			8192
	pilot pulses data:  3220			1024

	Resulting timing is very similar!


	Forth tape headers
	––––––––––––––––––
	1   byte  blocktype (0x00 = header block, 0xFF = data block)
	header block:
	1	byte  filetype	(0x00 for Dict, 0x20* for binary data)
	10  bytes filename	padded with spaces
	2	bytes filesize	length of file
	2	bytes address	Dict: 0x3C51 (15441); binary data: Default load address, used when desired target = 0
	2	bytes current word.											Unused  for binary data
	2	bytes value of system var CURRENT [Address 0x3C31, 15409].	Unused* for binary data
	2	bytes value of system var CONTEXT [Address 0x3C33, 15411].	Unused* for binary data
	2	bytes value of system var VOCLNK  [Address 0x3C35, 15413].	Unused* for binary data
	2	bytes value of system var STKBOT  [Address 0x3C37, 15415].	Unused* for binary data
	1	Checksum [filetype .. STKBOT] XORed together (unlike Spectrum WITHOUT Blocktype)

	*Filetype for bytes file may be any non zero value.
	*Unused bytes are filled with spaces.
	The file body is loaded to the begin of free memory: 0x3C51 when memory is empty.
	Addresses inside of the file body are relative to loading position, idR. 0x3C51.
*/



struct Timing
{
uint32 const	ccps;				// cpu cycles per second
uint32 const	cc_pilot;			// cpu cycles per pilot half phase
uint32 const	cc_bit0;			// cpu cycles per 0-bit half phase
uint32 const	cc_bit1;			// cpu cycles per 1-bit half phase
uint32 const	cc_sync1;			// cpu cycles for 1st sync phase
uint32 const	cc_sync2;			// cpu cycles for 2nd sync phase
Time   const	pause[2];			// trailing pause after [0]=header: 1 sec; [1]=data: 2 sec
uint32 const	pilot_pulses[2];	// pilot pulses: [0]=8064 Header; [1]=3220 Data

		Time	spp_pilot() const { return (Time)cc_pilot/ccps; }	// seconds per pulse
		Time	spp_sync1() const { return (Time)cc_sync1/ccps; }	// seconds per pulse
		Time	spp_sync2() const { return (Time)cc_sync2/ccps; }	// seconds per pulse
		Time	spp_bit0()  const { return (Time)cc_bit0/ccps; }	// seconds per pulse
		Time	spp_bit1()  const { return (Time)cc_bit1/ccps; }	// seconds per pulse
};

Timing const timing[2] =
{
	{3500000, 2168,855,1710,667,735,{1.000,2.0},{8064,3220}},	// ZX Spectrum
	{3250000, 2011,798,1588,601,791,{0.002,2.0},{8192,1024}}	// Jupiter Ace
};



// ###############################################################################
//                      Utilities
// ###############################################################################


/*	Calculate checksum
	if the passed buffer includes the block checksum, then the returned checksum must be 0,
	else the returned checksum must equal the byte finalizing this block.
	jupiter ace checksum does not include the block type byte.
		this is a don't care for header blocks (type byte = 0x00)
		but results in complemented checksum for data blocks (type byte = 0xff)
		or something else (for ill. blocktypes) if calculated as for a ZX Spectrum.
*/
uint calcTapeBlockChecksum( uint8 const* data, int blen, bool jupiter )
{
	uint8 const* e = data+blen;
	uint sum = 0;
	if(jupiter) data++;
	while(data<e) sum ^= *data++;
	return sum;
}


/*	Guess whether this is a Jupiter Ace header block
	This can be used to detect whether sampled audio data (e.g. audio in or tzx) is for the Jupiter Ace.
	For .tap files this test should be done purely based on the block length of the first block.
	For real audio the gap length between header and data could be tested too.
*/
bool isaJupiterHeaderBlock( uint8 const* data, int blen )
{
	if(blen!=27) return no;			// Jupiter header length: 1+25+1
	if(data[0]!=0) return no;		// block type: 0=header
	return (data[1]|0x20)==0x20;	// data type: 0=forth dict, 0x20=binary
}

bool isaZxSpectrumHeaderBlock( uint8 const* data, int blen)
{
	if(blen!=19) return no;			// ZX Spectrum header length: 1+17+1
	if(data[0]!=0) return no;		// block type: 0=header
	return data[1]<=3;				// data type: 0=prog, 1,2=data, 3=code
}


/*	Calculate major block info from tape data
	ZX Spectrum / Jupiter Ace is auto-detected from block length
*/
cstr calcMajorTapBlockInfo( uint8 const* data, int blen )
{
	if (blen<=0)	return nullptr;				// not decodable
	if (blen<2)		return "Blips";				// minimum length is 3 = type + data[1] + crc

	uint btyp = data[0];						// block type
	if(btyp==0xff)	return "Data block";		// zxsp & jupiter
	if(btyp!=0x00)	return catstr( "Block with illegal type: ", tostr(btyp) );	// zxsp & jupiter
	if(blen!=27 && blen!=19) return "Long header";
	bool is_jupiter = blen==27;

	uint htyp = data[1];						// zxsp & jupiter: header type
	if(is_jupiter?htyp&~0x20:htyp>3) return catstr( "Header with illegal type: ", tostr(htyp) );

// xzsp & jupiter: Standard Header Block (btyp = 0x00):

// get file name:
// zxsp & jupiter:
//	 name[10] = header[2 to 12(excl.)]; padded with spaces
//	 char($60) = ascii('`') = zxsp = jupiter = Brit. Pound
//	 char($7F) = zxsp = jupiter = (c)
//	 zxsp:    char ≥ $80 = Basic tokens and graphics characters
//	 jupiter: char ≥ $80 = inverse (to be verified)

	char name[101];								// int32 enough for 10 x "RANDOMIZE "
	ptr d = name;
	for (int i=2;i<12;i++)						// 10 bytes name
	{
		int8 c = data[i];
		if(c<0)
		{
			if(is_jupiter) c &= 0x7F;
			else { d = stpcpy(d,basic_token[c+128]); if(c>=(int8)0xa3) *d++=' '; continue; } // graphic characters, basic token
		}
		if(c<' ')  { *d++ = '?'; continue; }	// control code
		if(c=='`') { static char const brt[] = "£"; d = stpcpy(d,brt); continue; }
		if(c==127) { static char const cpy[] = "©"; d = stpcpy(d,cpy); continue; }
		*d++ = c;
	}
	*d = 0;
	while( d>name && d[-1]==' ' ) *--d = 0;		// remove padded spaces

	if(is_jupiter)
	{
		return catstr(htyp ? "Binary data: " : "Dictionary: ", d==name ? "(unnamed)" : name);
	}
	else	// ZX Spectrum
	{
		static cstr const typstr[] = { "Program: ", "Data: ", "Data: ", "Code: " };
		return catstr(typstr[htyp], d==name?"(unnamed)":name);
	}
}


/*	Calculate minor block info from tape data
	ZX Spectrum / Jupiter Ace is auto-detected from block length
	returns NULL for blips
	returns length info for long headers and for data blocks
*/
cstr calcMinorTapBlockInfo( uint8 const* data, int blen )
{
	if (blen<2) return nullptr;					// blib
	uint btyp = data[0];						// block type
	if(btyp==0x00)								// else not a header block
	if(blen==27 || blen==19)					// else long header
	{
		// standard header block
		uint dlen = peek2Z(data+12);
		uint dpos = peek2Z(data+14);
		uint htyp = data[1];						// header type

		if(blen==27)	// Jupiter Ace header block
		{
			if(htyp==0x00)  return usingstr("length: %i bytes",dlen);			// Forth Dict
			if(htyp==0x20)	return usingstr("%i bytes, address: %i",dlen,dpos);	// Binary data
		}
		else			// ZX Spectrum header block
		{
			if(htyp==0 /*prog*/) return usingstr( dpos>9999 ? "length: %i bytes" : "%i bytes, starts at line %i", dlen, dpos );
			if(htyp<=2 /*data*/) return usingstr( "length: %i bytes", dlen );
			if(htyp==3 /*code*/) return usingstr( "%i bytes, address: %i", dlen, dpos );
		}
	}
	return usingstr("%i bytes",blen-2);
}


/*	Calculate major and minor block info from tape data
	The minor block info is only set for header blocks,
	for data blocks the tape recorder should display a progress indicator
*/
void calcTapBlockInfos(uint8 const* data, int blen, cstr& major, cstr& minor )
{
	cstr a = calcMajorTapBlockInfo(data,blen);
	cstr b = calcMinorTapBlockInfo(data,blen);
	delete[] major; major = newcopy(a);
	delete[] minor; minor = newcopy(b);
}



// ###############################################################################
//                      c'tor & d'tor
// ###############################################################################


TapData::TapData(bool is_zxsp, bool is_jupiter)
:
	TapeData(isa_TapData),
	is_zxsp(is_zxsp),
	is_jupiter(is_jupiter),
	pause(0),
	pilot_pulses(0),
	csw_pilot(0),
	csw_data(0),
	csw_pause(0)
{}


TapData::~TapData()
{}


TapData::TapData(const TapData &q)
:
	TapeData(q),
	data(q.data),
	is_zxsp(q.is_zxsp),
	is_jupiter(q.is_jupiter),
	pause(q.pause),
	pilot_pulses(q.pilot_pulses),
	csw_pilot(q.csw_pilot),
	csw_data(q.csw_data),
	csw_pause(q.csw_pause)
{}


/*	construct new TapData from tap data bytes in buffer
	data in buffer must include blocktype and checksum byte
*/
TapData::TapData( Array<uint8>& q, uint ppilot, Time pause, bool is_zxsp)
:
	TapeData(isa_TapData,original_data),
	data(q),
	is_zxsp(is_zxsp),
	is_jupiter(!is_zxsp),
	pause(pause),
	pilot_pulses(ppilot),
	csw_pilot(0),
	csw_data(0),
	csw_pause(0)
{}



// ###############################################################################
//                      convert
// ###############################################################################


/*	convert TapeData block of any kind to a TapData block
	set's data_trust_level to indicate success or failure
*/
TapData::TapData( TapeData const& q )
:
	TapeData(isa_TapData)
{
	switch(q.isaId())
	{
	case isa_TapData:	new(this) TapData( static_cast<TapData const&>(q) ); break;
	case isa_O80Data:	new(this) TapData(no,no); trust_level = conversion_failed; break;
	case isa_TzxData:	new(this) TapData( static_cast<TzxData const&>(q) ); break;
	default:            new(this) TapData( CswBuffer(q,3500000) ); break;
	}
}

/*	Convert TapData to CswBuffer:
*/
CswBuffer::CswBuffer(TapData const& q, uint32 ccps)
:
	CswBuffer(ccps,(q.pilot_pulses&1)^1,666)	// even => start with hi pulse
{
	xlogIn("new CswBuffer(TapData&)");

	uint32 q_bytes = q.count();
	cu8ptr q_data  = q.getData();

	bool jup = q.is_zxsp ? no : q.is_jupiter ? yes : ccps==timing[1].ccps;
	Timing const& t = timing[jup];

	max  = q.pilot_pulses + 2 /*sync*/ + q_bytes*16 + (1+2*uint32(q.pause*ccps+1)/0xFFFF) + 3;
	data = new uint16[max];

	writePureTone( q.pilot_pulses, t.spp_pilot() );			// pilot
	writePulse( t.spp_sync1() );							// sync
	writePulse( t.spp_sync2() );
	writePureData( q_data, q_bytes*8, t.spp_bit0(), t.spp_bit1() );	// data
	if(jup) writePulse(903.0/3250000.0);
	writeTzxPause( q.pause );								// pause

	seekStart();
}



/*	convert CswBuffer to TapData:
	we must not use methods which use the 'current r/w position' of the CswBuffer
	because this position is used by the tape recorder which may be playing right now
*/
TapData::TapData( CswBuffer const& bu )
:
	TapeData(isa_TapData)
{
	xlogIn("new TapData(CswBuffer&)");
	new(this) TapData(bu.getData(), bu.getTotalPulses(), bu.ccPerSecond());
}


/*	convert CswBuffer to TapData:
	sets trust_level
		on failure to no_data, conversion_failed, truncated_data_error, checksum_error
		on success to decoded_data
	sets is_zxsp or is_jupiter if possible
	sets pilot_pulses
	sets pause
	sets csw_pilot, csw_data and csw_pause for analysis by caller,
		 e.g. to detect two blocks without gap (-> Jupiter Ace)
*/
TapData::TapData( uint16 const* bu, uint32 sz, uint32 ccps )
:
	TapeData(isa_TapData),
	is_zxsp(no),
	is_jupiter(no),
	pause(0),
	pilot_pulses(0),
	csw_pilot(0),
	csw_data(0),
	csw_pause(0)
{
	xlogIn("new TapData(CswBuffer&)");
	if(sz==0) return;

	Timing const& t = timing[ccps==timing[1].ccps];
	double f = (double)ccps / t.ccps;
	uint cc_pilot = uint(t.cc_pilot * f + 0.5);
	uint cc_sync1 = uint(t.cc_sync1 * f + 0.5);
	uint cc_sync2 = uint(t.cc_sync2 * f + 0.5);
	uint cc_bit1  = uint(t.cc_bit1  * f + 0.5);
	uint cc_bit0  = uint(t.cc_bit0  * f + 0.5);

	uint const pulses_per_pilot_sequence_min = 256;						// min expected pilot pulses
	uint const cc_per_pilot_min	 = cc_pilot - (cc_pilot-cc_sync1)/2;	// ultimate limit
	uint const cc_per_sync1_max	 = cc_sync1 + (cc_pilot-cc_sync1)/3;	// ultimate limit
	uint const cc_per_sync12_max = (cc_sync1+cc_sync2) *3/2;	        // ultimate limit for sync1+sync2
	uint const cc_per_bit11_max	 = cc_bit1*2 *11/9;						// ultimate limit for 2 data bit pulses


	uint16 const* q = bu;		// running pulse pointer
	uint16 const* e = q+sz;		// end of pulses


p1:	xlogline("---1---");
	// Erwarte mindestens 256 Pilotpulse ~ 0.15 sec
	//	es wird die Gesamtzeit für eine Phase (2 Pulse) gemessen, um auch stark asynchrone Signale zu erkennen.
	//	es wird jedoch ein 'enges' Limit gesetzt: SOLLLÄNGE*9/10 ... SOLLLÄNGE*10/9
	//	danach wird der Pilotton als gültig anerkannt.


#define NEXTPULSE()		a = b, b = *q++; \
						while(q<e && *q==0 && ++q<e) { b += *q++; }

	uint32 a;			// last pulse length
	uint32 b=0;			// current puls length

p11:
	csw_pilot = q-bu-1;
	for(pilot_pulses=1; pilot_pulses <= pulses_per_pilot_sequence_min; pilot_pulses++)
	{
		if(q==e) return;		// no_data
		NEXTPULSE();

		if( a+b < 2*(cc_pilot*9/10) ) goto p11;	// zu kurz
		if( a+b > 2*(cc_pilot*10/9) ) goto p11;	// zu lang
		if( b < cc_per_pilot_min ) goto p11;	// zu unsymmetrisch => erkennen des syncs ungewiss
	}


	xlogline("---2---");
	// Warte auf den ersten Sync-Puls:
	//	Der Pilot wird solange als gültig anerkannt, wie zwei aufeinander folgende Pulse im Limit liegen.
	//	Es wird ein 'weites' Limit akzeptiert: SOLLLÄNGE*8/10 ... SOLLLÄNGE*10/8.
	//	Sobald ein Puls anliegt, der kurz genug ist, wird der Sync-Puls erwartet.

	while(q<e)
	{
		NEXTPULSE();
		if (b <= cc_per_sync1_max) break;		// sync!
		if( a+b < 2*(cc_pilot*8/10) ) goto p1;	// zu kurz
		if( a+b > 2*(cc_pilot*10/8) ) goto p1;	// zu lang
		pilot_pulses++;
	}


	xlogline("---3---");
	// Erwarte den 2. Sync-Puls:
	//	Der erste Puls war <= SYNC1_MAX.
	//	Beide Pulse müssen <= 2*SYNC1_MAX sein. Zu kurz gibt es nicht.

	if(q==e) { trust_level = conversion_failed; return; }
	NEXTPULSE();

	if(a+b > cc_per_sync12_max) goto p1;		// zu lang


	xlogline("---4---");
	// Pilotton und Sync-Pulse wurden erkannt und eingelesen
	// Jetzt werden die Datenbytes decodiert:

	data.grow( uint32((e-q)/16+1) );
	uint8* z = data.getData();
	uint byte = 0;
	uint bits;
	uint16 const* q0;	// start of byte
	csw_data = q-bu;

	for(;;)				// loop over bytes
	{
		q0 = q;
		csw_pause = q-bu;

		for(bits=0; bits<8; bits++)	// loop over bits
		{
			if(q==e) goto p5;
			NEXTPULSE();
			if(q==e) goto p5;
			NEXTPULSE();

			if(a+b > cc_per_bit11_max) goto p5;		// too long => end recording

			byte += byte + (a+b >= cc_bit0+cc_bit1);
		}
		*z++ = byte;
	}

	// jetzt sollte bits==0 sein
	// a zu lang => typisch ZX Spectrum (if saved from real ZX Spectrum – emulated we don't know)
	// b zu lang => typisch Jupiter Ace (if saved from real Jupiter Ace – emulated we don't know)


p5:	data.shrink(z-data.getData());
	if(data.count()<3) { trust_level = conversion_failed; return; }

//	calc_block_infos();

	bool is_header = data[0]==0;
	is_zxsp    = is_header && isaZxSpectrumHeaderBlock(data.getData(),data.count());
	is_jupiter = is_header && isaJupiterHeaderBlock(data.getData(),data.count());

	uint sum   = calcTapeBlockChecksum(data.getData(),data.count(),false);

	if(!is_zxsp && !is_jupiter && data[0]!=0)
	{
		if(sum==0) is_zxsp = true;							// correct checksum for ZX Spectrum
		else if(sum==data[0]) { is_jupiter=true; sum=0; }	// correct checksum for Jupiter Ace
	}

	// calc pause:
	// Die Pause umfasst ggf. auch den letzten Puls, den die Jupiter Ace Taperoutine speichert!
	uint32 ccpause = 0;
	for(q=q0;q<e;) ccpause += *q++;
	pause = (Time)ccpause / ccps;


	trust_level = sum==0 ? decoded_data				// ok
				: bits>0 ? truncated_data_error
				: checksum_error;
}






// ############################################################


/*  write block to .tap file
	if omit_typebyte then don't write the typebyte at data[0]	(for Jupiter Ace .tap files)
*/
void TapData::writeToFile(FD& fd, bool omit_typebyte ) const throws // file_error,data_error
{
	uint32 n = data.count();
	if(n<2) return;		// too short: min. 2 bytes required for typebyte and checksum => ignore
	if(n>=0x10002) throw data_error(".tap block ≥ $10002 bytes");	// too long

	fd.write_uint16_z(n-omit_typebyte);
	fd.write_bytes(data.getData()+omit_typebyte,n-omit_typebyte);
}


/*  read block from .tap file
	if 'add_typebyte' is set then add space for a typebyte at data[0], initially set to 0x00
		this is to support Jupiter Ace .tap files without typebytes.
		we could already set the typebyte to 0xff for data,
		but the only test for a header block is whether block size == 26
		and a data block might accidentially be 26 bytes long as well.
		So the caller must fix the typebyte.
*/
void TapData::readFromFile(FD &fd, bool add_typebyte) throws // bad_alloc,file_error
{
	assert(data.count()==0);

	uint32 n = fd.read_uint16_z(); if(n+add_typebyte<2) n += 0x10000;
	data.grow(n+add_typebyte);
	fd.read_bytes(data.getData()+add_typebyte,n);
	trust_level = TrustLevel::original_data;
}


/*  write data[] to .tap file
	TODO: flag for Jupiter Ace .tap file: save typebyte yes or no?
	currently the typebyte is saved.
	This is probably incompatible with other Jupiter Ace emulators but ok with zxsp.
*/
//static
void TapData::writeFile( cstr fpath, TapeFile& data ) throws // file_error,data_error,bad_alloc
{
	xlogIn("TapData::writeFile(%s)",fpath);

	FD fd; fd.open_file_w(fpath);			// throw file_error

	for(uint i=0; i<data.count(); i++)
	{
		TapeFileDataBlock* db = data[i];
		db->tapdata =
			db->tapdata  ? db->tapdata :
			db->o80data  ? nullptr :
		//	db->tapedata ? new TapData(*db->tzxdata) :		sollte unnötig sein!
			db->tzxdata  ? new TapData(*db->tzxdata) :
						   new TapData(*db->cswdata);

		// note: auch ein misslungener TapData-Block bleibt erhalten
		// => zukünftige Konvertierungsversuche unnötig

		if(db && db->tapdata->trust_level >= truncated_data_error)
			db->tapdata->writeToFile(fd,no);
	}
}


/*  read data[] from .tap file
	static
*/
void TapData::readFile( cstr fpath, TapeFile& data ) throws // file_error,data_error,bad_alloc
{
	xlogIn("TapData::readFile(%s)",fpath);
	assert(data.count()==0);

	FD fd(fpath,'r');					// throw file_error
	if(fd.file_size()<2) return;		// empty file

	uint16 len = fd.read_uint16_z();
	fd.rewind_file();

	bool is_jupiter = (len|1) == 27;	// Jupiter Ace .tap file with or without typebytes
	bool add_typebyte = is_jupiter && len==26;	// Jupiter Ace
	int  typebyte = 0x00;				// Jupiter Ace: start with 0x00 = Header, toggle with 0xFF = Data
	Timing const& t = timing[is_jupiter];

	while( fd.file_remaining()>2 )
	{
		xlogline("read block...");
		TapData* tapdata = new TapData(!is_jupiter,is_jupiter);
		tapdata->readFromFile(fd, add_typebyte);

		if(add_typebyte) tapdata->data[0] = typebyte;
		else             typebyte = tapdata->data[0];

		bool is_data = is_jupiter ? typebyte!=0 : typebyte&0x80;
		//assert(is_data<=1);
		tapdata->pause = t.pause[is_data];
		tapdata->pilot_pulses = t.pilot_pulses[is_data];
		data.append(tapdata);
//		tapdata->calc_block_infos();

		typebyte ^= 0xFF;
	}
}


cstr TapData::calcMajorBlockInfo() const noexcept
{
	return calcMajorTapBlockInfo(data.getData(),data.count());
}

cstr TapData::calcMinorBlockInfo() const noexcept
{
	return calcMinorTapBlockInfo(data.getData(),data.count());
}




































