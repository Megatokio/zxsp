#pragma once
// Copyright (c) 2000 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

/*	class TapeFile
	--------------

	Representant for tape files inserted into the tape recorder.

	The TapeFile class is not reentrant.
	The user must take care for proper blocking,
	because (at least) Record() and Play() are called from
	a different thread (the core audio io thread).

	different block types:
		tap		read from .tap file				created from zxsp rom save routine
		o80		read from .o or .80 file		created from zx80 rom save routine patch
		p81		read from .p or .81 file		created from zx81 rom save routine patch
		rles	read from .rles file			created by recording into the internal tape recorder
												created by recording into the global tape recorder when set to csw mode
		tzx		read from tzx file
		pzx		read from pzx file
		mono	read from mono audio file 		created by recording into the global tape recorder when set to hifi mono
mode stereo	read from stereo audio file		created by recording into the global tape recorder when set to hifi stereo
mode

	csw (tap,o80,p81,tzx,pzx) can be converted to rles
	rles can be converted to mono
	mono can be converted to stereo
	stereo can be converted to mono
	mono can be converted to rles
	rles can be converted to generic csw files (tzx, pzx)
		and may be convertible to dedicated csw files (tap, o80, p81)

	tap blocks can be flash-loaded into zxsp
	o80 blocks can be flash-loaded into zx80
	p81 blocks can be flash-loaded into zx81
	rles may be flash-loadable

	rles, mono, stereo can be played by the global tape recorder
	rles can be played by the internal tape recorder

note:
	Framework	AudioToolbox/AudioToolbox.h
	Declared in	AudioToolbox/AudioFile.h
*/

#include "CswBuffer.h"
#include "TapeData.h"
#include "TapeFileDataBlock.h"
#include "Templates/Array.h"
#include <QObject>


/*  Manage a tape file
	not re-entrant!
	tapes are split in TapeFileDataBlocks
	TapeFileDataBlocks are stored in data[]
	pos points to the current block

	last block must always be empty except while recording into it
	=> data[] always contains at least one block
	=> pos always points to an existing block
	=> cached values are always valid
*/


class TapeFile : protected Array<TapeFileDataBlock*>
{
	friend class TapeRecorder;
	using Array<TapeFileDataBlock*>::cnt; // make cnt and data visible
	using Array<TapeFileDataBlock*>::data;

	cstr   filepath;	 // allocated
	uint32 machine_ccps; // cc per second
	bool   write_protected;
	bool   modified;
	enum State { stopped = 0, playing, recording };
	State mode;

	uint pos; // aktueller Block in data[]
	// cached values:
	TapeFileDataBlock* current_block; // aktueller Block; secondary pointer
	CswBuffer*		   blk_cswbuffer; // aktueller Block; secondary pointer
	uint32			   blk_cc_size;	  // aktueller Block; block size in tapeblock_cc's
	Time			   blk_starttime; // aktueller Block;
	uint32			   blk_cc_offset; // aktueller Block; cc_base in tapeblock_cc's; valid during play/record

	// while recording:
	bool   current_phase;
	int32  current_cc;
	uint32 num_data_pulses;

private:
	void purge()
	{
		while (cnt) delete data[--cnt];
		Array<TapeFileDataBlock*>::purge();
	}
	void remove(uint i)
	{
		assert(i < cnt);
		delete data[i];
		Array<TapeFileDataBlock*>::remove(i);
	}
	void update_blk_info();
	void goto_block(uint i)
	{
		pos = i;
		update_blk_info();
	}
	void append_empty_block();
	void insert_empty_block(uint i);


public:
	TapeFileDataBlock* last() { return Array<TapeFileDataBlock*>::last(); }
	TapeFileDataBlock* operator[](uint i)
	{
		assert(i < cnt);
		return data[i];
	}
	uint count() const { return cnt; }
	void append(TapeFileDataBlock* blk) { Array<TapeFileDataBlock*>::append(blk); }
	void append(TapeData* tapedata) { append(new TapeFileDataBlock(tapedata, machine_ccps)); }
	void append(CswBuffer* csw) { append(new TapeFileDataBlock(csw)); }

public:
	TapeFile(uint32 machine_ccps, cstr filepath);
	TapeFile(const TapeFile&)			 = delete;
	TapeFile& operator=(const TapeFile&) = delete;
	virtual ~TapeFile();

