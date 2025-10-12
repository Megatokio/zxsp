// Copyright (c) 2016 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "RzxBlock.h"
#include "RzxFile.h"
#include "unix/files.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <zlib.h>


void throw_zlib_error(int err) noexcept(false) // DataError
{
	switch (err)
	{
	case Z_STREAM_END: throw DataError("zlib: stream end");
	case Z_NEED_DICT: throw DataError("zlib: need dict");
	case Z_OK: return;
	case Z_BUF_ERROR: throw DataError(zlibbuffererror); // decompressed data too long
	case Z_MEM_ERROR: throw DataError(zliboutofmemory); // => can be catched as a DataError
	case Z_DATA_ERROR: throw DataError(zlibdataerror);	// data corrupted
	case Z_VERSION_ERROR: throw DataError(zlibversionerror);
	default: throw DataError("zlib: unknown error %i", err);
	}
}


/*	Input Recording Block
	Typical state change:

	CREATOR				readIRB()								compress()		purge()			rewind()
	  ↓					  ↓										  ↓				  ↓				  ↓
	<EndOfBlock>		<Playing>			<Playing>			<Compressed>	<EndOfBlock>	<Playing>
	  ↓					  ↓					  ↓					  ↓
	startFrame()		N * readByte()		startRecording()	uncompress()
	  ↓					  ↓					  ↓					  ↓
	<Recording>			nextFrame()			<Recording>			<Playing>
	  ↓					  ↓					  ↓
	N * writeByte()		<Playing>			endFrame()
	  ↓					  					  ↓
	endFrame()								<EndOfBlock>
	  ↓
	<EndOfBlock>


	Compressed:

		cbu  =	valid
		ucbu =	nullptr,	ucsize = valid or 0
		fpos =	0
		apos =	0
		ipos =	0
		epos =	0
		current_frame = 0

	EndOfBlock:

		cbu  =	valid or nullptr
		ucbu =	valid or nullptr
		fpos =  ucsize
		apos -> start of data of prev. frame or 0 if fpos=0
		ipos	invalid
		epos ->	end of data of prev. frame or 0 if fpos=0
		current_frame = num_frames

	Playing:

		cbu  =	valid or nullptr
		ucbu =  valid
		fpos -> header of current frame
		apos -> start of data of prev. frame or 0 if fpos=0
		ipos -> in input data for current frame
		epos -> end of input data for current frame
		current_frame < num_frames

	Recording:

		cbu  =  nullptr
		ucbu =  valid
				ucmax = large enough for worst case (~10000 bytes at start of frame)
		fpos =  ucsize
			 -> header of current frame
				note: the current frame is behind ucsize!
		apos -> start of data of prev. frame or 0 if fpos=0
		ipos -> input data of current frame (behind ucsize)
		epos ->	end of data of prev. frame or 0 if fpos=0
		current_frame = num_frames
*/


/*	Initialize Block
	state = EndOfBlock
*/
void RzxBlock::init(IsaID id)
{
	memset(this, 0, sizeof(*this));
	isa = id;

	//	switch(isa)
	//	{
	//	case IsaInvalid:
	//		memset(this,0,sizeof(*this));
	//		break;
	//	case IsaSnapshotBlock:
	//		snapshot_filename = nullptr;
	//		break;
	//	case IsaMachineSnapshot:
	//		snapshot = nullptr;
	//		break;
	//	case IsaInputRecordingBlock:
	//		state = EndOfBlock;
	//		cc_at_start = 0;
	//		num_frames=0;
	//		current_frame=0;
	//		new(&frames) Array<RzxFrame>();
	//		new(&cbu) Array<uint8>();
	//		new(&ucbu) Array<uint8>();
	//		break;
	//	default:
	//		IERR();
	//	}
}

void RzxBlock::kill()
{
	switch (isa)
	{
	case IsaInvalid: break;
	case IsaSnapshotBlock: delete[] snapshot_filename; break;
	case IsaMachineSnapshot:
		logline("RzxBlock: delete machine snapshot: TODO"); //	TODO
		break;
	case IsaInputRecordingBlock:
		delete[] cbu;
		delete[] ucbu;
		break;
	default: IERR();
	}
}


//	helper:
//	grow or shrink ucbu[]
//
//	note: only data up to ucsize is copied to the new buffer.
//		  if state==recording then the current frame is not enclosed!
//
void RzxBlock::resize_ucbu(uint32 newmax)
{
	assert(isaInputRecordingBlock());
	assert(state != Recording); // wg. memcpy(,,ucsize)
	assert(ucsize <= ucmax);
	assert(ucsize <= newmax);

	uint8* newbu = new uint8[newmax];
	memcpy(newbu, ucbu, ucsize);
	delete[] ucbu;
	ucbu  = newbu;
	ucmax = newmax;
}

