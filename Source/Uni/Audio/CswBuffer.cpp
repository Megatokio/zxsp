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
*/

#include "CswBuffer.h"
#include "TapeData.h"
#include "globals.h"
#include "StereoSample.h"



/*
                    _________________________                  __________
                   |                         |                |
    _______________|  <-- current pulse -->  |________________|
                   ^
                   |
                  out
                  0->1

                    _________________________                  __________
                   |                         |                |
    _______________|  <-- current pulse -->  |________________|
                            ^         ^
                      . . . | . . . . | . .
                            in        in
                   ^
                   |
                   pos/cc_pos
*/


// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//                      c'tor, d'tor
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&


CswBuffer::CswBuffer (uint32 ccps, bool phase0, int)
:
    data(NULL),
    max(0),
	end(0),
	cc_end(0),
	ccps(ccps),
    recording(no),
    pos(0),
    cc_pos(0),
    phase(phase0),
    cc_offset(0)
{}

/*  purge contents:
	preserves phase0
*/
void CswBuffer::purge() noexcept
{
	phase = getPhase0();	// current phase

    delete[] data;
    data    = NULL;         // buffer
    max     = 0;            // allocated size
	end     = 0;            // used size
	cc_end  = 0;            // total samples up to 'end'
    recording = no;

    pos     = 0;            // current index in 'data'
    cc_pos  = 0;            // total samples up to 'pos' (excl.)
    cc_offset = 0;          // current CC offset inside current sample
}


CswBuffer::CswBuffer(TapeData const& q, uint32 ccps)
{
    xlogIn("new CswBuffer(TapeData&)");

    switch(q.isaId())
    {
    case isa_TapData:   new(this) CswBuffer(TapDataRef(q), ccps); break;
    case isa_TzxData:   new(this) CswBuffer(TzxDataRef(q), ccps); break;
    case isa_O80Data:   new(this) CswBuffer(O80DataRef(q), ccps); break;
    case isa_RlesData:  new(this) CswBuffer(RlesDataRef(q), ccps); break;
    case isa_AudioData:	new(this) CswBuffer(AudioDataRef(q), ccps); break;
    default:            IERR();
    }
}


/*	create CswBuffer from other CswBuffer
	may be used to resample
*/
CswBuffer::CswBuffer( CswBuffer const& q, uint32 ccps)
:
    data(NULL),
    max(q.end+80),		// add some extra, e.g. allow appending pause without realloc
	end(q.end),
	cc_end(q.cc_end),
	ccps(ccps),
    recording(no),
    pos(0),
    cc_pos(0),
    phase(q.getPhase0()),
    cc_offset(0)
{
	if(ccps==q.ccps)	// no resampling
	{
		data = new uint16[max];
		memcpy(data,q.data,end*sizeof(uint16));
		return;
	}

	// else: resampling required:

	// conversion factor q -> this
	double f = (double)ccps / q.ccps;

	// if f>1 then values grow and may exceed $FFFF and require more bytes!
	// this is especially true for long sequences of silence.
	// => adjust 'max':
	const uint cc_max = uint(65535/f);		// cc_max < 65536/f - 0.5
	for(uint32 i=0; i<end;)
	{
		uint32 cc = q.data[i++];
		while(i+1<end && q.data[i]==0) { i++; cc += q.data[i++]; max-=2; }
		if(cc>cc_max) max += (uint32(cc*f+0.5))/0xffff *2;
	}

	// allocate:
	data = new uint16[max];
	cc_end = 0;

	// resample and store data:
	uint32 zi=0, qi=0;
	while(qi<end)
	{
		uint32 cc = q.data[qi++];
		while(qi+1<end && q.data[qi]==0) { qi++; cc += q.data[qi++]; }
		cc = uint32(cc*f+0.5);
		cc_end += cc;
		while(cc>0xffff) { data[zi++] = 0xffff; data[zi++] = 0; cc -= 0xffff; }
		data[zi++] = cc;
	}
	end = zi;
	assert(zi<=max);
}




// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

/*  step foreward to next pulse
    don't call at buffer end
*/
//private
inline void CswBuffer::skip() const noexcept
{
    assert(pos<end);

    phase = !phase;
    cc_pos += data[pos++];
}

