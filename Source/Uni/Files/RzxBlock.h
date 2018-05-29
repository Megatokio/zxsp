/*	Copyright  (c)	Günter Woigk 2016 - 2018
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

#ifndef RZXBLOCK_H
#define RZXBLOCK_H

#include "kio/kio.h"
#include "Templates/Array.h"


extern void throw_zlib_error(int err) noexcept(false); // data_error


// Input Recording Block (IRB), ID=0x80
// or Snapshot Block, ID=0x30
// or Machine Snapshot (TODO)
struct RzxBlock
{
	enum IsaID
	{
		IsaInvalid=0,			// Creator
		IsaSnapshotBlock,		// ID=0x30
		IsaInputRecordingBlock,	// ID=0x80
		IsaMachineSnapshot		// internal, for winding, TODO
	};
	IsaID	isa;

	enum State					// for Input Recording Block
	{
		EndOfBlock=0,
		Playing,
		Recording,
		Compressed
	};

	union
	{
		struct						// Input Recording Block:
		{
			State	state;

			uint8*	cbu;			// compressed input recording data as in rzx file at IRB+0x12
			uint32	csize;			// compressed data size

			uint8*	ucbu;			// uncompressed input recording data as in rzx file at IRB+0x12
			uint32	ucsize;			// uncompressed data size
			uint32	ucmax;			// allocated size

			uint32	cc_at_start;	// cpu cycle counter at start of block, 0=unknown
			uint32	num_frames;

			// current frame play/record position:
			uint32	current_frame;	// frame number
			uint32	fpos;			// frame header position
			uint32	apos;			// start of data of previous frame
			uint32	ipos;			// input read/write position
			uint32	epos;			// playing: input data end
		};
		cstr	snapshot_filename;	// Snapshot Block: typically only the first block is a snapshot block.
		void*	zxsp_snapshot;		// Machine Snapshot: for fore/back winding. TODO
	};


	RzxBlock(IsaID id=IsaInvalid)	{ init(id); }
	~RzxBlock()						{ kill(); }

	bool	isaInputRecordingBlock() const		{ return isa==IsaInputRecordingBlock; }
	bool	isaIRB() const						{ return isa==IsaInputRecordingBlock; }
	bool	isaSnapshotBlock() const			{ return isa==IsaSnapshotBlock; }
	bool	isaMachineSnapshot() const			{ return isa==IsaMachineSnapshot; }
	bool	isaInvalid() const					{ return isa==IsaInvalid; }

	bool	isPlaying() const					{ assert(isaIRB()); return state==Playing; }
	bool	isRecording() const					{ assert(isaIRB()); return state==Recording; }
	bool	isCompressed() const				{ assert(isaIRB()); return state==Compressed; }
	bool	isEndOfBlock() const				{ assert(isaIRB()); return state==EndOfBlock; }
	bool	isLastFrame() const					{ assert(isPlaying()); return current_frame==num_frames-1; }

	// if Playing:
	uint16	iCount() const						{ return peek2Z(&ucbu[fpos]); }
	uint16	iCount(uint32 fpos) const			{ return peek2Z(&ucbu[fpos]); }
	uint16	inputCount() const					{ return peek2Z(&ucbu[fpos+2]); }
	uint16	inputCount(uint32 fpos) const		{ return peek2Z(&ucbu[fpos+2]); }
	void	setICount(uint32 fpos, uint n)		{ poke2Z(&ucbu[fpos], n); }
	void	setInputCount(uint32 fpos, uint n)	{ poke2Z(&ucbu[fpos+2], n); }
	uint32	startCC() const						{ return fpos ? 0 : cc_at_start; }

	void	purge();							// any time
	void	startRecording();					// playing
	int		rewind();							// any time
	int		getByte();							// playing
	int		nextFrame();						// playing
	void	storeByte(uint8);					// recording
	void	startFrame(uint32 cc);				// recording
	void	endFrame(uint);						// recording
	void	amendFrame(uint);					// recording
	void	compress();
	int		uncompress() noexcept(false); // data_error

	// read from / write to rzx file:
	void	readInputRecordingBlock(FD&, uint32 blklen)			throws; // file_error,data_error
	void	readSnapshotBlock(FD&, uint32 blklen, cstr filename)throws; // file_error,data_error
	void	write(FD&)											throws;

private:
	friend class RzxFile;
	void	init(IsaID);
	void	kill();
	void	resize_ucbu(uint32 newmax);
	void	scan_ucbu();
	void	 _compress();
	void	_uncompress() noexcept(false); // data_error
};





#endif






















