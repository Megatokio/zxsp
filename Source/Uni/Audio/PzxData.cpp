/*	Copyright  (c)	Günter Woigk 2000 - 2012
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




#define SAFE	3
#define	LOG 	1

#include "unix/file_utilities.h"
#include "unix/tempmem.h"
#include "PzxData.h"
#include "TapData.h"


void PzxData::kill()
{}

void PzxData::init()
{
	data.Purge();
	current_position = total_playtime = 0.0;
}

void PzxData::init( TapeData const& q )
{
	switch(q.isaId())
	{
	case isa_PzxData:	init( static_cast<PzxData const&>(q) );	 break;
	case isa_RlesData:	init( static_cast<RlesData const&>(q) ); break;
	case isa_MonoData:
	case isa_StereoData:
	default:			init( RlesData(q) ); break;
	}
}

void PzxData::init( PzxData const& q )	// shared data
{
	data = q.data;
	current_position = q.current_position;
	total_playtime   = q.total_playtime;
}


void PzxData::purge()
{
	current_position = total_playtime = 0.0;
	data.Purge();
	TapeData::purge();
}

// ############################################################################


void PzxData::calcTotalPlaytime()
{
	total_playtime = 0.0;
	cu8ptr q = data.Data();
	cu8ptr e = q + data.Count();

	while(q<e)
	{
		uint32 id   = Peek4X(q); q += 4;
		uint32 blen = Peek4Z(q); q += 4;
		cu8ptr p   = q;			q += blen;

		switch(id)
		{
		case 'DATA':
			{
				uint32   bits = Peek4Z(p) & 0x7fffffff;	p += 4;				// total bits in data
				uint    tail = Peek2Z(p);	p += 2;							// cc for trailing pulse
				uint    cnt0 = Peek1Z(p);	p += 1;							// pulses for 0-bits
				uint    cnt1 = Peek1Z(p);	p += 1;							// pulses for 1-bits
				Time cc0 = 0.0; while(cnt0--) { cc0 += Peek2Z(p); p+=2; }	// accumulate cc for 0-bits
				Time cc1 = 0.0; while(cnt1--) { cc1 += Peek2Z(p); p+=2; }	// accumulate cc for 1-bits
				uint32 b1 = CountBitsL(p,bits);								// count 1-bits
				total_playtime += ( b1*cc1 + (bits-b1)*cc0 + tail ) / 3500000.0;
			}	break;

		case 'PULS':
			while( p<e )
			{
				uint32 cnt = 1, dur = Peek2Z(p);
				if( dur > 0x8000  ) { cnt = dur & 0x7FFF; dur = Peek2Z(p+2);  }
				if( dur >= 0x8000 ) { dur = ((dur & 0x7FFF)<<16) + Peek2Z(p+4); }
				total_playtime += float(cnt) * dur/3500000.0;
			}	break;

		case 'PAUS':
			{
				uint32 cc = Peek4Z(p) & 0x7fffffff;
				total_playtime += cc / 3500000.0;
			}	break;
		}
	}
}

void PzxData::calcBlockInfos()
{
	u8ptr q = data.Data();
	u8ptr e = q + data.Count();
	while(q<e)
	{
		uint32 id   = Peek4X(q); q+=4;
		uint32 blen = Peek4Z(q); q+=4;
		cu8ptr p   = q;			q += blen;

		switch(id)
		{
		case 'BRWS':
			{
				setMajorBlockInfo( (cstr)p );
			}
		case 'DATA':
			{
				uint32   bits = Peek4Z(p) & 0x7fffffff;	p += 4;	// total bits in data
											p += 2;				// cc for trailing pulse
				uint    cnt0 = Peek1Z(p);	p += 1;				// pulses for 0-bits
				uint    cnt1 = Peek1Z(p);	p += 1;				// pulses for 1-bits
						p += 2*cnt0 + 2*cnt1;					// skip pulse tables

				if(cnt0==2&&cnt1==2)
				{
					if(!major_block_info) setMajorBlockInfo( TapData::calcMajorBlockInfo(bits/8,p) );
					setMinorBlockInfo( TapData::calcMinorBlockInfo(bits/8,p) );
				}
				else if(cnt0==8&&cnt1==18)
				{
					if(!major_block_info) setMajorBlockInfo("ZX80/ZX81 program block");
					setMinorBlockInfo( usingstr("%i bytes",int(bits/8)) );
				}
			}
		case 'STOP':
			{
				int flag = Peek2Z(p);
				if(!major_block_info) setMajorBlockInfo( flag==0 ? "Stop the tape" : "Stop tape if loading into 48k model");
			}
		}
	}
}


// ############################################################################



			// PZX header block:
						//		dc.b	major version number
						//		dc.b	minor version number
						//		dc.s	program name
						//	n*2*dc.s	info  dictionary:
						//
						//	Tag        TZX ID   Info
						//  ---------- ------ - ------------------------
						//	Publisher  [0x01] - Software house/publisher
						//	Author     [0x02] - Author(s)
						//	Year       [0x03] - Year of publication
						//	Language   [0x04] - Language
						//	Type       [0x05] - Game/utility type
						//	Price      [0x06] - Original price
						//	Protection [0x07] - Protection scheme/loader
						//	Origin     [0x08] - Origin
						//	Comment    [0xFF] - Comment(s)
						//	Some keys, like Author or Comment, may be used more than once.
						//


// ############################################################################


//void PzxData::WriteToFile( int fd ) const throw(file_error)
//{
//	write_bytes( fd, data.Data(), data.Size() );
//}


#if 0
// read pzx block from pzx file
//	validates and stores only valid data
//	returns after PAUS, STOP or file end
//	on file end, an empty block is returned
//	only the chunks BRWS, PULS, DATA, PAUS and STOP are stored.
//
void PzxData::ReadFromFile( int fd ) throw(bad_alloc,file_error,data_error)
{
	XLogIn("PzxData::ReadFromFile");

	init();
	int32  old_sz = 0;
	u8ptr q,e;

a:	off_t flen = file_remaining(fd); if(flen<8) return;	// eof

	data.Grow(old_sz+8);
	q = data.Data()+old_sz;
	read_data( fd, q, 8 );
	int32 id   = Peek4X(q);
	int32 blen = Peek4Z(q+4); if( blen>16 MB ) throw(data_error("pzx: illegal blen"));

	data.Grow( old_sz+8+blen+20 );		// grow & add spare fore late end-of-block tests
	q = data.Data()+old_sz+8;			// read pointer
	e = q+blen;							// end of block data

#define SET_BLEN(N) Poke4Z( data.Data()+old_sz+4, (blen=(N)) )

	switch(id)
	{
	case 'PULS':	// arbitrary repetitions of pulses
	{
		XXLogIn("PULS block");
		// [cnt,dur] pairs follow until block end.
		// incomplete tupel at block end is silently discarded

		while( q<e )
		{
		// decode duration and repeat count acc. to code sample in specs:
			u8ptr q0=q; uint32 cnt = 1, dur = Peek2Z(q); q+=2;
			if( dur > 0x8000  ) { cnt = dur & 0x7FFF; dur = Peek2Z(q); q+=2; }
			if( dur >= 0x8000 ) { dur = ((dur & 0x7FFF)<<16) + Peek2Z(q); q+=2; }
			if(q>e)
			{
				XXLogLine("incomplete final pulse");
				SET_BLEN( blen-(e-q0) );
				break;
			}
			XXLogLine("%u x %u cc", int(cnt), int(dur) );
		}
		old_sz += 8+blen;
	}	goto a;

	case 'DATA':
	{
		XXLogIn("DATA block");

		uint32   bits = Peek4Z(q) & 0x7fffffff;	q += 4;	// total bits in data
		XXLogLine("%i bytes + %i bits", int(bits/8),int(bits%8));
		uint    tail = Peek2Z(q);	q += 2;			// trailing pulse
		uint    cnt0 = Peek1Z(q);	q += 1;			// pulses for bit==0
		uint    cnt1 = Peek1Z(q);	q += 1;			// pulses for bit==1
		uint16*	dur0 = (uint16*)q;	q += 2*cnt0;	// table of pulse durations for bit 0
									if(q>e) q=e+1;	// security: avoid reading off-buffer
		uint16*	dur1 = (uint16*)q;	q += 2*cnt1;	// table of pulse durations for bit 1

		int32 req_sz = (bits+7)/8;
		if( e-q != req_sz )
		{
			if( e-q < req_sz ) throw(data_error("pzx: DATA too short"));
			else { SET_BLEN( blen-(e-q)+req_sz); }
		}

		if(XXLOG){
			LogLine("pulses for bit 0: %i", int(cnt0));
			for(uint i=0;i<cnt0;i++) Log("%i ",int(Peek2Z(dur0+i))); Log(" cc\n");
			LogLine("pulses for bit 1: %i", int(cnt1));
			for(uint i=0;i<cnt1;i++) Log("%i ",int(Peek2Z(dur1+i))); Log(" cc\n");
			LogLine("length of tail pulse: %i cc", int(tail));
		}

		old_sz += 8+blen;
	}	goto a;

	case 'BRWS':
	{
		XXLogIn("BRWS block");
		if(blen<=1) { old_sz=old_sz; goto a; }
		*e=0; u8ptr p; for( p=q; *p; p++ ) { if(*p<' ') *p=' '; }	// replace ctl codes
		if(++p!=e) { SET_BLEN(blen-(e-p)); }
		XXLogLine( "%s",(ptr)q );
		old_sz += 8+blen;
	}	goto a;

	case 'PAUS':
	{
		XXLogIn("PAUS block");
		if(blen<4) throw(data_error("pzx: PAUS too short"));
		if(blen>4) { SET_BLEN(4); }
		old_sz += 8+blen;
	}	break;		// block end

	case 'STOP':
	{
		XXLogIn("STOP block");
		if(blen<2) throw(data_error("pzx: STOP too short"));
		if(blen>2) { SET_BLEN(2); }
		old_sz += 8+blen;
	}	break;		// block end

	default:
		XXLogLine( "unexpected block type $%8lx", id );
		old_sz = old_sz;	// skip
		goto a;
	}				// switch(id)

	data.Shrink(old_sz);
	calc_total_playtime();
	return;			// ok
}
#endif

void PzxData::init( RlesData const& )
{
	TODO();
}

/*static*/
void PzxData::readFile( cstr fpath, AoP<TapeData>& tapeblocks, MetaData& metadata ) throw(file_error,data_error,bad_alloc)
{
	XLogIn("PzxData::readFile(%s)",fpath);
	(void)tapeblocks;
	(void)metadata;
	TODO();
}