/*  step backward to previous pulse
    don't call at buffer start
*/
//private
inline void CswBuffer::rskip() const noexcept
{
    assert(pos>0);

    phase = !phase;
    cc_pos -= data[--pos];
}

/*  grow buffer
    may shrink buffer as well,
    but new 'max' size must be ≥ current 'end' size
*/
//private
void CswBuffer::grow(uint32 n) throws/*bad alloc*/
{
    assert(n>=end);

    uint16* z = new uint16[n];
    memcpy(z,data,end*sizeof(uint16));
    delete[] data;
    data = z;
    max  = n;
}


/*  grow the data[] buffer:
    does not shrink the buffer
*/
//public
void CswBuffer::growBuffer(uint32 n) throws/*bad alloc*/
{
    if(n>max) grow(n);
}


/*  shrink buffer to fit the actually used size
    if reallocating the buffer fails, nothing is done (fail safe)
        this is highly unlikely nowadays
*/
void CswBuffer::shrinkToFit() noexcept
{
    try { if(max>end) grow(end); }
    catch(bad_alloc&){}
}


// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//                  Moving the Current Position
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

/*  move 'pos/cc_pos' to start of this buffer
*/
void CswBuffer::seekStart() const noexcept
{
    if(pos&1) phase = !phase;
    cc_offset = cc_pos = pos = 0;
}

/*  move 'pos/cc_pos' to end of this buffer
*/
void CswBuffer::seekEnd() const noexcept
{
    if((end-pos)&1) phase = !phase;
	pos = end;
	cc_offset = 0;
	cc_pos = cc_end;
}

/*  move 'cc_pos' to the given position
    moves 'pos/cc_pos' to the pulse which contains the given cc:
        cc_pos[pos] ≤ cc < cc_pos[pos+1]
*/
CC CswBuffer::seekCc(CC cc) const noexcept
{
    if(recording)
    {
        assert(cc>=cc_pos+cc_offset);
    }
    else
    {
        if(cc==0)      { seekStart(); return 0; }
        if(cc>=cc_end) { seekEnd(); return cc_end; }

        while(cc>cc_pos) skip();
        while(cc<cc_pos) rskip();
    }
    cc_offset = cc-cc_pos;
    return cc;
}

/*	move pos to the given buffer index
	moves pos to the newpos updating cc_pos and pulse
	cc_offset is set to 0 == start of pulse
*/
void CswBuffer::seekPos( uint32 newpos ) const noexcept
{
	assert(!recording);

	if(newpos==0)	{ seekStart(); return; }
	if(newpos>=end)	{ seekEnd();   return; }

	if((newpos^pos)&1) phase = !phase;

	while(newpos>pos) cc_pos += data[pos++];
	while(newpos<pos) cc_pos -= data[--pos];

    cc_offset = 0;
}


// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//              CPU INPUT / OUTPUT
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&


bool CswBuffer::inputCc(uint32 cc) const noexcept
{
    assert(!recording);
    assert(cc>=cc_pos+cc_offset);

    if(cc>=cc_end) seekEnd(); else while(cc>=cc_pos+data[pos]) skip();
    cc_offset = cc-cc_pos;
    return phase;
}

bool CswBuffer::inputTime(Time s) const noexcept
{
	CC cc = s*ccps;

    assert(!recording);
    assert(cc>=cc_pos+cc_offset);

    if(cc>=cc_end) seekEnd(); else while(cc>=cc_pos+data[pos]) skip();
    cc_offset = cc-cc_pos;
    return phase;
}

void CswBuffer::outputCc(uint32 cc, bool bit) throws/*bad alloc*/
{
    assert(recording);
    assert(cc >= cc_pos+cc_offset);

    if(bit!=phase) writePulseCc(cc-cc_pos);
    cc_offset = cc-cc_pos;
}

void CswBuffer::outputTime(Time s, bool bit) throws/*bad alloc*/
{
	CC cc = s*ccps;

    assert(recording);
    assert(cc >= cc_pos+cc_offset);

    if(bit!=phase) writePulseCc(cc-cc_pos);
    cc_offset = cc-cc_pos;
}