// helper:
// validate frame data in ucbu[]
// calculate num_frames
// error: logs message and truncates ucsize & num_frames.
//		  caller may detect this and may invalidate cbu[]
//
// after validate() the following potential problems are fixed:
//	• repeated frame at block start		fixed: input count = 0.
//	• truncated frame at block end		removed
//	• frame with icount = 0				block truncated
//	• frame with icount > 20000			block truncated. note: frame may still be excessively long
//
void RzxBlock::scan_ucbu()
{
	// if first frame is marked as a repeated frame, then set it's input count to 0:
	if (ucsize >= 4 && inputCount(0) == 0xffff)
	{
		xlogline("first frame was marked as a repeated frame. (fixed)");
		setInputCount(0, 0);
		delete[] cbu;
		cbu	  = nullptr;
		csize = 0;
	}

	uint32 busize = ucsize; // original ucbu[] size
	ucsize		  = 0;		// ucbu[] size
	current_frame = 0;		// just in case
	num_frames	  = 0;		// number of frames in ucbu[]
	uint32 i	  = 0;		// index in ucbu[]
#ifdef XLOG
	uint32 nframes = 0, rframes = 0;
	uint32 mini = 99999, maxi = 0, gesi = 0, numi = 0;
	uint32 minic = 99999, maxic = 0, gesic = 0;
#endif

	while (i + 4 <= busize)
	{
		uint ic = iCount(i);
		uint in = inputCount(i);

		if (in == 0xffff)
		{
#ifdef XLOG
			rframes++;
#endif
			i += 4;
		}
		else
		{
#ifdef XLOG
			nframes++;
			mini = min(mini, in);
			maxi = max(maxi, in);
			gesi += in;
			numi++;
#endif
			i += 4 + in;
		}

		if (i > busize) break; // truncated frame

		//		if(ic==0)    { xlogline("frame with instr. count = 0. (truncated)"); goto x; }
		if (ic == 0) xlogline("frame with instr. count = 0. (preserved)");
		if (ic > 20000)
		{
			xlogline("instr. count too high: %u. (truncated)", ic);
			goto x;
		} // 3.5e6/50/4 = 17500

		// 3.5e6/50/(28/2)*0.8 = 4000		denk: contended ram: *0.8 ?
		// 3.5e6/60/(28/2)*0.8 = 3333		60 Hz
		// 3.25e6/60/(28/2)    = 3869		b&w machines
		if (ic > 9 && ic < 4000 && num_frames && i < ucsize)
			xlogline("instr. count too low: %u. (ignored)", ic); // just log

		// num. inputs can't be higher than num. instructions:
		// note: may be useful if provided for repeated frames
		if (in != 0xffff && ic < in) xlogline("num. inputs > instr. count: %u vs. %u. (ignored)", in, ic); // just log

			//		if(in==0 && i>0)	// SPIN: benutzt keine repeated frames => da gibts dann auch welche mit 0
			//		{
			//			xlogline("frame %u: num inputs = 0, num instr = %u", num_frames, ic);
			//		}

#ifdef XLOG
		if (num_frames) minic = min(minic, ic);
		maxic = max(maxic, ic);
		gesic += ic;
#endif

		// count frame:
		num_frames += 1;
		ucsize = i;
	}

	if (ucsize < busize)
	{
		xlogline("truncated frame at end: %u bytes)", busize - ucsize);
	x:
		delete[] cbu;
		cbu	  = nullptr;
		csize = 0;
	}

#ifdef XLOG
	logline("normal frames:   %u", nframes);
	logline("repeated frames: %u", rframes);
	logline("num inputs (min,Ø,max) = %u, %u, %u", mini, numi ? gesi / numi : 0, maxi);
	logline("num instR (min,Ø,max) = %u,%u,%u", minic, num_frames ? gesic / num_frames : 0, maxic);
#endif
}

