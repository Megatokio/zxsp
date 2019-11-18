#pragma once
/*	Copyright  (c)	Günter Woigk 2012 - 2019
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

#include "kio/kio.h"
#include "zxsp_types.h"
#include "CswBuffer.h"
#include "TapeData.h"


/*  Manage one block of data in a tape file (class TapeFile)

	TapeData* tapedata contains data formatted for a specific tape file format.
	the tapedata block may be null. if it is present, it must match cswdata.

	CswBuffer* cswdata contains raw audio data; csw = compressed square wave.
	the cswdata buffer must always be present.
	if a TapeFileDataBlocks is grown, ccps and cswdata must be set immediately.

	playing and recording is cpu cycle based.
	cc at start of block = 0.
*/


class TapeFileDataBlock
{
public:
	CswBuffer*  cswdata;		// decoded data for use by input() / output()
	TapeData*   tapedata;		// original data from TapeFile (may be NULL)
	TapData*	tapdata;		// converted to TapData (may be NULL or same as tapedata)
	O80Data*	o80data;		// converted to O80Data (may be NULL or same as tapedata)
	TzxData*	tzxdata;		// converted to TzxData (may be NULL or same as tapedata)

	cstr	major_block_info;
	cstr	minor_block_info;	// NULL for data (non-header) block

	enum State { stopped=0, playing, recording };
	State	mode;

private:
	TapeFileDataBlock(TapeFileDataBlock const&) = delete;
	TapeFileDataBlock& operator=(TapeFileDataBlock const&) = delete;

public:
	void	calc_block_infos();
	void	purge();

public:
	explicit TapeFileDataBlock(CswBuffer*);
	TapeFileDataBlock(TapeData*, uint32 ccps);
	TapeFileDataBlock(TapeData*, CswBuffer*);
	~TapeFileDataBlock();

	void	purgeBlock		();

	bool	isStopped       ()			{ return mode==stopped; }
	bool	isRecording     ()			{ return mode==recording; }
	bool	isPlaying       ()			{ return mode==playing; }

// block info:
	uint32	getTotalCc		() const noexcept { return cswdata->getTotalCc(); }
	Time    getTotalTime    () const noexcept { return cswdata->getTotalTime(); }
	bool    isEmpty         () const noexcept { return cswdata->getTotalCc()==0; }
	bool    isNotEmpty      () const noexcept { return cswdata->getTotalCc()!=0; }
	bool	isSilenceOrNoise() const noexcept { return cswdata->isSilenceOrNoise(); }

	void	calcBlockInfos	();
	cstr    getMajorBlockInfo()			{ return isRecording() ? "Recording…" : major_block_info; }
	cstr    getMinorBlockInfo()			{ return isRecording() ? durationstr(getCurrentTimePos()):minor_block_info; }
	void	setMajorBlockInfo(cstr s)	{ delete[] major_block_info; major_block_info = newcopy(s); }
	void	setMinorBlockInfo(cstr s)	{ delete[] minor_block_info; minor_block_info = newcopy(s); }

// convert:
	TapData* getTapData() noexcept;
	O80Data* getO80Data() noexcept;

// tape position:
// cswdata must be valid
	bool	isAtStart()					{ assert(cswdata); return cswdata->isAtStart(); }
	bool	isAtEnd()					{ assert(cswdata); return cswdata->isAtEnd(); }
	bool	isNearStart(Time proximity)	{ assert(cswdata); return cswdata->getCurrentTime() <= proximity;}
	bool	isNearEnd(Time proximity)	{ assert(cswdata); return getTimeRemaining() <= proximity; }
	uint32	getCurrentCcPos()			{ assert(cswdata); return cswdata->getCurrentCc(); }
	Time	getCurrentTimePos()			{ assert(cswdata); assert(cswdata->ccps); return cswdata->getCurrentTime(); }
	Time	getTimeRemaining()			{ assert(cswdata); return cswdata->getTotalTime() - cswdata->getCurrentTime();}
	void    seekTimePos(Time t)			{ assert(cswdata); assert(mode==stopped); cswdata->seekTime(t); }
	void    seekCcPos(uint32 cc)		{ assert(cswdata); assert(mode==stopped); cswdata->seekCc(cc); }
	void    seekStart()					{ assert(cswdata); assert(mode==stopped); cswdata->seekStart(); }
	void    seekEnd()					{ assert(cswdata); assert(mode==stopped); cswdata->seekEnd(); }

// play & record:
// cswdata must be valid
	void	startPlaying    (uint32 cc);
	void	startRecording  (uint32 cc);
	void	stop            (uint32 cc);
	bool	input           (uint32 cc)			{ assert(cswdata); assert(mode==playing); return cswdata->inputCc(cc);}
	void	output          (uint32 cc, bool b)	{ assert(cswdata); assert(mode==recording); cswdata->outputCc(cc,b); }
	void	videoFrameEnd   (uint32 cc);
};

