void CswBuffer::stopRecording (CC cc) throws/*bad alloc*/
{
    assert(recording);
    assert(cc >= cc_pos+cc_offset);

    writePulseCc(cc-cc_pos);	// finalize current pulse
    writePulseCc(0);			// but don't toggle phase

    recording = no;
	shrinkToFit();
}

void CswBuffer::startRecording (CC cc) noexcept
{
    assert(!recording);
    assert(cc <= cc_end);
	assert(cc >= cc_pos+cc_offset);

    seekCc(cc);
    cc_end = cc_pos;
    end = pos;
    recording = yes;
}





// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//              Read and Write Pulses
//              Used to convert to or from a TapeData subclass.
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

/*  read current pulse
    and step to next pulse.
    Returns 0 at buffer end
*/
uint32 CswBuffer::readPulse() const noexcept
{
    if(pos>=end) return 0;

    uint32 cc = data[pos];
    while( ++pos<end && data[pos]==0 && pos+1<end) { cc += data[++pos]; }
    assert(pos<=end);
    assert(pos<=max);

    phase = !phase;
    cc_pos += cc;
    return cc;
}

void CswBuffer::putbackPulse() const noexcept
{
	// call only to put back what you have read:
	assert(pos>0);

	// step back 1 pos:
    uint32 cc = data[--pos];

    // step back N*2 more pos if pulses separated by 0-length pulses:
    // note: there may be a lonely 0-pulse at data[0]
    while( pos>1 && data[pos-1]==0) { cc += data[----pos]; }

    phase = !phase;
    cc_pos -= cc;
}

/*  append pulse
*/
void CswBuffer::writePulseCc(uint32 cc) throws/*bad alloc*/
{
    while(cc>>16) { writePulseCc(0xFFFFu); writePulseCc(0); cc -= 0xFFFFu; }

    if(pos==max) growBuffer(max + (max>>1) + 10000);
    assert(pos<=end);
    assert(pos<max);

    data[pos++] = cc;

    end = pos;
    cc_end = cc_pos += cc;
    phase = !phase;
}

/*  elongate last pulse
*/
void CswBuffer::appendToPulseCc(uint32 cc) throws/*bad alloc*/
{
    if(pos==0)
        phase = !phase;
    else
    {
        rskip();
        cc += data[pos];
    }
    writePulseCc(cc);
}

/*  write pulse with given polarity
    if bit toggles then write a new pulse
    else elongate the last pulse.
    Can be used as first pulse to set starting polarity of this buffer
*/
void CswBuffer::writePulseCc(uint32 cc, bool bit) throws/*bad alloc*/
{
    if(bit==phase) writePulseCc(cc);  // new pulse
    else appendToPulseCc(cc);         // append
}

/*  set polarity of current phase
    if polarity is wrong then a 0-pulse is added to toggle polarity
*/
void CswBuffer::setPhase(bool bit) throws/*bad alloc*/
{
    if(phase!=bit) writePulseCc(0);
}


//	Store a pure tone:
//	E.g. store pilot tone of a block
//	try to reproduce the overall frequency as exact as possible.
//	some games (Tomahawk) seem to be very sensible to the overall timing.
//
void CswBuffer::writePureTone( uint32 pulses, Time seconds_per_pulse )
{
	CC cc_per_pulse = (CC)(seconds_per_pulse * ccps + 0.5);
	while(pulses--) writePulseCc(cc_per_pulse);
}


//	Store pure data:
//	Store data of tape block into buffer
//
void CswBuffer::writePureData( cu8ptr bu, uint32 total_bits, Time spp_bit0, Time spp_bit1 )
{
	CC cc_bit0 = (CC)(spp_bit0*ccps+0.5);
	CC cc_bit1 = (CC)(spp_bit1*ccps+0.5);

	while( total_bits )
	{
		int8 byte = *bu++;
		int  b = min(8u,total_bits);
		total_bits -= b;

		while( b-- )
		{
			uint32 cc = byte<0 ? cc_bit1 : cc_bit0;
			byte <<= 1;
			writePulseCc(cc);
			writePulseCc(cc);
		}
	}
}