//	helper:
//	compress ucbu[ucsize] -> cbu[csize]
//	ucbu[] remains valid
//
//	If csize != 0, then it is assumed that the cbu[] is valid and nothing is done.
//
//	if called while recording, then the current frame is lost.
//	(the frame can't be finalized because we don't have the icount.)
//
void RzxBlock::_compress()
{
	assert(isaInputRecordingBlock());
	assert(ucsize <= ucmax || (ucmax == 0 && ucbu == nullptr));
	assert(csize == 0 || cbu != nullptr);

	if (csize == 0 && ucsize != 0)
	{
		assert(ucsize <= ucmax && ucbu != nullptr);

		delete[] cbu;
		cbu			 = nullptr;
		uLongf zsize = compressBound(ucsize);
		xlogline("compressBound(%u) = %u", uint(ucsize), uint(zsize));
		std::unique_ptr<uint8[]> zbu(new uint8[zsize]);
		int						 err = ::compress(&zbu[0], &zsize, &ucbu[0], ucsize);
		assert(err == Z_OK);
		cbu	  = new uint8[zsize];
		csize = zsize;
		memcpy(&cbu[0], &zbu[0], zsize);
		xlogline("%u -> %u bytes (ratio = %.6g)", ucsize, csize, double(ucsize) / csize);
	}
}

//	helper:
//	uncompress cbu[csize] -> ucbu[ucsize]
//	cbu[] remains valid
//
//	returns nullptr or error message
//
//	if ucsize!=0 then it is assumed that ucbu[ucsize] was formerly valid
//		and ucsize is the uncompressed size of cbu[]
//
void RzxBlock::_uncompress() noexcept(false) // DataError
{
	assert(isaInputRecordingBlock());
	assert(ucmax == 0 || ucbu != nullptr);

	if (csize == 0) // empty
	{
		assert(ucsize == 0);
	}

	else if (ucsize) // was uncompressed previously => uncompress in one go:
	{
		delete[] ucbu;
		ucbu		 = nullptr;
		ucmax		 = 0;
		ucbu		 = new uint8[ucsize];
		ucmax		 = ucsize;
		uLongf rsize = ucsize;
		int	   err	 = ::uncompress(&ucbu[0], &rsize, &cbu[0], csize);
		assert(err == Z_OK);
		assert(rsize == ucsize);
	}

	else // we don't know the uncompressed size:
	{
		z_stream zs;
		memset(&zs, 0, sizeof(zs));
		zs.next_in	= cbu;
		zs.avail_in = csize;
		int err		= inflateInit(&zs);
		if (err) throw_zlib_error(err);

		double ratio;
		uint32 newsize = 1 kB + csize * 10; // expected ratio ≤ 10

		while (zs.avail_in)
		{
			if (newsize > 666 MB) // worst case assumption for 4h gameplay with 30' tape loading in one block…
			{
				if (ucsize == 666 MB) throw DataError("zlib bomb");
				else newsize = 666 MB;
			}

			resize_ucbu(newsize);
			zs.next_out	 = &ucbu[ucsize];
			zs.avail_out = ucmax - ucsize;
			err			 = inflate(&zs, Z_NO_FLUSH);
			if (err == Z_STREAM_END) assert(zs.avail_in == 0);
			else if (err) throw_zlib_error(err);
			assert(zs.next_out + zs.avail_out == ucbu + ucmax);
			ucsize	= ucmax - zs.avail_out;
			ratio	= (double)ucsize / (csize - zs.avail_in);
			newsize = ucsize + zs.avail_in * (ratio * 1.25);
		}

		xlogline("%u -> %u bytes (ratio = %.8g)", csize, ucsize, ratio);

		inflateEnd(&zs); // free buffers

		// shrink ucbu[]:
		//	the uncompressed size is only unknown after loading from file.
		//	then the block will be compressed (again) until it is played or modified.
		// so even a huge overallocation is no problem.
		// caller may compress+uncompress block to reclaim overallocated memory.
		//
		// if(ucmax/4 > ucsize/3 && ucmax>ucsize+50000) _resize_ucbu(ucsize);
	}
}


// ===========================================================================
//								   _____      _____
//						 /\       |  __ \    |_   _|
//						/  \      | |__) |     | |
//					   / /\ \     |  ___/      | |
//					  / ____ \    | |         _| |_
//					 /_/    \_\   |_|        |_____|
//
// ===========================================================================


/*	PLAY: read byte from input recording block
	Playing -> Playing

	return ≥ 0	byte
		   -1	out of sync or end of file
*/
int RzxBlock::getByte()
{
	assert(isaInputRecordingBlock());
	assert(state == Playing);

	if (ipos < epos) return ucbu[ipos++];
	return -1;
}


/*	RECORD: store next byte
	Recording -> Recording
*/
void RzxBlock::storeByte(uint8 byte)
{
	assert(isaInputRecordingBlock());
	assert(state == Recording);
	assert(ipos < ucmax); // enough room in ucbu[] was asserted in startFrame()

	ucbu[ipos++] = byte;
}


