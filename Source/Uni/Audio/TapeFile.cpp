// Copyright (c) 2000 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

/*
	class TapeFile: representation for program tapes
*/

#include "TapeFile.h"
#include "AudioData.h"
#include "CswBuffer.h"
#include "O80Data.h"
#include "RlesData.h"
#include "TapData.h"
#include "TapeFile.h"
#include "TapeFileDataBlock.h"
#include "TzxData.h"
#include "cpp/cppthreads.h"
#include "globals.h"
#include "unix/files.h"
#include "zxsp_types.h"
#include <QAudioDecoder>
#include <QAudioFormat>


// helper
static isa_id isaIdFromFilename(cstr path)
{
	cstr ext = lowerstr(leftstr((extension_from_path(path)), 4));

	return eq(ext, ".tap")	? isa_TapData :
		   eq(ext, ".tzx")	? isa_TzxData :
		   eq(ext, ".rle")	? isa_RlesData :
		   eq(ext, ".o")	? isa_O80Data :
		   eq(ext, ".80")	? isa_O80Data :
		   eq(ext, ".p")	? isa_O80Data :
		   eq(ext, ".81")	? isa_O80Data :
		   eq(ext, ".aiff") ? isa_AudioData :
		   eq(ext, ".aif")	? isa_AudioData :
		   eq(ext, ".wav")	? isa_AudioData :
		   eq(ext, ".mp3")	? isa_AudioData :
							  isa_unknown;
}


// helper
void TapeFile::update_blk_info()
{
	assert(pos < this->count());

	current_block = data[pos];
	blk_cswbuffer = current_block->cswdata;
	blk_cc_size	  = current_block->getTotalCc();
	blk_starttime = getStartOfBlock(pos);
	//  blk_cc_offset  = xxx; wird in startRecording/Playing gesetzt und in input/output/videoFrameEnd aktualisiert
}


// helper
void TapeFile::append_empty_block()
{
	bool phase0 = cnt > 0 ? last()->cswdata->getFinalPhase() : 0;
	append(new CswBuffer(machine_ccps, phase0, 666));
	//	last()->setMajorBlockInfo("End of tape");
	//	modified = yes;
}


// helper
void TapeFile::insert_empty_block(uint i)
{
	assert(i <= cnt);

	if (i == cnt)
	{
		append_empty_block();
		return;
	}

	bool phase0 = data[i]->cswdata->getPhase0();
	insertrange(i, i + 1);
	assert(data[i] == nullptr);
	data[i] = new TapeFileDataBlock(new CswBuffer(machine_ccps, phase0, 666));
	//	data[i]->setMajorBlockInfo("Empty block");
	//	modified = yes;
}


// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//          c'tor & d'tor
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&


/* CREATOR
 */
TapeFile::TapeFile(uint32 machine_ccps, cstr filename) :
	filepath(newcopy(filename)), machine_ccps(machine_ccps), write_protected(yes), modified(no), mode(stopped), pos(0),
	current_block(nullptr), blk_cswbuffer(nullptr), blk_cc_size(0), blk_starttime(0.0), blk_cc_offset(0)
{
	xlogIn("new TapeFile");

	assert(machine_ccps != 0);
	assert(filename != nullptr);

	try
	{
		readFile(filename);
	}
	catch (AnyError& e)
	{
		showAlert("Could not read tape file:\n%s", e.what());
		modified		= no;
		write_protected = yes;
		append_empty_block();
		goto_block(0);
		current_block->seekStart();
	}
}


/*	DESTRUCTOR
 */
TapeFile::~TapeFile()
{
	xlogIn("~TapeFile");
	if (modified)
	{
		try
		{
			if (mode == recording) current_block->stop(current_block->cswdata->getCurrentCc());
			mode = stopped;
			writeFile(filepath);
		}
		catch (AnyError& e)
		{
			showAlert("An error occured while writing to tape file:\n%s", e.what());
		}
	}
	delete[] filepath;
	while (cnt) delete data[--cnt];
}


// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//								Queries
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&


int TapeFile::setWriteProtected(bool wp) volatile noexcept
{
	//	assert(isMainThread());

	write_protected = wp;
	int err			= set_file_writable(filepath, wp ? NOBODY : OWNER | GROUP);
	if (!err) return ok;
	write_protected = !is_writable(filepath);
	return err;
}