//	store tzx-style pause:
//	if pause = 0 do nothing
//	else
//	tzx:  store ≥1ms opposite level, padding pause with 'low' level to mimic silence level read on ula port.
//	tzx:  don't toggle after pause, next phase level remains 'low'.
//
//	kio:  the silence level depends on model!
//	kio:  => store xlong pulse with opposite level. Play() will reduce level to 0.0. Silence level managed by machine::input().
//	kio:     force next phase level to 'low'. (as required by tzx 'pause' spec.)
//
void CswBuffer::writeTzxPause ( Time seconds )
{
	if(seconds>0)	// pause = 0  =>  do nothing
	{
		writePulseCc(seconds*ccps);
		setPhase(0);
	}
}


/*	skip what looks like silence with evtl. some noise
	"silence" is defined as pulses of ≥ 2ms
	which may be interleaved with some short pulses:
		key clicks are typically 1 short pulse,
		more is suspicious, but a byte stored in a data signal is at least 16 pulses.
	note: ZX80 data has pulses of 4689cc @ 3.25MHz ~ 1.44ms
	NOTE: real audio data should be offset' slightly before compression to csw
		  to make silent passages fall on one side of the threshold.
	during "silence" the polarity of silence may change once (from the last bit written to 0-level threshold)
	toggling more often is more likely "noise"
*/
void CswBuffer::skipSilence(uint N) const noexcept
{
a:	uint32 a    = pos;				// a = potential end of silence / start of data signal
	uint32 cc_a = cc_pos;			// cc at a

	for(uint n=0; n<N; n++)			// at most N short pulses
	{
		uint32 cc = readPulse();
		if(cc==0) return;			// end of buffer
		if(cc>=ccps/500) goto a;	// ≥ 2ms
	}

	// step back to start of non-silence:
	pos = a;
	cc_pos = cc_a;
	if(N&1) phase = !phase;
}


/*	Test whether this block is probably silence or noise only:
	block is detected as "silence" if it contains almost only long pulses of ≥ 2 ms
	block may contain any number of short bursts of up to 5 (n_max) consecutive short pulses.
*/
bool CswBuffer::isSilenceOrNoise() const noexcept
{
	const CC  cc_2ms = ccps/500;	// max. length for a non-silence pulse: 2ms ~ 250Hz
	const int n_max  = 5;			// max. allowed consecutive non-silence pulses

	uint32 pos = 0;
a:	for(int n=0; n<n_max; n++)		// at most n_max short pulses
	{
		if(pos==end) return yes;
	    uint32 cc = data[pos];
	    while( ++pos<end && data[pos]==0 && pos+1<end) { cc += data[++pos]; }
	    assert(pos<=end);
	    assert(pos<=max);
		if(cc >= cc_2ms) goto a;	// long pulse <=> silence
	}

	// N consecutive short pulses detected:
	return no;
}


/*	Split buffer at current position
	=> this buffer is at end
	   current phase of this buffer toggles if cc_offset inside pulse was > 0
*/
void CswBuffer::splitAtCurrentPos( CswBuffer& other )
{
	assert(!recording);
	assert(!other.recording);

	other.purge();
	other.ccps = ccps;
	other.phase = phase;

	if(pos >= end) return;		// end of buffer

	// there is a remainder to split:

	uint32  n = end-pos+1;		// length of rem.
	uint16* q = data + pos;		// first pulse to copy
	uint offs = cc_offset;		// split position inside first pulse to copy

	// avoid 0-length pulses at start of other buffer:

	assert(offs <= *q);
	while(n && offs == *q)
	{
		q++;
		n--;
		offs = 0;
		other.phase ^= 1;
	}
	if(n==0) return;			// end of buffer

	// copy data

	uint16* z = new uint16[n];
	other.data = z;
	other.max = n;
	other.end = n;
	memcpy(z, q, n*sizeof(*q));
	*z -= offs;
	other.cc_end = cc_end - (cc_pos+cc_offset);

	// truncate this buffer:

	if(cc_offset)				// truncate split pulse
	{
		data[pos++] = cc_offset;
		cc_offset = 0;
		phase ^= 1;
	}

	end = pos;					// update end
	cc_end -= other.cc_end;		// update cc_end

	shrinkToFit();
}