/*	PLAY: go to next frame
	Playing -> Playing

	return > 0	icount
		   -1	end of block:
				state still Playing, frame NOT incremented
				ready to switch to recording!
*/
int RzxBlock::nextFrame()
{
	xlogIn("RzxBlock::nextFrame");

	assert(isaInputRecordingBlock());
	assert(state == Playing);

	if (ipos != epos && // normal frame or repeated frame
		ipos != apos)	// frame with incount == 0
		//		ipos!=4 )		// frame 0 with incount == 0
		logline("RzxBlock::nextFrame: ipos!=epos. This may indicate OutOfSync.");

	if (++current_frame == num_frames) // end of block? => don't move!
	{
		current_frame--;

		IFDEBUG(uint cnt = inputCount(fpos); uint32 nextfpos = cnt == 0xFFFF ? fpos + 4 : epos;
				assert(nextfpos == ucsize);)
		return -1;
	}

	// increment fpos, update apos:
	uint cnt = inputCount(fpos);
	if (cnt == 0xFFFF) // repeated frame
	{
		// apos = apos;		// start of previous frame data does not change
		fpos += 4; // fpos -> current frame header
	}
	else // normal frame
	{
		apos = fpos + 4; // apos -> start of previous frame data
		fpos = epos;	 // fpos -> current frame header
	}

	// set epos, ipos:
	assert(fpos + 4 <= ucsize);
	cnt = inputCount(fpos);
	if (cnt == 0xFFFF) // repeated frame
	{
		ipos = apos; // ipos = prev. frame data start
					 // epos = epos;		// epos = prev. frame data end
	}
	else // normal frame
	{
		ipos = fpos + 4;   // ipos = this frame data start
		epos = ipos + cnt; // epos = this frame data end

		assert(epos <= ucsize);
	}

	return iCount(fpos);
}


/*	RECORD: start a new frame
	EndOfBlock -> Recording
*/
void RzxBlock::startFrame(uint32 cc)
{
	xlogIn("RzxBlock::startFrame");

	assert(isaInputRecordingBlock());
	assert(state == EndOfBlock);
	assert(fpos == ucsize);
	assert(ucsize == 0 || ucbu != nullptr);
	assert(current_frame == num_frames);

	// compressed buffer becomes invalid:
	delete[] cbu;
	cbu	  = nullptr;
	csize = 0;

	// fpos = valid
	// apos = valid
	// epos = valid

	// set recording position behind frame header:
	ipos = fpos + 4;

	// assert enough space in buffer for worst case:
	// (70000cc/frame) / (8cc/io_instr) + 10%  =  9625 io_instr/frame
	if (ipos + 9999 > ucmax) resize_ucbu(ucmax + 32000);

	if (fpos == 0) cc_at_start = cc;
	state = Recording;
}


/*	RECORD: finish current frame
	Recording -> EndOfBlock
	if icount==0 then no frame is stored!
*/
void RzxBlock::endFrame(uint icount)
{
	xlogIn("RzxBlock::endFrame");

	assert(isaInputRecordingBlock());
	assert(state == Recording);
	assert(fpos == ucsize);

	state = EndOfBlock;

	if (icount == 0)			  // no instructions executed
	{							  // might happen if recording is stopped immediately after a ffb
		assert(ipos == fpos + 4); // 0 instr can do at most 0 inputs…
		//		return;						// => don't store a frame BUMMER: may be needed for INT after loading a
		// snapshot!
	}

	// store the instruction (r register) count:
	setICount(fpos, icount);

	// get input count:
	uint cnt = ipos - (fpos + 4);

	//	if( fpos!=0 && cnt <= epos-apos && memcmp(&ucbu[apos],&ucbu[fpos+4],cnt)==0 )
	//	if( fpos!=0 && (cnt==0 || (cnt == epos-apos && memcmp(&ucbu[apos],&ucbu[fpos+4],cnt)==0)) ) // picky other
	// emulators… 	if( fpos!=0 && cnt == epos-apos && memcmp(&ucbu[apos],&ucbu[fpos+4],cnt)==0 ) // non-compliant picky
	// other emulators…
	//	{
	//		// store a repeated frame:
	//		setInputCount(fpos,0xFFFF);
	//		// apos=apos;
	//		// epos=epos;
	//		fpos += 4;
	//	}
	//	else
	{
		// store a normal frame:
		setInputCount(fpos, cnt);
		apos = fpos + 4;
		epos = apos + cnt;
		fpos = epos;
	}

	current_frame = num_frames += 1;
	ucsize		  = fpos;
	// ipos invalid
}


