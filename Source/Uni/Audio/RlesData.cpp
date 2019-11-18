/*	Copyright  (c)	Günter Woigk 2000 - 2018
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


	class RlesData
	--------------

	run-length encoded 1-bit data block
	- read from rles file
	- recorded in the tape recorder when set to 1-bit mode

	can be imported directly from
	- mono data
	- stereo data
	- tzx or pzx
	- tap, o80 or p81

	can be played and recorded by the tape recorder

note:
	Framework	AudioToolbox/AudioToolbox.h
	Declared in	AudioToolbox/AudioFile.h


// ---


	buffer<uchar>			rles Daten
	samples_per_second		Samplerate für diesen Block

	Der Puffer enthält abwechselnd eine Lauflängenangabe für die High-Phase und die Low-Phase.
	Der Puffer startet mit der Low-Phase, ggf. mit Lauflänge '0'.
	Mehrere Phasen gleicher Auslenkung können aneinandergehängt werden,
	wenn man für das gegenteilige Sample dazwischen als Lauflänge 0 einträgt.

	pilot_start			Sample-Position des Beginns des Nutzsignals (idR. Pilot-Ton)
	data_start			Sample-Position des Beginns der Nutzdaten (idR. nach dem Sync-Bit)
	data_end			Sample-Position des Endes   des Nutzsignals

Play() == Read():

	buffer.Pos()		nächstes zu lesendes rles-Byte
	total_samples		insgesamt im Buffer gespeicherte Samples
	time_akku			noch zu spielende Zeiten "last_sample" bis wieder ein Byte gelesen werden muss.
	last_sample			zuletzt gespieltes Sample. Für Signalabschwächung gegen 0 in Pausen.

	aktuelle Phase	  = buffer.Pos() & 1  ?  "low"  :  "high"
	Gesamtspielzeit   =	total_samples/samples_per_second

Record() = Write():

	buffer.Pos()		Position für nächstes zu schreibendes rles-Byte
	time_akku			Summierer für Zeiten "high" oder "low" bis wieder ein Byte geschrieben werden muss.
	last_sample			zuletzt gelesenes Sample
	total_samples		invalid

	aktuelle Phase    = buffer.Pos() & 1  ?  "high"  :  "low"

Notes:

	Beim Speichern von time_akku ändert sich die Laufzeit wegen der Sampleraten-Anpassung.
	Damit ändert sich in dem Moment die exakte aktuelle Position und Gesamtspiellänge.
	Nach Schreiben eines n-Sample-Puffers mit Write() erhöht sich die aktuelle Position deshalb nicht exakt um n Samples!
*/

#include <zlib.h>
#include <math.h>
#include "StereoSample.h"
#include "TapeFile.h"
#include "TapeFileDataBlock.h"
#include "RlesData.h"
#include "TapData.h"
#include "TzxData.h"
#include "O80Data.h"
#include "AudioData.h"
#include "Dsp.h"
#include "TapeRecorder.h"
#include "globals.h"








//					disfunct










/* ---- ZX Spectrum tape loader timing ----
		generell: 0-Bits werden mit 2 kHz, 1-Bits mit 1 kHz gespeichert.
		Pilot-Phasen sind etwas länger als 1-Bits,
		das Sync-Bit ist etwas kürzer als ein 0-Bit.
		Signale können beim Lesen invertiert sein.
		Durch schlechte Aufnahmen verschieben sich Taktflanken z.T. gewaltig.

Messwerte am emulierten ZX Spectrum +128 @ 44.1 kHz:
		Pilot	   27 Samples
		Sync	8 + 9 Samples
		0-Bit	   11 Samples
		1-Bit	   22 Samples

lt. RamSoft "ZX Spectrum Loaders Guide":
		Sync	667+735 cpu cycles  ~ 2623+2381 Hz
		0-Bit		855 cpu cycles  ~ 2047 Hz
		1-Bit	   1710 cpu cycles  ~ 1023 Hz
		Pilot	   2168 cpu cycles  ~  807 Hz
*/
//Frequency const	cc_per_second	= 3500000.0;	// cpu cycles per second
//uint32 const	cc_pilot	= 2168;				// cpu cycles per pilot pulse
//uint32 const	cc_bit0	=  855;				// cpu cycles per 0-bit pulse
//uint32 const	cc_bit1	= 1710;				// cpu cycles per 1-bit pulse
//uint32 const	cc_sync1	=  667;				// cpu cycles for 1st sync pulse
//uint32 const	cc_sync2	=  735;				// cpu cycles for 2nd sync pulse
//Time  const	secs_per_pilot	= cc_pilot / cc_per_second;		// duration of one pulse
//Time  const secs_per_sync1	= cc_sync1 / cc_per_second;		// duration of one pulse
//Time  const secs_per_sync2	= cc_sync2 / cc_per_second;		// duration of one pulse
//Time  const secs_per_bit0	= cc_bit0 / cc_per_second;		// duration of one pulse
//Time  const secs_per_bit1	= cc_bit1 / cc_per_second;		// duration of one pulse