/*	normalize a CSW buffer
	removes 0-length pulses at start and end
	merges and normalizes length-pattern of long pulses
	returns this for convenience
*/
CswBuffer* CswBuffer::normalize() noexcept
{
	xlogIn("CswBuffer::normalize()");

	if(is_normalized()) return this;

	uint16* q = data;
	uint16* z = data;
	uint16* e = q+end;

	// skip initial zeros:
	while(q<e && *q==0)
	{
		q++; phase ^= 1;
	}

	// crop trailing zeros:
	while(q<e && *(e-1)==0)
	{
		e--;
	}

	// normalize long and joined pulses:
	while(q<e)
	{
		uint16 n = *q++;
		if(n) { *z++ = n; continue; }		// short pulse

		// n=0 => joined pulse:

		n = *q++;							// pulse to join
		uint p = 0xffff - *--z;				// remaining space in prev. pulse

		if(n<=p) { *z++ += n; continue; }	// append everything

		*z++ = 0xffff;						// normalize joined pulse
		*z++ = 0;
		*z++ = n-p;
	}

	// update this.end:
	xlogline(end==z-data ? "no change" : "%u bytes removed", uint(end-(z-data)));
	end = z-data;

	// fix pos, cc_pos and cc_offset:
	uint32 ccpos = cc_pos;
	seekStart();
	seekCc(ccpos);

	return this;
}


bool CswBuffer::is_normalized() const noexcept
{
	if(end==0) return yes;

	uint16* q = data;
	uint16* e = q+end;

	if(*q==0) return no;		// buffer starts with 0
	if(*--e==0) return no;		// buffer ends with 0

	while(e>q)					// verify that all joined pulses are normalized
	{
		if(*--e) continue;
		if(*--e!=0xffff) return no;
	}

	return yes;
}


/*	add audio to buffer
	ss[count] = buffer
	sps		  = samples/second
	zpos	  = start offset in ss[] (fractional)
	qpos+qoffs= start position in this CswBuffer: pulse index and cc offset inside pulse
	return:
	  zpos, qpos and qoffs updated:
		zpos == count && qpos<count: there is more audio in this CswBuffer
		zpos < count && qpos==count: the end of this CswBuffer was reached
*/
void CswBuffer::addToAudioBuffer(
		StereoSample* ss, uint count, double sps,
		double& zpos, uint32& qpos, double& qoffs, Sample volume )
{
	assert(zpos>=0 && zpos<=count);
	assert(qpos>=0 && qpos<=end);
	assert(qoffs>=0 && qoffs<=(qpos<end?data[qpos]:0));

	if(qpos>=end) return;

	const double sspcc = sps / ccps;					// sspcc = stereosample / cc
	if((qpos^pos^phase^1) & 1) volume = -volume;		// volume & phase

	double ss_a = zpos;									// current pulse start index in ss[] (fractional)
	double ss_e = zpos + (data[qpos] - qoffs) * sspcc;	// current pulse end index in ss[] (fractional)
	qoffs = 0;

	for(;;)
	{
		if(ss_e > count)
		{
			qoffs = data[qpos] - (ss_e - count) / sspcc;
			ss_e = count;
		}

		uint a = uint(ss_a);
		uint e = uint(ss_e);

		if(a==e)
		{
			ss[e] += (ss_e-ss_a) * volume;
		}
		else
		{
			ss[a] += volume * (a+1-ss_a);
			while(++a<e) ss[a] += volume;
			if(e<count) ss[e] += volume * (ss_e-e);
		}

		if(qoffs) break;		// --> qpos=current_pulse < end, qoffs=cc_offset_inside_current_pulse, ss_e=count
		if(++qpos==end) break;	// --> qpos=end, qoffs=0, ss_e = fractional_index_in_ss[] < count

		volume = -volume;
		ss_a = ss_e;
		ss_e += data[qpos] * sspcc;
	}

	zpos = ss_e;
}

















