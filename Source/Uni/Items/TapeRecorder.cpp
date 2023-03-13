// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "TapeRecorder.h"
#include "Audio/TapeFile.h"
#include "Audio/TapeFileDataBlock.h"
#include "DspTime.h"
#include "Machine.h"
#include "RecentFilesMenu.h"
#include "ZxInfo.h"
#include "unix/files.h"
#include <Templates/Array.h>


#define WINDING_SPEED 40.0


/*	Sounds für TS2020:
 */
cstr ts2020_fname[] = {"ts2020/open lid empty",	 "ts2020/open lid loaded",
					   "ts2020/close lid empty", "ts2020/close lid loaded",
					   "ts2020/pause dn",		 "ts2020/pause up",
					   "ts2020/play dn",		 "ts2020/play up",
					   "ts2020/ff dn",			 "ts2020/ff up",
					   "ts2020/rewind dn",		 "ts2020/rewind up",
					   "ts2020/record dn",		 "ts2020/record up",
					   "ts2020/motor empty",	 "ts2020/motor play loaded",
					   "ts2020/motor ff loaded", "ts2020/motor rewind loaded"};

/*	Sounds für Plus2 tape recorder:
	some sounds are borrowed from TS2020
*/
cstr plus2_fname[] = {
	"2a/open empty",
	"2a/open loaded",
	"2a/close empty",
	"2a/close loaded",
	"2a/pause on",
	"2a/pause off",
	"2a/play on",
	"2a/play off",
	"2a/ff on",
	"2a/ff off",
	"2a/rewind on",
	"2a/rewind off",
	"2a/record on",
	"2a/record off",
	"ts2020/motor empty",
	"ts2020/motor play loaded",
	"ts2020/motor ff loaded",
	"ts2020/motor rewind loaded"};

/*	Sounds für external taperecorder:
	most sounds are borrowed from +2A and TS2020
*/
static const cstr walkman_fname[] = {
	"walkman_open empty", // open empty
	"walkman_open empty", // open loaded
	"2a/close empty",	  // close empty
	"2a/close empty",	  // close loaded
	"2a/pause on",
	"2a/pause off",
	"2a/play on",
	"2a/play off",
	"2a/pause on",	// ff
	"2a/pause off", // ff
	"2a/pause on",	// rewind
	"2a/pause off", // rewind
	"2a/pause on",	// record
	"2a/pause off", // record
	"ts2020/motor empty",
	"ts2020/motor play loaded",
	"ts2020/motor ff loaded",
	"ts2020/motor rewind loaded"};


// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// C'tor & D'tor:


TapeRecorder::TapeRecorder(Machine* machine, isa_id id, const cstr audio_names[], bool auto_start, bool fast_load) :
	Item(machine, id, isa_TapeRecorder, Internal(machine->model_info->has_tape_drive), nullptr, nullptr),
	auto_start_stop_tape(auto_start),
	instant_load_tape(fast_load),
	machine_ccps(machine->model_info->cpu_cycles_per_second),
	state(stopped),
	record_is_down(no),
	pause_is_down(no),
	stop_position(0.0),
	tapefile(nullptr)
{
	list_id = machine->isA(isa_MachineZxsp)	   ? gui::RecentZxspTapes :
			  machine->isA(isa_MachineZx81)	   ? gui::RecentZx81Tapes :
			  machine->isA(isa_MachineZx80)	   ? gui::RecentZx80Tapes :
			  machine->isA(isa_MachineJupiter) ? gui::RecentJupiterTapes :
												 gui::RecentFiles;

	if (list_id == gui::RecentFiles)
	{
		showWarning("TapeRecorder: unknown machine");
		list_id = gui::RecentZxspTapes;
	}

	// sounds:
	memset(sound, 0, sizeof(sound));
	memset(sound_count, 0, sizeof(sound_count));

	for (uint i = 0; i < NELEM(sound); i++) // load sound files:
	{
		if (audio_names[i] == nullptr) continue; // dafür gibt es keinen Sound

		for (uint j = 0; j < i; j++) // suche doppelt verwendeten Sound:
		{
			if (eq(audio_names[i], audio_names[j]))
			{
				sound[i]	   = sound[j];
				sound_count[i] = sound_count[j];
				break;
			}
		}
		if (sound_count[i]) continue; // multiple used sound already loaded

		FD	   fd(catstr(appl_rsrc_path, "Audio/", audio_names[i], ".raw"), 'r');
		uint32 cnt = fd.file_size() >> 1;
		int16* zbu = new int16[cnt];
		fd.read_bytes(zbu, cnt << 1);

		Sample* data = sound[i] = new Sample[cnt];
		sound_count[i]			= cnt;
		for (uint j = 0; j < cnt; j++) data[j] = ldexpf((int16)peek2Z(zbu + j), i < sound_motor_empty ? -16 : -15);
		delete[] zbu;
	}
}