/*	PLAY: truncate frame (and block) and start recording at current position
	Playing -> Recording
*/
void RzxBlock::startRecording()
{
	xlogIn("RzxBlock::startRecording");

	assert(isaInputRecordingBlock());
	assert(state == Playing);
	assert(ucsize == 0 || ucbu != nullptr);

	// compressed buffer becomes invalid:
	delete[] cbu;
	cbu	  = nullptr;
	csize = 0;

	// fpos valid
	// apos valid
	// ipos -> in input data for current frame		(which may be a repeated data block)
	// epos -> end of input data for current frame	(which may be a repeated data block)

	if (apos == 0) // first frame
	{
		// ipos = ipos;
		epos = 0;
		assert(current_frame == 0 && num_frames == 0);
	}
	else if (epos <= fpos) // repeated frame
	{
		// the first bytes of the repeated buffer must be copied to the current frame's own buffer:
		// these are from apos to ipos (excl.)

		if (fpos + 4 + ipos - apos > ucmax) resize_ucbu(fpos + 4 + 20000);

		memcpy(&ucbu[fpos + 4], &ucbu[apos], ipos - apos);

		ipos = fpos + 4 + ipos - apos;
		epos = apos + peek2Z(&ucbu[apos - 2]);
	}
	else // normal frame
	{
		// ipos = ipos;
		epos = apos + inputCount(apos - 4);
	}

	// truncate file:
	ucsize	   = fpos;
	num_frames = current_frame;

	// assert enough space in buffer for worst case:
	// (70000cc/frame) / (8cc/io_instr) + 10%  =  9625 instr/frame
	if (fpos + 4 + 9999 > ucmax) resize_ucbu(ucmax + 20000);

	state = Recording;
}


/*	ANY TIME: purge block
	state -> EndOfBlock
*/
void RzxBlock::purge()
{
	xlogIn("RzxBlock::purge");

	kill();
	init(isa);
}


/*	UNCOMPRESSED: rewind block
	state -> Playing | EndOfBlock

	return > 0	icount
		   -1	state = EndOfBlock

	note: if state==Recording, then the current frame is lost.
		  the frame can't be finalized because we don't have the icount.
*/
int RzxBlock::rewind()
{
	xlogIn("RzxBlock::rewind");

	assert(isaInputRecordingBlock());
	assert(state != Compressed); // uncompressing might throw

	current_frame = 0;

	if (ucsize)
	{
		assert(inputCount(0 /*fpos*/) != 0xffff);

		fpos = apos = 0;
		ipos		= 4;
		epos		= ipos + inputCount(0 /*fpos*/);
		state		= Playing;
		return iCount(0 /*fpos*/);
	}
	else
	{
		assert(num_frames == 0);

		fpos = apos = epos = 0;
		// ipos invalid
		state = EndOfBlock;
		return -1;
	}
}


/*	COMPRESSED: uncompress cbu[] -> ucbu[]
	cbu[] remains valid
	state -> Playing | EndOfBlock

	does not validate the uncompressed data
	rewinds the block.

	return > 0	icount
		   -1	state = EndOfBlock
*/
int RzxBlock::uncompress() noexcept(false) // DataError
{
	xlogIn("RzxBlock::uncompress");

	assert(isaInputRecordingBlock());
	assert(state == Compressed);

	_uncompress();
	state = EndOfBlock; // wg. assert() in rewind(). eigentlich Invalid. Oder wir brauchen ein _rewind()
	return rewind();
}


/*	ANY TIME: compress ucbu[] -> cbu[]
	purges ucbu[]
	state -> Compressed

	does not validate the uncompressed data
	If cbu is not nullptr, then it is assumed to be valid and not compressed again.

	note: if state==Recording, then the current frame is lost.
		  the frame can't be finalized because we don't have the icount.
*/
void RzxBlock::compress()
{
	xlogIn("RzxBlock::compress");

	assert(isaInputRecordingBlock());
	if (state == Compressed) return;

	assert(csize == 0 || cbu != nullptr);
	assert(ucsize == 0 || ucbu != nullptr);
	assert(ucsize <= ucmax);

	_compress();
	state = Compressed;
	delete[] ucbu;
	ucbu  = nullptr;
	ucmax = 0;
	// ucsize = ucsize;				// preserved for faster decompression!
	fpos = apos = ipos = epos = 0;
	current_frame			  = 0;
}