void TapeFile::setFilepath(cstr fpath) volatile noexcept
{
	//	assert(isMainThread());

	fpath = fullpath(fpath);
	delete[] filepath;
	filepath = newcopy(fpath);
	try
	{
		create_file(fpath); // touch
	}
	catch (FileError&)
	{}
	write_protected = !is_writable(filepath);
	modified		= yes;
}


/*	Get total play time of tape
 */
Time TapeFile::getTotalPlaytime() const
{
	Time t = 0.0;
	for (uint i = 0; i < count(); i++) t += data[i]->getTotalTime();
	return t;
}


/*	Get position of start of block
 */
Time TapeFile::getStartOfBlock(uint blk)
{
	assert(blk < count());

	Time t = 0.0;
	while (blk) t += data[--blk]->getTotalTime();
	return t;
}


// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//								Load / Save Tape
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&


void TapeFile::readFile(cstr path) throws // file_error,data_error,bad_alloc
{
	xlogIn("TapeFile:readFile");
	assert(mode == stopped);

	purge();
	delete[] filepath;
	filepath = nullptr;

	isa_id ft = isaIdFromFilename(path);
	switch (ft)
	{
	case isa_RlesData: RlesData::readFile(path, *this); break;
	case isa_AudioData: AudioData::readFile(path, *this); break;
	case isa_TapData: TapData::readFile(path, *this); break;
	case isa_TzxData: TzxData::readFile(path, *this); break;
	case isa_O80Data: O80Data::readFile(path, *this); break;
	default: throw DataError("unknown file type");
	}

	for (uint i = 0; i < this->count(); i++)
	{
		TapeFileDataBlock* d = data[i];
		(void)d;
		assert(d->cswdata != nullptr);
		assert(d->cswdata->ccPerSecond() == machine_ccps);
		assert(d->isStopped());
		xlogline("%s", d->major_block_info);
		xlogline("%s", d->minor_block_info);
		xlogline("  total cc = %u", uint(d->getTotalCc()));
		xlogline("  total time = %u", uint(d->getTotalTime()));
	}

	modified		= no;
	filepath		= newcopy(path);
	write_protected = !is_writable(path);

	append_empty_block();
	goto_block(0);
	current_block->seekStart();
}


void TapeFile::writeFile(cstr path) throws
{
	xlogIn("TapeFile:writeFile");
	assert(mode == stopped);

	isa_id ft = isaIdFromFilename(path);
	switch (ft)
	{
	case isa_RlesData: RlesData::writeFile(path, *this); break;
	case isa_AudioData: AudioData::writeFile(path, *this); break;
	case isa_TapData: TapData::writeFile(path, *this); break;
	case isa_TzxData: TzxData::writeFile(path, *this, TzxConversionDefault); break;
	case isa_O80Data: O80Data::writeFile(path, *this); break;
	default: throw DataError("unknown file type");
	}
}


// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//                          Play / Record / Stop
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&


/*	stop tape:
	if currently recording, then the blockInfos are calculated
*/
void TapeFile::stop(uint32 cc)
{
	xlogIn("TapeFile.stop()");

	current_block->stop(blk_cc_offset + cc); // recording => also calcBlockInfos

	if (mode == recording)
	{
		blk_cc_size = current_block->getTotalCc();		   // update_blk_info();
		if (current_block == last()) append_empty_block(); // allow ">>" to "End of file"
	}

	mode = stopped;
}


/*	start recording:
	• if near end of block advance to next block and eventually insert new empty block
	• purge this block and
	• start recording into it
*/
void TapeFile::startRecording(uint32 cc)
{
	xlogIn("TapeFile.startRecording()");

	assert(pos < count());
	assert(blk_cswbuffer && blk_cswbuffer == current_block->cswdata);

	if (getPlaytimeOfBlock() > 2.0 && isNearEndOfBlock(min(5.0, getPlaytimeOfBlock() / 4)))
	{
		seekStartOfNextBlock();
		if (getPlaytimeOfBlock() > 2.0) insertBlockBeforeCurrent();
	}

	blk_cc_size	  = 0;		// update_blk_info();
	blk_cc_offset = 0 - cc; // blk_cswbuffer->getCurrentCc() - cc;
	current_block->startRecording(0 /*cc+blk_cc_offset*/);

	mode	 = recording;
	modified = yes;

	// for auto block splitting:
	current_phase	= current_block->cswdata->getCurrentPhase();
	current_cc		= cc;
	num_data_pulses = 0;
}