Walkman::Walkman(Machine* machine, bool auto_start, bool fast_load) :
	TapeRecorder(machine, isa_Walkman, walkman_fname, auto_start, fast_load)
{}

TS2020::TS2020(Machine* machine, bool auto_start, bool fast_load) :
	TapeRecorder(machine, isa_TS2020, ts2020_fname, auto_start, fast_load)
{}

Plus2TapeRecorder::Plus2TapeRecorder(Machine* machine, bool auto_start, bool fast_load) :
	TapeRecorder(machine, isa_Plus2Tapedeck, plus2_fname, auto_start, fast_load)
{}

Plus2aTapeRecorder::Plus2aTapeRecorder(Machine* machine, bool auto_start, bool fast_load) :
	TapeRecorder(machine, isa_Plus2aTapedeck, plus2_fname, auto_start, fast_load)
{}


TapeRecorder::~TapeRecorder()
{
	delete tapefile; // will also write to file if modified

	for (uint i = 0; i < NELEM(sound); i++)
	{
		Sample* s = sound[i];
		if (!s) continue;
		delete[] s;
		for (uint j = i + 1; j < NELEM(sound); j++)
			if (sound[j] == s) sound[j] = nullptr;
	}
}


// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// Item Interface:


void TapeRecorder::videoFrameEnd(int32 cc)
{
	if (!tapefile) return;

	if (state == playing) // play or record, maybe paused
	{
		if (pause_is_down) return;

		tapefile->videoFrameEnd(cc);

		if (tapefile->isAtEndOfTape() && !record_is_down)
		{
			state = stopped;
			tapefile_stop(cc);
			play_sound(sound_play_up);
		}
	}
	else if (state == winding) // schnellen Vorlauf
	{
		xxlog(">>");
		Time current_position = tapefile->getCurrentPosition() + WINDING_SPEED * cc / machine_ccps;
		if (current_position >= stop_position)
		{
			current_position = stop_position;
			play_sound(sound_ff_up);
			state = stopped;
		}
		tapefile->seekPosition(current_position);
	}
	else if (state == rewinding) // schnellen Rücklauf
	{
		xxlog("<<");
		Time current_position = tapefile->getCurrentPosition() - WINDING_SPEED * cc / machine_ccps;
		if (current_position <= stop_position)
		{
			current_position = stop_position;
			play_sound(sound_rewind_up);
			state = stopped;
		}
		tapefile->seekPosition(current_position);
	}
}


// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// rom patch handlers:

/*  Handle Auto-Start of Tape:
	Caller must have locked list_lock (e.g. when called from cpu patch)
*/
void TapeRecorder::autoStart(CC cc) noexcept
{
	if (!tapefile) return;								// kein Tape eingelegt
	if (state == winding || state == rewinding) return; // Benutzer ist am spulen...
	if (state == playing && !pause_is_down) return;		// läuft schon

	if (state != playing) play_sound(sound_play_down);
	state = playing;
	if (pause_is_down) play_sound(sound_pause_up);
	pause_is_down = no;

	if (record_is_down) tapefile_record(cc);
	else tapefile_play(cc);
}


/*  Handle Auto-Stop of Tape:
	Caller must have locked list_lock (e.g. when called from cpu patch)
*/
void TapeRecorder::autoStop(CC cc) noexcept
{
	if (!tapefile) return;								// kein Tape eingelegt
	if (state == winding || state == rewinding) return; // Benutzer ist am spulen...
	if (state != playing || pause_is_down) return;		// läuft nicht

	tapefile_stop(cc);

	if (tapefile->isNearEndOfTape() || this->isaId() == isa_Walkman)
	{
		state = stopped;
		play_sound(sound_play_up);
	}
	else
	{
		pause_is_down = yes;
		play_sound(sound_pause_down);
	}
}