// ################################################################################



RlesData::RlesData()
: TapeData(isa_RlesData)
{}

RlesData::~RlesData()
{
}

RlesData::RlesData( TapeData const& q ) noexcept(false) // data_error
:
	TapeData(isa_RlesData)
{
	switch( q.isaId() )
	{
		case isa_TapData:	new(this) RlesData(static_cast<TapData const&>(q)); break;
		case isa_TzxData:	new(this) RlesData(static_cast<TzxData const&>(q)); break;
		case isa_O80Data:	new(this) RlesData(static_cast<O80Data const&>(q)); break;
		case isa_RlesData:	new(this) RlesData(static_cast<RlesData const&>(q)); break;
		case isa_AudioData: new(this) RlesData(static_cast<AudioData const&>(q)); break;
		default:            IERR();
	}
}


CswBuffer::CswBuffer(RlesData const& , uint32 ccps)
:
	ccps(ccps)
{
	TODO();
}


// #######################################################################################
//							import from other *Data
// #######################################################################################


RlesData::RlesData( TzxData const& ) noexcept(false) // data_error
:
	TapeData(isa_RlesData)
{
	TODO();
}

RlesData::RlesData( TapData const& ) noexcept(false) // data_error
:
	TapeData(isa_RlesData)
{
	TODO();//buffer.startRecordingPulses();
//	buffer.writeZxspData( q.data.Data(), q.data.Count()*8, 1.000 /*pause*/ );
//	buffer.stop();
}

RlesData::RlesData( O80Data const& ) noexcept(false) // data_error
:
	TapeData(isa_RlesData)
{
	TODO();//buffer.startRecordingPulses();
//	buffer.writeZx80Data( q.data.Data(), q.data.Count() );
//	buffer.stop();
}

RlesData::RlesData(class AudioData const& ) noexcept(false) // data_error
:
	TapeData(isa_RlesData)
{
	TODO();
}

RlesData::RlesData(RlesData const& ) noexcept(false) // data_error
:
	TapeData(isa_RlesData)
{
	TODO();
}



// #######################################################################################

#if 0
void RlesData::calcBlockInfos()
{
	uint samples_per_bit_0	= int( cc_per_bit_0 / cc_per_second * buffer.samplesPerSecond() +0.5 );
	uint samples_per_bit_1	= int( cc_per_bit_1 / cc_per_second * buffer.samplesPerSecond() +0.5 );
	uint limit01 = samples_per_bit_0+samples_per_bit_1;

	uint8 bu[20];
	uint8 const* q = buffer.getData()+data_start;

	int data_bytes = (data_end-data_start)/16;	// TODO: calc data_start and data_end
	int imax = Min(data_bytes,20);

	for( int i=0; i<imax; i++ )
	{
		uchar byte = 0;
		for( int j=0;j<8;j++ ) { byte += byte + (uint(q[0])+uint(q[1])>=limit01); q+=2; }
		bu[i] = byte;
	}

	setMajorBlockInfo( TapData::calcMajorBlockInfo( data_bytes, bu ) );
	setMinorBlockInfo( TapData::calcMinorBlockInfo( data_bytes, bu ) );
}
#endif

////virtual
//cstr RlesData::getMajorBlockInfo()
//{
//    TODO();
//}

////virtual
//cstr RlesData::getMinorBlockInfo()
//{
//    TODO();
//}



// #######################################################################################
//							tape recorder play() / record()
// #######################################################################################



//void RlesData::startPlaying(Time)
//{
//    buffer.start(TapeFile::playing|TapeFile::edges);
//    recording = no;
//}

//void RlesData::startRecording(Time)
//{
//    assert(!recording);
//    purge();
//    buffer.grow(1 MB);
//    buffer.start(TapeFile::recording|TapeFile::edges);
//    recording = yes;
//}