/*	ANY TIME: read Input Recording Block from rzx file
	state -> Playing | EndOfBlock

	in:  blen = block length as in rzx file
	out: at start of block

	throw file_error: real problem, not just EndOfFile. the block is empty.
	throw DataError: EndOfFile, uncompression error, error in block or frame data.
					  the block is usable but truncated at the error position.
*/
void RzxBlock::readInputRecordingBlock(FD& fd, uint32 blen)
{
	xlogIn("read Input Recording Block");

	//	0x00 	0x80 	BYTE 	Recording block ID
	//	0x01 	18+? 	DWORD 	Block length (with compressed data)
	//	0x05 	NF		DWORD 	Number of frames in the block.
	//	0x09 	0x00 	BYTE 	Reserved.
	//	0x0A 	-		DWORD 	T-STATES counter at the beginning
	//	0x0E 	-		DWORD 	Flags
	//							b0: Protected (frames are encrypted with x-key).
	//							b1: Compressed data.
	//	0x12 	-		FRAME 	Sequence of input frames (compressed)
	//
	//	i/o recording frame:
	//	0x00 	IC		WORD		Instruction count (number of R increments, excl. INT)
	//	0x02 	N		WORD		Number of INPUT port reads in this frame (their return values follow).
	//								If N==65535, this is a repeated frame, and the prev. frame must be reused.
	//	0x04 	-		BYTE[N] 	Return values for the CPU I/O port reads.

	kill();
	init(IsaInputRecordingBlock);

	if (blen < 18) throw DataError("block too short");
	blen -= 18; // payload data length

	uint32 frames = fd.read_uint32_z(); // number of frames
	(void)fd.read_char();				// reserved
	cc_at_start	 = fd.read_uint32_z();	// cpu t-cycle counter at start of block
	uint32 flags = fd.read_uint32_z();	// b1 = compressed

	xlogline("%sdata length = %u", flags & 2 ? "compressed " : "", blen);
	xlogline("num. frames = %u", frames);
	xlogline("cpu cycle counter at start = %u", cc_at_start);
	xlogline("flags = 0x%08x%s%s", flags, flags & 1 ? " signed" : "", flags & 2 ? " compressed" : "");

	// cc testen:
	// cc kann nur im Bereich 1 .. 48 liegen.
	// Beim ersten Block nach einem Snapshot ist jedoch die gesamte Frame-Länge ~70000 möglich.
	// Deshalb sollte cc_at_start vom Aufrufer geprüft werden.
	if (cc_at_start > 80000) cc_at_start = 0; // Grobtest

	// num frames testen:
	// TODO: ggf. Block splitten
	// if(frames > 12*60*60*60) throw DataError("illegal num_frames");	// 12 Stunden @ 60 Hz

	if (flags & 2) // compressed data
	{
		cbu	  = new uint8[blen];
		csize = fd.read_bytes(&cbu[0], blen, 123);
		try
		{
			_uncompress();
		}
		catch (DataError& e)
		{
			scan_ucbu();
			rewind();
			if (csize == blen)
				throw; // throw zlib error only if not after EndOfFile error
					   // "file truncated" will be thrown below
		}
	}
	else // uncompressed data
	{
		ucbu   = new uint8[blen];
		ucmax  = blen;
		ucsize = fd.read_bytes(&ucbu[0], blen, 123);
	}

	uint32 zsize = ucsize;

	scan_ucbu(); // calculates num_frames, may truncate ucsize
	rewind();

	if (flags & 2 ? csize != blen : zsize != blen) // EndOfFile in fd.read_bytes()
		throw DataError("file truncated");

	if (num_frames != frames)
		throw DataError(num_frames < frames ? "less frames than expected" : "more frames than expected");

	if (ucsize != zsize) xlogline("some dirt after frames. (ignored)");
}