void TapeRecorder::audioBufferEnd(Time)
{
	// button sounds:
	for (uint i = 0; i < active_sound.count(); i++)
	{
		int		id	 = active_sound[i].id;	  // diesen sound spielen wir
		int32	qi	 = active_sound[i].index; // an dieser position sind wir schon
		Sample* data = sound[id];			  // -> daten dieses sounds

		int32 zi = 0;
		if (qi < 0)
		{
			zi = min(DSP_SAMPLES_PER_BUFFER, -qi);
			qi -= zi;
		}																		 // dest index in audio_out_buffer[]
		int32 ze = min(uint(DSP_SAMPLES_PER_BUFFER), zi + sound_count[id] - qi); // dest end index
		while (zi < ze) { os::audio_out_buffer[zi++] += data[qi++]; }			 // copy audio data

		active_sound[i].index = qi;
		if (uint32(qi) >= sound_count[id]) active_sound.remove(i--); // sound finished
	}

	// motor sound:
	if (state == playing || state == winding || state == rewinding)
	{
		int id = !tapefile		  ? sound_motor_empty :
				 state == playing ? pause_is_down ? sound_motor_empty : sound_motor_play :
				 state == winding ? sound_motor_ff :
									sound_motor_rewind;

		uint32	qi	 = motor_sound_pos % sound_count[id]; // an dieser position sind wir schon
		Sample* data = sound[id];						  // -> daten dieses sounds

		int32 zi = 0; // dest index in audio_out_buffer[]
	r:
		int32 ze = min(uint(DSP_SAMPLES_PER_BUFFER), zi + sound_count[id] - qi); // dest end index
		while (zi < ze) { os::audio_out_buffer[zi++] += data[qi++]; }			 // copy audio data
		if (qi == sound_count[id])
		{
			qi = 0;
			goto r;
		}

		motor_sound_pos = qi;
	}

	// tape audio out:
	if (isPlaying()) play_block();
}


/*	test whether we are willing to read a TapData or O80Data block
	• tapefile must be loaded
	• tape recorder must not be set to recording
	• tape recorder must be stopped, paused or playing; not fast winding
	• does not assert that there actually is a block to load
*/
bool TapeRecorder::can_read_block() const noexcept
{
	return tapefile != nullptr && !record_is_down && (state == stopped || state == playing);
}


/*  get next (current) block as a std ZXSP tape block
	Caller must have locked list_lock (e.g. when called from cpu patch)
*/
TapData* TapeRecorder::getZxspBlock() noexcept
{
	assert(can_read_block());
	assert(state == stopped || state == playing);
	assert(tapefile);

	if (state == stopped) { return tapefile->readTapDataBlock(); }
	else // playing:
	{
		if (!pause_is_down) tapefile_stop(current_cc(), yes);
		TapData* bu = tapefile->readTapDataBlock();

		// restore consistency:
		if (tapefile->isNearEndOfTape()) // either stop TapeRecorder as well
		{
			state = stopped;
			play_sound(sound_play_up);
		}
		else // or restart TapeFile
		{
			if (!pause_is_down) tapefile_play(current_cc());
		}
		return bu;
	}
}


/*  get next (current) block as a std ZX80/ZX81 tape block
	Caller must have locked list_lock (e.g. when called from cpu patch)
*/
O80Data* TapeRecorder::getZx80Block() noexcept
{
	assert(can_read_block());
	assert(state == stopped || state == playing);
	assert(tapefile);

	if (state == stopped) { return tapefile->readO80DataBlock(); }
	else // playing:
	{
		if (!pause_is_down) tapefile_stop(current_cc(), yes);
		O80Data* bu = tapefile->readO80DataBlock();

		// restore consistency:
		if (tapefile->isNearEndOfTape()) // either stop TapeRecorder as well
		{
			state = stopped;
			play_sound(sound_play_up);
		}
		else // or restart TapeFile
		{
			if (!pause_is_down) tapefile_play(current_cc());
		}
		return bu;
	}
}

bool TapeRecorder::can_store_block() const noexcept
{
	return state == playing &&
		   //	_tapefile != nullptr &&	implied by record_is_down
		   record_is_down && (auto_start_stop_tape || !pause_is_down);
}

/*	write ZX Spectrum or ZX80 block to tape
	• if near end of block advance to next block and eventually insert new empty block
	  else current block will be overwritten!
	• TapeRecorder must be recording
	• pause button may be down if auto_start_stop_tape is set
	• return 0: not handled
	  return 1: handled
*/
void TapeRecorder::storeBlock(TapeData* q) noexcept
{
	assert(can_store_block());
	assert(tapefile);

	if (tapefile->isStopped()) // pause down
	{
		tapefile->writeTapeDataBlock(q);
	}
	else // pause not down
	{
		assert(tapefile->isRecording());
		tapefile_stop(current_cc());
		tapefile->writeTapeDataBlock(q);
		tapefile_record(current_cc());
	}
}