	bool isStopped() const volatile noexcept { return mode == stopped; }
	bool isRunning() const volatile noexcept { return mode != stopped; }
	bool isPlaying() const volatile noexcept { return mode == playing; }
	bool isRecording() const volatile noexcept { return mode == recording; }
	bool isModified() const volatile noexcept { return modified; }

	bool isWriteProtected() const volatile noexcept { return write_protected; }
	int	 setWriteProtected(bool) volatile noexcept;
	void setFilepath(cstr) volatile noexcept;
	cstr getFilepath() const volatile noexcept { return filepath; }

	// Tape I/O:
	void stop(uint32 cc);
	void startRecording(uint32 cc);
	void startPlaying(uint32 cc);
	bool input(uint32 cc);			  // MUST be playing
	void output(uint32 cc, bool bit); // MUST be recording
	void videoFrameEnd(int32 cc);	  // MUST be playing or recording

	// record/playback with system samples_per_second
	//	void		play				( StereoSample* buffer, int count );
	//	void		record				( StereoSample const* buffer, int count );

	// load / save file:
	void writeFile(cstr filepath); // MUST be stopped
	void readFile(cstr filepath);  // MUST be stopped
	bool canBeSavedAs(cstr filepath, cstr* why = nullptr);

	Time getTotalPlaytime() const;
	Time getCurrentPosition() const { return blk_starttime + current_block->getCurrentTimePos(); }
	void seekPosition(Time);
	void seekStart();
	void seekEnd();
	bool isAtEndOfTape() const { return isLastBlock() && isAtEndOfBlock(); }
	bool isAtStartOfTape() const { return isFirstBlock() && isAtStartOfBlock(); }
	bool isNearEndOfTape(Time proximity = 1.0) const { return isLastBlock() && isNearEndOfBlock(proximity); }
	bool isNearStartOfTape(Time proximity = 1.0) const { return isFirstBlock() && isNearStartOfBlock(proximity); }

	// block handling:
	int	 getTotalBlocks() { return count(); }
	uint getCurrentBlockIndex() { return pos; }
	Time getStartOfBlock() { return blk_starttime; }
	Time getEndOfBlock() { return blk_starttime + current_block->getTotalTime(); }
	Time getStartOfBlock(uint blk);
	Time getEndOfBlock(uint blk) { return getStartOfBlock(blk) + data[blk]->getTotalTime(); }
	bool isLastBlock() const { return pos == count() - 1; }
	bool isFirstBlock() const { return pos == 0; }

	Time getPlaytimeOfBlock() const { return current_block->getTotalTime(); }
	bool isAtStartOfBlock() const { return current_block->isAtStart(); }
	bool isAtEndOfBlock() const { return current_block->isAtEnd(); }
	bool isNearStartOfBlock(Time proximity = 1.0) const { return current_block->isNearStart(proximity); }
	bool isNearEndOfBlock(Time proximity = 1.0) const { return current_block->isNearEnd(proximity); }

	void seekBlock(int n);		 // MUST be stopped; goto start of block n
	void seekStartOfBlock();	 // MUST be stopped
	void seekEndOfBlock();		 // MUST be stopped
	void seekStartOfNextBlock(); // MUST be stopped; goto start of next block
	void seekEndOfPrevBlock();	 // MUST be stopped; goto end of prev block

	void purgeCurrentBlock();		 // MUST be stopped
	void deleteCurrentBlock();		 // MUST be stopped
	void insertBlockBeforeCurrent(); // MUST be stopped
	void insertBlockAfterCurrent();	 // MUST be stopped

	cstr getMajorBlockInfo() const
	{
		return pos == count() - 1 ? "End of tape" : current_block->getMajorBlockInfo();
	}																			  // e.g. "Program: <name>"
	cstr getMinorBlockInfo() const { return current_block->getMinorBlockInfo(); } // e.g. "246/3000 bytes"
	void setMajorBlockInfo(cstr info) { current_block->setMajorBlockInfo(info); }

	TapData* readTapDataBlock() noexcept;
	O80Data* readO80DataBlock() noexcept;
	void	 writeTapeDataBlock(TapeData*);

	uint32 ccps() { return machine_ccps; }
};


typedef TapeFile* TapeFilePtr;