/*	read snapshot from rzx file block 0x30
	in:	blen = block length from rzx file
	if readSnapshot throws, then the snapshot_filename is nullptr.
*/
void RzxBlock::readSnapshotBlock(FD& fd, uint32 blen, cstr filename) // throws file_error,DataError
{
	//	0x00 	0x30 	BYTE		Snapshot block ID
	//	0x01 	17+SL 	DWORD		Block length
	//	0x05 	-		DWORD		Flags
	//								b0: External Data (snapshot descriptor only, not a snapshot image)
	//								b1: Compressed data.
	//	0x09 	-		ASCIIZ[4] 	Snapshot filename extension ("SNA", "Z80", etc).
	//	0x0D 	USL 	DWORD		Uncompressed snapshot length (same as SL is the snapshot is not compressed)
	//	0x11 	-		BYTE[SL] 	Snapshot data or descriptor.
	//
	//  snapshot descriptor:
	//	0x00 	-		DWORD		Checksum (0 = ignore)
	//	0x04 	-		ASCIIZ[N] 	Snapshot filename

	xlogIn("read Snapshot Block");

	kill();
	init(IsaSnapshotBlock);

	if (blen < 17) throw DataError("block too short");
	blen -= 17; // payload data length

	uint32 flags = fd.read_uint32_z(); // flags: b0=snapshot descriptor, b1=compressed
	char   ext[] = "1234";
	fd.read_bytes(ext, 4);			   // filename extension
	xlogline("extension = %s", ext);   // TODO: test extension
	uint32 uclen = fd.read_uint32_z(); // uncompressed snapshot length
	str	   ssfn;					   // snapshot filename

	if (flags & 1) // external data
	{
		if (blen > MAXPATHLEN) throw DataError("snapshot filepath too long");

		fd.skip_bytes(4);		   // skip crc: probably always 0
		blen -= 4;				   // snapshot filename length
		ssfn = tempstr(blen);	   // snapshot filename
		fd.read_bytes(ssfn, blen); // TODO: test extension
		xlogline("external snapshot file: %s", ssfn);
		FD zd(ssfn, 'r'); // test for readable or throw
		zd.read_char();	  // test for readable or throw
	}
	else if (flags & 2) // compressed snapshot
	{
		if (blen > 1 MB) throw DataError("compressed snapshot too long");
		if (uclen > 1 MB) throw DataError("uncompressed snapshot too long");

		xlogline("compressed data: %u --> %u bytes", blen, uclen);

		cstr tmpdir = "/tmp/zxsp/"; // catstr(tempdirpath(), "/zxsp/");
		create_dir(tmpdir);
		ssfn = catstr(
			tmpdir, basename_from_path(filename), tostr(-fd.file_position()),
			// tostr(-int32(uclen)),
			tostr(-int32(fd.file_mtime())), ".", ext);

		if (is_file(ssfn) && file_size(ssfn) == uclen)
		{
			xlogline("skipping snapshot because it's already in the temp");
			fd.skip_bytes(blen);
		}
		else
		{
			std::unique_ptr<uint8[]> qbu(new uint8[blen]);
			std::unique_ptr<uint8[]> zbu(new uint8[uclen]);
			fd.read_bytes(&qbu[0], blen);
			uLongf zlen = uclen;
			int	   err	= ::uncompress(&zbu[0], &zlen, &qbu[0], blen);
			if (err) throw_zlib_error(err);
			if (zlen != uclen) throw DataError("zlib: decompressed data has wrong size");
			xlogline("tempfile = %s", ssfn);
			FD zd(ssfn, 'w');
			zd.write_bytes(&zbu[0], zlen);
		}
	}
	else // uncompressed snapshot
	{
		if (uclen != blen) throw DataError("uncompressed snapshot length mismatch");
		if (uclen > 1 MB) throw DataError("uncompressed snapshot too long");

		xlogline("uncompressed data: %u bytes", blen);

		cstr tmpdir = "/tmp/zxsp/"; // catstr(tempdirpath(), "/zxsp/");
		create_dir(tmpdir);
		ssfn = catstr(tmpdir, basename_from_path(filename), tostr(-fd.file_position()), tostr(-int32(uclen)), ".", ext);

		if (is_file(ssfn) && file_size(ssfn) == uclen)
		{
			xlogline("skipping snapshot because it's already in the temp");
			fd.skip_bytes(blen);
		}
		else
		{
			std::unique_ptr<uint8[]> zbu(new uint8[blen]);
			fd.read_bytes(&zbu[0], blen);
			xlogline("tempfile = %s", ssfn);
			FD zd(ssfn, 'w');
			zd.write_bytes(&zbu[0], blen);
		}
	}

	snapshot_filename = newcopy(ssfn);
}