/*	start playing:
 */
void TapeFile::startPlaying(uint32 cc)
{
	xlogIn("TapeFile.startPlaying()");

	assert(pos < this->count());
	assert(blk_cswbuffer && blk_cswbuffer == current_block->cswdata);

	if (mode == playing) return;
	if (mode == recording) stop(cc);

	mode = playing;

	//    cc *= blk_cc_ratio;

	blk_cc_offset = blk_cswbuffer->getCurrentCc() - cc;

	current_block->startPlaying(cc + blk_cc_offset);
}


/*  input from tape:
	tape must be playing
*/
bool TapeFile::input(uint32 machine_cc)
{
	assert(mode == playing);
	assert(blk_cswbuffer && blk_cswbuffer == current_block->cswdata);

a:
	uint32 blk0_cc = machine_cc + blk_cc_offset;
	if (blk0_cc <= blk_cc_size) return blk_cswbuffer->inputCc(blk0_cc);

	// End of tape?
	// note: tape will be stopped at videoFrameEnd()
	if (pos + 1 == this->count()) return blk_cswbuffer->inputCc(blk_cc_size);

	// step into next block:
	current_block->stop(blk0_cc);
	blk_cc_offset -= blk_cc_size;
	goto_block(pos + 1);
	current_block->startPlaying(0);

	goto a;
}


/*  output to tape
	tape must be recording
*/
void TapeFile::output(uint32 cc, bool bit)
{
	assert(mode == recording);

	// scrutinize pulse for auto block splitting:
	if (current_phase != bit)
	{
		Time d = (Time)(cc - current_cc) / machine_ccps;
		if (d <= 0.001) // ≥ 500 Hz => looks like a valid data pulse => count it
		{
			num_data_pulses++;
		}
		else if (current_block->getCurrentTimePos() >= 2.0 && num_data_pulses >= 400) // pulse too long but "enough"
																					  // data pulses seen => split block
		{
			xlogline("TapeFile::output: split recording block");
			current_block->stop(blk_cc_offset + cc);
			insert_empty_block(pos + 1);
			goto_block(pos + 1);
			blk_cc_offset = -cc;
			current_block->startRecording(0 /*blk_cc_offset+cc*/);
		}
		else // pulse too long => restart counting
		{
			num_data_pulses = 0;
		}

		current_phase = bit;
		current_cc	  = cc;
	}

	blk_cc_size = blk_cc_offset + cc;
	assert(blk_cswbuffer && blk_cswbuffer == current_block->cswdata);
	blk_cswbuffer->outputCc(blk_cc_size, bit);
}


/*  Notification: video frame end
	items should run up to cc
	time base of machine will be decremented by cc after this call
*/
void TapeFile::videoFrameEnd(int32 cc)
{
	assert(mode == playing || mode == recording);

	blk_cc_offset += cc;

	if (mode == playing)
	{
		if (blk_cc_offset > blk_cc_size) // beyond end of block?
		{
			// End of tape?
			// note: tape will be stopped by TapeRecorder
			if (pos + 1 == this->count()) return;

			// step into next block:
			current_block->stop(blk_cc_offset);
			blk_cc_offset -= blk_cc_size;
			goto_block(pos + 1);
			current_block->startPlaying(0);
		}
	}
	else // if mode==recording)
	{
		if (cc > current_cc && (Time)(cc - current_cc) / machine_ccps > 0.01 // longer than 10 ms silence
			&& num_data_pulses >= 400										 // enough valid data pulses pending
			&& current_block->getCurrentTimePos() >= 2.0)					 // block longer than minimum => split block
		{
			xlogline("TapeFile::videoFrameEnd: split recording block");
			current_block->stop(blk_cc_offset);
			insert_empty_block(pos + 1);
			goto_block(pos + 1);
			blk_cc_offset = 0;
			current_block->startRecording(0 /*blk_cc_offset+cc*/);

			num_data_pulses = 0;
			current_cc		= cc;
		}


		//		if( blk_cc_offset/10 > blk_cswbuffer->getCurrentCc() &&	// no output last 90% of this frame (10% ~ timer
		//interrupt handler max. duration…) 		blk_cswbuffer->getTotalPulses()>200)					// not at start of
		//block (TODO: better test)
		//		{
		//			// Block-Ende-Erkennung
		//			// Blöcke schon bei der Aufnahme trennen, damit
		//			// sinnvolle Block-Info angezeigt werden kann und
		//			// damit der Block für realloc-on-auto-grow nicht zu lang wird
		//			//
		//			xlogline("TapeFile::videoFrameEnd: split recording block");
		//			current_block->stop(blk_cc_offset);
		//			insert_empty_block(pos+1);
		//			goto_block(pos+1);
		//			blk_cc_offset = 0;
		//			current_block->startRecording(0/*blk_cc_offset+cc*/);
		//		}
	}

	current_cc -= cc;

	current_block->videoFrameEnd(blk_cc_offset);
}


// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//                              Instant Load & Save
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&


/*  get the next standard data block from tape
	returns either valid TapData block or nullptr
*/
TapData* TapeFile::readTapDataBlock() noexcept
{
	assert(mode == stopped);

	if (isNearEndOfBlock(min(5.0, getPlaytimeOfBlock() / 4))) seekStartOfNextBlock();

	for (;;)
	{
		if (isAtEndOfTape()) return nullptr;
		TapData* bu = current_block->getTapData();
		seekStartOfNextBlock();
		if (bu->trust_level >= TapeData::conversion_success) return bu;
	}
}


/*  get the next standard data block from tape
	returns either valid O80Data block or nullptr
*/
O80Data* TapeFile::readO80DataBlock() noexcept
{
	assert(mode == stopped);

	if (isNearEndOfBlock(min(5.0, getPlaytimeOfBlock() / 4))) seekStartOfNextBlock();

	for (;;)
	{
		if (isAtEndOfTape()) return nullptr;
		O80Data* bu = current_block->getO80Data();
		seekStartOfNextBlock();
		if (bu->trust_level >= TapeData::conversion_success) return bu;
	}
}


/*	write TapeData block to file
	• instant-save TapData, O80Data or other TapeData block
	• if near end of block advance to next block and eventually insert new empty block
	• purge this block and
	• store TapeData block into this block
*/
void TapeFile::writeTapeDataBlock(TapeData* q)
{
	assert(mode == stopped);

	if (isNearEndOfBlock(min(5.0, getPlaytimeOfBlock() / 4)) && !current_block->isSilenceOrNoise())
	{
		seekStartOfNextBlock();
		if (!current_block->isSilenceOrNoise()) insertBlockBeforeCurrent();
	}

	current_block = new TapeFileDataBlock(q, machine_ccps);
	current_block->seekEnd();
	delete data[pos];
	data[pos] = current_block;
	modified  = yes;
	update_blk_info();
	blk_cc_offset = current_block->getCurrentCcPos();
}


// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//                              Winding
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&


void TapeFile::seekStart()
{
	assert(mode == stopped);

	goto_block(0);
	current_block->seekStart();
}

void TapeFile::seekEnd()
{
	assert(mode == stopped);

	goto_block(this->count() - 1);
	current_block->seekEnd();
}


/*	Move tape to selected position
	• move tape position to proper block and inside this block to proper offset
	• if tape position is exactly at the end of a block,
		then if this block is not empty, then move to start of the next block
		else if this block is empty, then stay in this block
*/
void TapeFile::seekPosition(Time t)
{
	assert(mode == stopped);

	Time a = getStartOfBlock(); // aktueller Block
	Time e = getEndOfBlock();	// geht von [a bis [e

	while (t < a && pos > 0)
	{
		e = a;
		a -= data[--pos]->getTotalTime();
	}

	while (
		a != e ?
			t >= e // block not empty: move to next block if beyond or at end of block
			:
			t > e &&
				pos + 1 <
					count()) // block empty: move to next block if beyond end, except if this is the final empty block
	{
		if (pos + 1 == this->count()) append_empty_block(); // 2014-02-16 wg. nicht mehr auto-grow; korr. 2014-4-12
		a = e;
		e += data[++pos]->getTotalTime();
	}

	update_blk_info();
	t = t - a;
	limit(0.0, t, current_block->getTotalTime());
	current_block->seekTimePos(t);
}


/*	Move tape position to the start of the selected tapedata
 */
void TapeFile::seekBlock(int n)
{
	assert(mode == stopped);

	limit(0, n, int(this->count()) - 1);
	goto_block(n);
	current_block->seekStart();
}