void TapeRecorder::play_block()
{
	const uint count = DSP_SAMPLES_PER_BUFFER;

	double	zpos  = 0.0;
	uint&	block = speaker.blk;
	uint32& qpos  = speaker.pos;
	double& qoffs = speaker.offs;

	while (block < tapefile->cnt)
	{
		CswBuffer* bu = (*tapefile)[block]->cswdata;
		bu->addToAudioBuffer(os::audio_out_buffer, count, ::samples_per_second, zpos, qpos, qoffs, speaker.volume);
		if (zpos == count) return;
		block++;
		qpos  = 0;
		qoffs = 0;
	}
}

/*	stop tape file
	if tapefile was playing, then the last audio buffer is played, to ensure all bits are played,
	in case the user is downloading this into a real spectrum.
	This can be supressed with 'mute=1' for instant-load, though this does not help much.
	It's just a little bit less annoying.
*/
void TapeRecorder::tapefile_stop(CC cc, bool mute)
{
	assert(!tapefile->isStopped());

	if (tapefile->isPlaying() && !mute) play_block();
	tapefile->stop(cc);
}

void TapeRecorder::tapefile_play(CC cc)
{
	assert(tapefile->isStopped());

	tapefile->startPlaying(cc);

	speaker.blk	 = tapefile->pos;
	speaker.pos	 = tapefile->current_block->cswdata->pos;
	speaker.offs = tapefile->current_block->cswdata->cc_offset;
}

void TapeRecorder::tapefile_record(CC cc)
{
	assert(tapefile->isStopped());
	tapefile->startRecording(cc);
}


// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// Slots:


/*  SLOT:
	STOP wurde gedrückt:
	alle Tasten raus außer PAUSE
*/
void TapeRecorder::stop()
{
	xlogIn("TapeRecorder.stop");
	assert(is_locked());

	if (state == stopped && !record_is_down) return;

	if (tapefile && tapefile->isRunning()) tapefile_stop(current_cc());

	if (state == winding) play_sound(sound_ff_up);
	if (state == rewinding) play_sound(sound_rewind_up);
	if (state == playing) play_sound(sound_play_up);
	state = stopped;
	if (record_is_down) play_sound(sound_record_up);
	record_is_down = no;
}


/*  Eject tape
	may be called when no tapefile loaded
	returns the current tapefile, if any
	caller must delete it and may do this outside locked machine
	plays the "open lid" sound
*/
TapeFile* TapeRecorder::eject()
{
	xlogIn("TapeRecorder.eject");
	assert(is_locked());

	stop();
	TapeFile* tf = tapefile;
	tapefile	 = nullptr;
	play_sound(tf ? sound_open_deck_loaded : sound_open_deck_empty);
	return tf;
}


/*  Insert tape into the recorder
	newtapefile may be nullptr
	plays the "close lid" sound
*/
void TapeRecorder::insert(TapeFile* newtapefile)
{
	xlogIn("TapeRecorder.insert(TapeFile)");
	assert(is_locked());
	assert(!isLoaded());

	stop();
	tapefile = newtapefile;
	play_sound(newtapefile ? sound_close_deck_loaded : sound_close_deck_empty);
}


/*  Insert tape into the recorder
	for use in machine creator
	no audio effects
*/
void TapeRecorder::insert(cstr filepath)
{
	xlogIn("TapeRecorder.insert(filepath)");
	assert(is_locked());
	assert(filepath != nullptr);

	state = stopped;

	delete tapefile;
	tapefile = nullptr;
	tapefile = new TapeFile(machine_ccps, filepath);
}


void TapeRecorder::setFilename(cstr new_filename) noexcept
{
	xlogIn("TapeRecorder.setFilename");
	assert(isMainThread());
	assert(isLoaded());
	assert(new_filename != nullptr);

	tapefile->setFilepath(new_filename);
}

int TapeRecorder::setWriteProtected(bool f) noexcept
{
	xlogIn("TapeRecorder.setWriteProtected");

	return tapefile ? tapefile->setWriteProtected(f) : -1 /*no tape*/;
}