/*	; info text for display: (multiple, optional)
		dc.m	"info"
		dc.l	length					; total length of data following
		dc.m	"info text",$00			; UTF-8 with c-style delimiter
		dc.s	padding					; optional some padding

	; Raw data: (multiple)
		dc.m	"rles"
		dc.l	length					; total length of data [bytes] following
		dc.l	sampling_frequency		; frequency in hertz
		dc.s	rles_data				; one high and low phase per byte

	; Header of a file concatenated by the user:
		dc.m	"RlesTape"				; file magic
		dc.m	"1.0", $00				; major + minor version
*/
/*static*/
void RlesData::readFile (cstr /*fpath*/, TapeFile& /*tapeblocks*/) throws
{
	TODO();
#if 0
	xlogIn("RlesData::readFile(%s)",fpath);

// read file:
	FD fd(fpath,'r');				// throw file_error
	off_t flen = fd.file_size();				// throw file_error
	ADA<uint8> data; data  = (uint8*)tempmem(flen);	// throw bad_alloc
	fd.read_data(data+0,flen);					// throw file_error

// header:
//		dc.m	"RlesTape"				; 8 bytes: file magic
//		dc.m	"1.0", $00				; 4 bytes: major + minor version, delimiter
	{
		if( flen<12 || !eq(substr(data+0,data+8),"RlesTape") ) throw data_error("not a rles file");
		char version_h = data[8];
		//char version_l = data[10];
		if(version_h!='1') throw data_error("rles: version %s not supported",substr(data+8,data+11));
		//metadata->AddData("file_type","rles");
		//metadata->AddData("version_h",CharStr(version_h));
		//metadata->AddData("version_l",CharStr(version_l));
	}

	u8ptr q = data+0;
	u8ptr e = q+flen;
	RlesData* rlesdata = NULL;
	cstr info;
	uint32 id,len;

	for( ;e-q>=8; q+=len )
	{
		id  = peek4Z(q); q+=4; if(id=='Rles') { len=8; continue; }
		len = peek4Z(q); q+=4; if(uint32(e-q)<len) throw data_error("rles: premature end of file");
		if(!rlesdata)
		{
			rlesdata = new RlesData();
			tapeblocks.append(rlesdata);
			info=NULL;
		}

		switch(id)
		{
		case 'info':
			if(len&&len<=80)
			{
				q[len-1] = 0;		// security
				info = (str)q;
				break;
			}
			throw data_error("rles: broken 'info' chunk");

		case 'rles':
			{
				TODO();
//				uint32 sps = Peek4Z(q);
//				xlogline( "rles compressed audio, f=%lu Hz, %lu samples", sps, uint32(len) );
//				if( sps<8000 || sps>100000 )
//				{ throw data_error("rles: sample frequency out of range"); }
//				rlesdata->buffer.setSamplesPerSecond( sps );

//				TODO();//rlesdata->buffer.startRecordingPulses();
//				for( u8ptr p=q+4, e=p+len-1; p<=e; p++ )
//				{
//					uchar byte = *p;
//					rlesdata->buffer.write( byte&0x0f || p==e ? (byte>>4) : (byte>>4)*15, 1 );
//					rlesdata->buffer.write( byte&0xf0 || p==q ? (byte&15) : (byte&15)*15, 0 );
//				}
//				rlesdata->buffer.stop();
//				if(info) rlesdata->setMajorBlockInfo(info);
//				rlesdata = NULL;
//				break;
			}
		}
	}
#endif
}


/*static*/
void RlesData::writeFile (cstr fpath, TapeFile &tapedata ) noexcept(false) // file_error,data_error,bad_alloc
{
	xlogIn("RlesData::writeFile(%s)",fpath);
	TODO();(void)tapedata;
	(void)fpath;
}


#if 0
void RlesData::WriteToFile( int fd ) const throws			// .rles file
{
	if(buffer.Size()==0) return;

	if(MajorBlockInfo())
	{
		write_ulong_z(fd,'info');
		uint32 len = strlen(MajorBlockInfo())+1;
		write_ulong_z(fd,len);
		write_bytes(fd,MajorBlockInfo(),len);
	}

	write_ulong_z(fd,'rles');
	TODO();	//	write_ulong_z(fd,buffer.Size()+4);
	write_ulong_z(fd,uint32(samples_per_second));
	TODO();	//	write_bytes(fd,buffer.Data(),buffer.Size());
}
#endif


//uint32 RlesData::samplesPerSecond()
//{
//	return 0;	// TODO
//}

