/*	Move tape position to the start of the current tapedata
 */
void TapeFile::seekStartOfBlock()
{
	assert(mode == stopped);

	current_block->seekStart();
}


/*	Move tape position to the end of the current tapedata
 */
void TapeFile::seekEndOfBlock()
{
	assert(mode == stopped);

	current_block->seekEnd();
}


/*	Move tape position to the start of the next tapedata.
	except if already in last tapedata, then append empty tapedata
	except if last tapedata is already empty, then do nothing
*/
void TapeFile::seekStartOfNextBlock()
{
	assert(mode == stopped);
	assert(pos < this->count());

	if (pos + 1 < this->count() || current_block->isNotEmpty())
	{
		if (pos + 1 == this->count())
			append_empty_block(); // 2014-02-16 wg. nicht mehr auto-grow  &&  hat wohl eh gebummert
		goto_block(pos + 1);
		current_block->seekStart();
	}
}


/*	Move tape position to the end of the previous tapedata.
	except if already in first block, then move to start of block
*/
void TapeFile::seekEndOfPrevBlock()
{
	assert(mode == stopped);
	assert(pos < this->count());

	if (pos)
	{
		goto_block(pos - 1);
		current_block->seekEnd();
	}
	else
		current_block->seekStart();
}


void TapeFile::purgeCurrentBlock()
{
	assert(mode == stopped);

	if (current_block->isNotEmpty()) modified = yes;

	current_block->purgeBlock();
	update_blk_info();
}

/*	Delete the current tapedata.
	Set the tape position to the start of the next tapedata.
	Except if this is the last tapedata, then just purge it.
*/
void TapeFile::deleteCurrentBlock()
{
	assert(mode == stopped);

	if (current_block->isNotEmpty()) modified = yes;

	if (current_block == last())
		current_block->purgeBlock();
	else
		remove(pos);
	update_blk_info();
	current_block->seekStart();
}


/*	Insert an empty tapedata at the position of the current tapedata.
	Set the tape position to the start of this tapedata. (implicitely, new tapedata is empty)
*/
void TapeFile::insertBlockBeforeCurrent()
{
	assert(mode == stopped);

	this->insert_empty_block(pos);
	update_blk_info();
}

void TapeFile::insertBlockAfterCurrent()
{
	assert(mode == stopped);

	this->insert_empty_block(pos + 1);
	goto_block(pos + 1);
	update_blk_info();
}


// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// Input, Output and AudioBufferEnd are only called when the Tape is attached to a Machine


#if 0
void TapeFile::input( Time when, int32, uint16 addr, uint8& byte, uint8& mask )
{
	TODO(); // check for end of block
	(void)addr;
	if(state==playing)
	{
		byte = tapedata[pos]->input(when) ? byte | EAR_IN_MASK : byte & ~EAR_IN_MASK;
		mask |= EAR_IN_MASK;
	}
}

void TapeFile::output( Time when, int32, uint16 addr, uint8 byte )
{
/*	a zero in bit 3 activates the MIC output,
	a one in bit 4 activates the EAR output and the internal speaker.
	the EAR and MIC sockets are coupled by resistors, so activating one activates the other;
	the EAR output produces a louder sound, but MIC out is used ROM:SAVETAPE.
*/
	(void)addr;

	if(state==recording)
	{
		tapedata[pos]->output( when, ~byte&MIC_OUT_MASK );
	}
}

void TapeFile::audioBufferEnd( Time when )
{
	tapedata[pos]->audioBufferEnd(when);
}
#endif


#if 0
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// Play and Record are called for the global tape recorder:

/*	play tape:
	tape -> audio buffer
*/
void TapeFile::play ( StereoSample* buffer, int count )
{
	if( state!=playing ) return;
	assert(pos<tapedata.count());

	for(;;)
	{
		int n = tapedata[pos]->play(buffer,count);
		if( n>=count ) return;
		if( isLastBlock() ) break;		// don't skip beyond last tapedata
		pos++;
		buffer+=n; count-=n;
	}

	while(count) *buffer++ = 0.0;
}


/*	record tape:
	audio buffer -> tape
*/
void TapeFile::record ( StereoSample const* buffer, int count )
{
	if(state!=recording) return;
	assert(pos<tapedata.count());
	tapedata[pos]->record(buffer,count);
	// TODO: check return value for end-of-block detection
}
#endif