/*  SLOT:
	set / reset Pause:
	PAUSE haut keine anderen Tasten raus und kann
	jederzeit zusätzlich zu allen anderen Tasten gedrückt sein
*/
TapeRecorder* TapeRecorder::pause(bool f)
{
	xlogIn("TapeRecorder.pause");
	assert(is_locked());

	if (f == pause_is_down) return this;

	pause_is_down = f;
	play_sound(f ? sound_pause_down : sound_pause_up);

	if (!tapefile) return this;

	if (state == playing)
	{
		if (pause_is_down) tapefile_stop(current_cc());			// suspend playing or recording
		else if (record_is_down) tapefile_record(current_cc()); // resume recording
		else tapefile_play(current_cc());						// resume playing
	}
	else if (state == winding)
	{
		if (pause_is_down) stop_position = tapefile->getEndOfBlock(); // => stop at block end
		else stop_position = tapefile->getTotalPlaytime();			  // => stop at tape end
	}
	else if (state == rewinding)
	{
		if (pause_is_down) stop_position = tapefile->getStartOfBlock(); // => stop at block start
		else stop_position = 0.0;										// stop at tape start
	}

	return this;
}


/*  SLOT:
	WIND FORWARD wurde gedrückt:
	alle anderen Tasten raus außer PAUSE:
	PAUSE dient als Flag für stop_at_block_end
*/
void TapeRecorder::wind()
{
	xlogIn("TapeRecorder.wind");
	assert(is_locked());

	if (state == winding) return;

	stop();

	if (tapefile) // loaded
	{
		if (pause_is_down) // pause = flag for 'stop at block end'
		{
			stop_position = tapefile->getEndOfBlock();
			if (tapefile->current_block->isEmpty()) tapefile->seekStartOfNextBlock();
		}
		else stop_position = tapefile->getTotalPlaytime();
	}

	state = winding;
	play_sound(sound_ff_down);
}


/*  SLOT:
	WIND BACKWARD wurde gedrückt:
	alle anderen Tasten raus außer PAUSE:
	PAUSE dient als Flag für stop_at_block_start
*/
void TapeRecorder::rewind()
{
	xlogIn("TapeRecorder.rewind");
	assert(is_locked());

	if (state == rewinding) return;

	stop();

	if (tapefile) // loaded
	{
		if (pause_is_down)
		{
			if (tapefile->isAtStartOfBlock()) tapefile->seekEndOfPrevBlock();
			stop_position = tapefile->getStartOfBlock();
		}
		else stop_position = 0.0;
	}

	state = rewinding;
	play_sound(sound_rewind_down);
}


/*  SLOT:
	PLAY wurde gedrückt:
	alle anderen Tasten raus außer PAUSE und RECORD:
*/
void TapeRecorder::play()
{
	xlogIn("TapeRecorder.play");
	assert(is_locked());

	if (state == playing) return;
	if (state == winding) play_sound(sound_ff_up);
	if (state == rewinding) play_sound(sound_rewind_up);
	play_sound(sound_play_down);

	if (tapefile && !pause_is_down)
	{
		assert(tapefile->isStopped());
		if (record_is_down) tapefile_record(current_cc());
		else tapefile_play(current_cc());
	}

	state = playing;
}


/*  SLOT:
	RECORD wurde getoggelt:
	RECORD lässt sich nur toggeln, wenn keine Taste (Play,Fore,Back) gedrückt ist
		   und wenn nicht write_protected.
	sonst muss man STOP drücken, oder PLAY, WIND oder REWIND
*/
void TapeRecorder::record()
{
	xlogIn("TapeRecorder.record");
	assert(is_locked());

	if (tapefile && state == stopped && !tapefile->write_protected)
	{
		record_is_down = !record_is_down;
		play_sound(record_is_down ? sound_record_down : sound_record_up);
	}
}


void TapeRecorder::deleteCurrentBlock() // delete current block and goto start of next
{
	xlogIn("TapeRecorder.deleteCurrentBlock");
	assert(is_locked());

	if (state != stopped) return;
	if (!tapefile) return;

	tapefile->deleteCurrentBlock();
}

void TapeRecorder::newBlockAfterCurrent() // add block after current and goto start
{
	xlogIn("TapeRecorder.newBlockAfterCurrent");
	assert(is_locked());

	if (state != stopped) return;
	if (!tapefile) return;

	tapefile->insertBlockAfterCurrent();
}

void TapeRecorder::newBlockBeforeCurrent() // add block before current and goto start
{
	xlogIn("TapeRecorder.newBlockBeforeCurrent");
	assert(is_locked());

	if (state != stopped) return;
	if (!tapefile) return;

	tapefile->insertBlockBeforeCurrent();
}