/*	write RzxBlock to rzx file
	compressed Snapshot Block or
	compressed Input Recording Block
		if called while recording, then the current frame is not included.
*/
void RzxBlock::write(FD& fd)
{
	switch (isa)
	{
	case IsaInvalid:
	case IsaMachineSnapshot: break;

	case IsaSnapshotBlock:
	{
		xlogline("write Snapshot Block");

		FD						 qf(snapshot_filename);
		uint32					 qlen = qf.file_size();
		std::unique_ptr<uint8[]> qbu(new uint8[qlen]);
		qf.read_bytes(&qbu[0], qlen);
		uint32					 zlen = max(qlen / 8 * 9, uint32(compressBound(qlen)));
		std::unique_ptr<uint8[]> zbu(new uint8[zlen]);
		uLongf					 zsize = zlen;
		int						 err   = ::compress(&zbu[0], &zsize, &qbu[0], qlen);
		if (err) throw_zlib_error(err);
		char qf_ext[4];
		strncpy(qf_ext, extension_from_path(snapshot_filename) + 1, 4); // pads with 0!

		fd.write_uint8(0x30);			// block ID
		fd.write_uint32_z(17 + zsize);	// block length
		fd.write_uint32_z(2);			// flags: b1=compressed data
		fd.write_bytes(qf_ext, 4);		// char[4]: filename extension
		fd.write_uint32_z(qlen);		// uncompressed length
		fd.write_bytes(&zbu[0], zsize); // compressed snapshot
		break;
	}
	case IsaInputRecordingBlock:
		xlogline("write Input Recording Block");
		_compress();
		fd.write_uint8(0x80);			// block ID
		fd.write_uint32_z(18 + csize);	// Block length (with compressed data)
		fd.write_uint32_z(num_frames);	// Number of frames in the block
		fd.write_uint8(0);				// reserved
		fd.write_uint32_z(cc_at_start); // T-STATES counter at the beginning
		fd.write_uint32_z(2);			// Flags: 2 = compressed
		fd.write_bytes(&cbu[0], csize); // Frame data (compressed)
		if (state == Recording)
		{
			delete[] cbu;
			cbu	  = nullptr;
			csize = 0;
		}
		break;

	default: IERR();
	}
}


/*	RECORDING: append the very short current frame to the previous frame
	Nach einem verzögerten Interrupt soll der aktuelle, sehr kurze Frame-Schnippel
	an den vorherigen Frame angehängt werden.

	state --> EndOfBlock

	if the previous frame is a repeater and if this frame already has some inputs,
	or if the current frame is the first frame, ((deprecated))
	then the frames are not united and the current frame is just finished with endFrame().
*/
void RzxBlock::amendFrame(uint icount)
{
	assert(isRecording());
	assert(icount && icount <= 12); // 48cc = 12*4
	assert(ucsize == fpos);

	if (fpos == 0)
	{
		xlogline("RzxBlock.amendFrame: could not amend the previous frame because this frame is the first frame");
		endFrame(icount);
		return;
	}

	assert(current_frame >= 1);
	assert(apos > 0);

#if 1 // never append frame snippets with inputs:

	if (ipos > (fpos + 4))
	{
		xlogline("RzxBlock.amendFrame: could not amend prev. frame because this frame already has inputs");
		endFrame(icount);
		return;
	}

	// find fpos of previous frame:
	bool   prev_isa_rep = apos + inputCount(apos - 4) != fpos;
	uint32 prev_fpos	= prev_isa_rep ? fpos - 4 : apos - 4;

	// add icount:
	setICount(prev_fpos, iCount(prev_fpos) + icount);

#else // append snippets with inputs if prev. frame isn't a repeated frame:

	uint in_count = ipos - (fpos + 4);
	assert(in_count <= 6); // 48cc = 6*(4+4)

	// apos -> input data of prev. frame
	// these may be the frame's own data or data of a repeated frame.

	bool prev_isa_rep = apos + inputCount(apos - 4) != fpos;

	if (prev_isa_rep && in_count != 0)
	{
		// wir müssen Input-Bytes anhängen, kennen aber die Anzahl der Inputs im vorigen Frame nicht.
		// Dort ist ja nur das Flag 0xFFFF gespeichert.
		// Die Anzahl der Inputs im vorigen Frame ist ≤ Anzahl der Inputs im wiederholten Frame.
		// Die kann gleich sein, muss aber nicht…
		// Im seltenen Fall, dass Anzahl der Inputs im wiederholten Frame == 0 ist, wüssten wir's natürlich…

		// Da das Rzx-File auch abspielbar ist, wenn wir den aktuellen Frame-Schnippel NICHT an den vorigen
		// Frame anhängen, und nur num_frames etwas an Aussagekraft verliert, hängen wir den Frame-Schnippel
		// hier eben NICHT an.

		xlogline("RzxBlock.amendFrame: could not amend prev. frame because it's a repeater and this frame has inputs");
		endFrame(icount);
		return;
	}

	// find fpos of previous frame:
	uint32 prev_fpos = prev_isa_rep ? fpos - 4 : apos - 4;

	// add icount:
	setICount(prev_fpos, iCount(prev_fpos) + icount);

	if (in_count)
	{
		// the current frame already has some inputs.
		// the prev. frame is no repeated frame.

		uint8 ibu[6];
		memcpy(ibu, ucbu + fpos + 4, in_count);

		memcpy(ucbu + fpos, ibu, in_count);
		epos = ucsize = fpos += in_count;
		setInputCount(prev_fpos, inputCount(prev_fpos) + in_count);
	}

#endif

	state = EndOfBlock;
}