bool TapeFile::canBeSavedAs(cstr filename, cstr* why)
{
	cstr ext = lowerstr(extension_from_path(filename));
	cstr msg = nullptr;
	if (why) *why = nullptr;

	// these always work:
	if (eq(ext, ".tzx")) return yes;
	if (eq(ext, ".wav")) return yes;
	if (eq(ext, ".aif")) return yes;
	if (eq(ext, ".aiff")) return yes;
	//	if(eq(ext,".rles")) return yes;


	bool p81  = eq(ext, ".p81");
	bool zx80 = eq(ext, ".o") || eq(ext, ".80");
	bool zx81 = eq(ext, ".p") || eq(ext, ".81") || p81;


	// test for writing to .tap file:

	if (eq(ext, ".tap") || eq(ext, ".tape"))
	{
		bool valid_block_seen = no;

		for (uint i = 0; i < count(); i++)
		{
			TapeFileDataBlock* tfd = data[i];
			if (tfd->o80data && tfd->o80data->is_zx80)
			{
				msg = "tape contains a block for the ZX80";
				goto x;
			}
			if (tfd->o80data && tfd->o80data->is_zx81)
			{
				msg = "tape contains a block for the ZX81";
				goto x;
			}

			TapData* tapdata = tfd->getTapData();
			if (tapdata == nullptr)
			{
				msg = "internal error: a block could not be converted to TapData";
				goto x;
			}

			switch (tapdata->trust_level)
			{
			case TapeData::TrustLevel::no_data: continue; // pause
			case TapeData::TrustLevel::conversion_failed: msg = "tape contains a block with unknown encoding"; goto x;
			case TapeData::TrustLevel::truncated_data_error:
			case TapeData::TrustLevel::checksum_error:
			case TapeData::TrustLevel::decoded_data:
			case TapeData::TrustLevel::original_data: valid_block_seen = yes; continue;
			default:
			{
				msg = "internal error: unknown trust level";
				goto x;
			}
			}
		}

		if (valid_block_seen) return yes; // one (or more) valid blocks, no major problems
		if (msg) goto x;				  // no valid block, only one (or more) truncated blocks
		if (why) *why = "empty file";
		return yes; // store 0 blocks in .p81 file is ok
	}


	// test for writing to ZX80 / ZX81 file:

	if (zx80 || zx81)
	{
		bool valid_block_seen = no;

		for (uint i = 0; i < count(); i++)
		{
			TapeFileDataBlock* tfd = data[i];
			if (tfd->tapdata && tfd->tapdata->trust_level >= TapeData::TrustLevel::conversion_success)
			{
				msg = "tape contains a block for the ZX Spectrum";
				goto x;
			}

			O80Data* o80 = tfd->getO80Data();
			if (o80 == nullptr)
			{
				msg = "internal error: a block could not be converted to O80Data";
				goto x;
			}

			switch (o80->trust_level)
			{
			case TapeData::TrustLevel::no_data: continue; // pause
			case TapeData::TrustLevel::conversion_failed: msg = "tape contains a block with unknown encoding"; goto x;
			case TapeData::TrustLevel::truncated_data_error:
				msg = "tape contains a truncated block";
				continue; // will be silently skipped
			case TapeData::TrustLevel::checksum_error:
			case TapeData::TrustLevel::decoded_data:
			case TapeData::TrustLevel::original_data:
				if (zx80 && o80->is_zx81)
				{
					msg = "tape contains a block for the ZX81";
					goto x;
				}
				if (zx81 && o80->is_zx80)
				{
					msg = "tape contains a block for the ZX80";
					goto x;
				}
				if (valid_block_seen && !p81)
				{
					msg = "tape contains more than one valid block";
					goto x;
				}
				valid_block_seen = yes;
				continue;
			default:
			{
				msg = "internal error: unknown trust level";
				goto x;
			}
			}
		}

		if (valid_block_seen) return yes; // one (or more) valid blocks, no major problems
		if (msg) goto x;				  // no valid block, only one (or more) truncated blocks
		if (p81)
		{
			if (why) *why = "empty file";
			return yes;
		} // store 0 blocks in .p81 file is ok
		msg = "tape is empty";
		goto x; // else error: no block
	}

	msg = "unknown file type";

x:
	if (why) *why = msg;
	return no;
}