/*static*/
void PzxData::writeFile( cstr fpath, AoP<TapeData>& tapeblocks, MetaData& metadata ) throw(file_error,data_error,bad_alloc)
{
	XLogIn("PzxData::writeFile(%s)",fpath);
	(void)tapeblocks;
	(void)metadata;
	TODO();
}



// #######################################################################################
//							export to RlesData
// #######################################################################################


void RlesData::init( PzxData const& qq ) throw(data_error)
{
	init();
	buffer.startRecordingPulses();

	cu8ptr qptr = qq.data.Data();
	cu8ptr qend = qptr + qq.data.Count();

a:	if(qend-qptr<8) { buffer.stop(); return; }	// end of data
	int32 id   = Peek4X(qptr); qptr+=4;
	int32 blen = Peek4Z(qptr); qptr+=4;	if( blen>qend-qptr ) throw(data_error("pzx: chunk size exceeds data"));
	cu8ptr q  = qptr;
	cu8ptr e  = qptr+=blen;

	switch(id)
	{
		case 'ZXTP':	// old PZX header (draft)
			XXLogLine("ZXTP block");
			throw( data_error("pzx: draft version pre 1.0. no longer supported.") );

		case 'PZXT':	// PZX header block:
		{				//		dc.b	major version number
						//		dc.b	minor version number
						//		dc.s	info  dictionary
						//
			XXLogIn("PZXT block");
			if(blen<2) throw(data_error("pzx: PZXT chunk too short"));
			uchar vh=*q++;			// major version number
			uchar vl=*q++;			// minor version number
			XXLogLine("version %hhu.%hhu", vh,vl );
			if( vh!=1 ) throw(data_error(usingstr("pzx: version %hhu.%hhu is not supported.",vh,vl)));
		}	goto a;

		case 'PULS':	// arbitrary repetitions of pulses
		{
			XXLogIn("PULS block");
			buffer.setPhase(0);		// first pulse is low
			while( q<e )
			{
			// decode duration and repeat count acc. to code sample in specs:
				uint32 cnt = 1, dur = Peek2Z(q); q+=2;
				if( dur > 0x8000  ) { cnt = dur & 0x7FFF; dur = Peek2Z(q); q+=2; }
				if( dur >= 0x8000 ) { dur = ((dur & 0x7FFF)<<16) + Peek2Z(q); q+=2; }
				if(q>e) { XXLogLine("truncated pulse"); goto a; }
			// store pulses:
				if(dur!=0) { while(cnt--) { buffer.writeCC(dur); } }
				else if(cnt&1) buffer.write(0);		// if cnt==1 and dur==0 then toggle phase
			}
		}	goto a;

		case 'DATA':
		{
			XXLogIn("DATA block");
			uint32   bits = Peek4Z(q);	q += 4;		// total bits in data
			bool    initial_phase = (int32)bits<0; bits &= 0x7fffffff;
			uint    tail = Peek2Z(q);	q += 2;		// trailing pulse
			uint    cnt0 = Peek1Z(q);	q += 1;		// pulses for bit==0
			uint    cnt1 = Peek1Z(q);	q += 1;		// pulses for bit==1
		#if defined(_LITTLE_ENDIAN)
			uint16*	dur0 = (uint16*)q;	q += 2*cnt0;// table of pulse durations for bit 0
			uint16*	dur1 = (uint16*)q;	q += 2*cnt1;// table of pulse durations for bit 1
		#elif defined(_BIG_ENDIAN)
			uint16*	dur0 = tempMem(2*cnt0); for( uint i=0; i<cnt0; i++ ) { dur0[i] = Peek2Z(q); q+=2; }
			uint16*	dur1 = tempMem(2*cnt1); for( uint i=0; i<cnt1; i++ ) { dur1[i] = Peek2Z(q); q+=2; }
		#else
			#error booboo
		#endif

			if( q + (bits+7)/8 > e ) throw(data_error("pzx: DATA chunk size mismatch"));
			buffer.setPhase(initial_phase);			// first pulse polarity
			if( !data_start ) data_start = buffer.getPos();

			while( bits )
			{
				uchar byte = *q++;
				uint32 n = Min(8u,bits); bits -= n;
				while( n-- )
				{
					if((byte>>n)&1) for( uint i=0; i<cnt1; i++ ) { buffer.writeCC(dur1[i]); }
					else			for( uint i=0; i<cnt0; i++ ) { buffer.writeCC(dur0[i]); }
				}
			}
			if(tail) buffer.writeCC(tail);
		}	goto a;

		case 'PAUS':
		{
			XXLogIn("PAUS block");
			int32 dur = Peek4Z(q); q+=4;
			bool initial_phase = dur<0; dur &= 0x7fffffff;
			if( q>e ) throw(data_error("pzx: PAUS chunk too short"));
			data_end = buffer.getPos();
			if(dur) buffer.writeCC(dur,initial_phase);
		}	goto a;

		case 'BRWS':
			XXLogLine("BRWS block");
			goto a;

		case 'STOP':
			XXLogLine("STOP block");
			buffer.writeTime(5.0);
			goto a;

		default:
			XXLogLine( "unknown block #$%08lX", id );
			goto a;

	}	// switch(id)

	return;				// ok
}






























