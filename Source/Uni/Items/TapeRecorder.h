#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

/*	Item TapeRecorder
	-----------------

	Repräsentiert einen Kassettenrecorder
	entweder eingebaut, oder ein externer.

	Dieser speist sein Audio-Signal in diese Maschine ein und bezieht es aus dieser.
	Dazu implementiert er die entsprechenden Item::input(…) und Item::output(…) Funktionen.

	Der Item TapeRecorder kann nicht von AudioIn/Out lesen/schreiben!
*/

#include "Audio/TapeFile.h"
#include "Dsp.h"
#include "Item.h"
#include "Machine.h"
#include "RecentFilesMenu.h"
#include "cpp/cppthreads.h"
#include <QMainWindow>


/*  Implementation Notes

	TapeRecorder ist ein Item
	TapeRecorder wird von einem TapeRecorderInsp gesteuert.
	TapeRecorder wiederum verwaltet ein TapeFile.

	TapeFile unterstützt lesen aus und schreiben in eine Datei.
	TapeFile kann bestimmte Zeitpunkte ansteuern oder bestimmte Blocks.
	TapeFile kann Blocks einfügen und löschen.
	TapeFile kann via output(cc,bit) in einen Block schreiben
	TapeFile kann via input(cc) aus einem Block lesen und dabei automatisch im nächsten Block weitermachen
	TapeFile arbeitet nach außen hin zumeist mit Time-basierten Funktionen,
			 input(), output() und zugehörige Funktionen arbeiten aber Sample-basiert.
	TapeFile besteht aus mehreren TapeFileDataBlock.

	TapeFileDataBlock besteht aus einem CswBuffer, einem TapeData und ggf. einem TapData, O80Data und TzxData.
	TapeFileDataBlock cacht Blockinfos.
	TapeFileDataBlock unterstützt das konvertieren zwischen CswBuffer und den möglichen Unterklassen von TapeData.
	TapeFileDataBlock arbeitet grundsätzlich auf CC-Ebene, hat aber auch einige Time-basierte Convenience-Methoden.

	CswBuffer speichert Lauflängen für 0- und 1-Bits.
	CswBuffer kann aufnehmen und wiedergeben. Dabei verwaltet er eine aktuelle Schreib/Leseposition.
	CswBuffer arbeitet nur Sample-basiert,
			  hat aber eine Variable samples_per_second die von TapeFile oder BlockData gesetzt wird.

	TapeData ist die Basisklasse für
			 AudioData, O80Data, TzxData, RlesData, TapData und evtl. weitere.
	TapeData kann nach CswData und zurück konvertiert werden.
			 Beim Einlesen einer Datei werden zunächst TapeData-Blöcke gespeichert,
			 die dann in Csw-Datablöcke konvertiert werden.
			 Beim Speichern einer Datei werden bei Bedarf CswBuffer oder TapeData anderen Typs konvertiert.
*/


class TapeRecorder : public Item
{
	friend class gui::TapeRecorderInsp;
	friend class gui::WalkmanInspector;

public:
	bool		 auto_start_stop_tape; // switch			read/write should be safe from any thread
	bool		 instant_load_tape;	   // switch			read/write should be safe from any thread
	gui::ListId	 list_id;			   // recent files		const
	const uint32 machine_ccps;		   // cpu cycles per second

private:
	// tape recorder state:
	// write access must be locked with list_lock:
	enum TRState {
		stopped,
		playing,
		winding,
		rewinding
	} state;			 // << > >> state	access must be locked (except for unreliable read access)
	bool record_is_down; // record button	access must be locked (except for unreliable read access)
	bool pause_is_down;	 // pause button		access must be locked (except for unreliable read access)
	Time stop_position;	 // wind / rewind	access must be locked

	// tape file:
	// data is accessed on main thread and audio interrupt
	// mostly accessed from the audio irpt
	// => access must be locked
	// tapefile must only be deleted on main thread and while locked or not running!

	TapeFile* tapefile;


private:
	// action sounds:
	// data is accessed on main thread and audio interrupt
	// => access must be locked via list_lock.

	struct PlaySoundInfo
	{
		int	  id;
		int32 index;
		PlaySoundInfo(int i = 0) : id(i), index(0) {}
	};
	Array<PlaySoundInfo> active_sound;	  // accessed on audio interrupt => access must be locked
	uint				 motor_sound_pos; // access: sound irpt only; in audioBufferEnd()
	void				 play_sound(int s)
	{
		CHECK_LOCK();
		active_sound.append(PlaySoundInfo(s));
	}

protected:
	Sample* sound[18]; // const
	uint32	sound_count[18];
	enum {
		sound_open_deck_empty,
		sound_open_deck_loaded,
		sound_close_deck_empty,
		sound_close_deck_loaded,
		sound_pause_down,
		sound_pause_up,
		sound_play_down,
		sound_play_up,
		sound_ff_down,
		sound_ff_up,
		sound_rewind_down,
		sound_rewind_up,
		sound_record_down,
		sound_record_up,
		sound_motor_empty,
		sound_motor_play,
		sound_motor_ff,
		sound_motor_rewind
	};

	// TR speaker / Ula mic in:
	struct Speaker
	{
		Sample volume; // 0.0 ... 1.0
		// last play position:
		uint   blk;	 // block index in TapeFile
		uint32 pos;	 // pulse index in CswBuffer
		double offs; // cc_offset in pulse (fractional)
		Speaker() : volume(0.05f), blk(0), pos(0), offs(0) {}
	} speaker;

private:
	void  CHECK_LOCK() const volatile { assert(is_locked()); }
	int32 current_cc() { return machine->cpu->cpuCycle(); }
	void  tapefile_stop(CC, bool mute = no);
	void  tapefile_play(CC);
	void  tapefile_record(CC);
	void  play_block();

public:
	TapeRecorder(Machine*, isa_id, const cstr audio_names[], bool auto_start = yes, bool fast_load = yes);
	~TapeRecorder() override;


	// Item interface:
	// overloaded methods

	// void	init			(/*t=0*/ int32 cc) override;
	// void	reset			(Time t, int32 cc) override;
	void input(Time, int32, uint16, uint8&, uint8&) override {}
	void output(Time, int32, uint16, uint8) override {}
	void audioBufferEnd(Time t) override;
	void videoFrameEnd(int32 cc) override;


	// Queries:

	// reliable only while locked
	// unreliable access anytime, e.g. for display
	bool isPlayDown() const volatile noexcept { return state == playing; }	   // PLAY button down
	bool isWinding() const volatile noexcept { return state == winding; }	   // WIND FORE button down
	bool isRewinding() const volatile noexcept { return state == rewinding; }  // WIND BACK button down
	bool isRecordDown() const volatile noexcept { return record_is_down; }	   // RECORD button down
	bool isPauseDown() const volatile noexcept { return pause_is_down; }	   // PAUSE button down
	bool isLoaded() const volatile noexcept { return tapefile != nullptr; }	   // Tapefile loaded
	bool isStopped() const volatile noexcept { return state == stopped; }	   // PLAY, WIND and REWIND all up
	bool isNotRecordDown() const volatile noexcept { return !record_is_down; } // RECORD button NOT down

	// reliable only while locked
	// unreliable access only from main thread (else tapefile might become void)
	bool isWriteProtected() const volatile noexcept
	{
		return tapefile ? tapefile->write_protected : 1;
	} // not loaded or tapefile is write protected
	cstr getFilepath() const volatile noexcept { return tapefile ? tapefile->filepath : nullptr; }
	bool isModified() const volatile noexcept { return tapefile ? tapefile->modified : no; }

	// access only while locked
	Time getTotalPlaytime() const noexcept
	{
		assert(is_locked());
		return tapefile ? tapefile->getTotalPlaytime() : 0;
	}
	Time getCurrentPosition() const noexcept
	{
		assert(is_locked());
		return tapefile ? tapefile->getCurrentPosition() : 0;
	}
	cstr getMajorBlockInfo() const noexcept
	{
		assert(is_locked());
		return tapefile ? tapefile->getMajorBlockInfo() : nullptr;
	}
	cstr getMinorBlockInfo() const noexcept
	{
		assert(is_locked());
		return tapefile ? tapefile->getMinorBlockInfo() : nullptr;
	}


	// Input & Output:

	// access only while locked
	// typically from audio interrupt
	bool	 can_store_block() noexcept;
	bool	 can_read_block() noexcept;
	void	 storeBlock(TapeData*) noexcept;
	TapData* getZxspBlock() noexcept;
	O80Data* getZx80Block() noexcept;
	void	 autoStart(CC) noexcept;
	void	 autoStop(CC) noexcept;
	bool	 isPlaying() const noexcept { return tapefile && state == playing && !pause_is_down && !record_is_down; }
	bool	 isRecording() const noexcept { return tapefile && state == playing && !pause_is_down && record_is_down; }
	bool	 input(int32 cc)
	{
		assert(is_locked());
		assert(isPlaying());
		return tapefile->input(cc);
	}
	void output(int32 cc, bool b)
	{
		assert(is_locked());
		assert(isRecording());
		tapefile->output(cc, b);
	}


	// Actions:

	// must be locked:
	// handle tape recorder buttons, menus etc.
	void	  insert(TapeFile*); // insert tape file
	void	  insert(cstr);		 // insert tape file		***MAY BLOCK FOR LONGISH TIME!!!***
	TapeFile* eject();			 // eject tape file (save file if modified)
	void	  play();			 // push 'play': action depends on pause and record button state
	void	  togglePlay()
	{
		if (isStopped())
			play();
		else
			stop();
	}
	void		  record();				   // push 'record': just set's the button state
	TapeRecorder* pause(bool);			   // toggle 'pause': action depends on other button states
	void		  wind();				   // push '>>': fast forward with auto stop if pause is down
	void		  rewind();				   // push '<<': fast backward with auto stop if pause is down
	void		  stop();				   // play, record, wind and rewind up
	void		  deleteCurrentBlock();	   // delete current block and goto start of next
	void		  newBlockBeforeCurrent(); // add block before current and goto start
	void		  newBlockAfterCurrent();  // add block after current and goto start

	// from main thread only:
	void setFilename(cstr) volatile noexcept;		  // set different filename to save to when ejected
	int	 setWriteProtected(bool f) volatile noexcept; // toggle write protection. returns errno

	// safe any time any thread:
	void setAutoStartStopTape(bool f) volatile noexcept { auto_start_stop_tape = f; }
	void setInstantLoadTape(bool f) volatile noexcept { instant_load_tape = f; }
};


class Plus2TapeRecorder : public TapeRecorder
{
public:
	Plus2TapeRecorder(Machine*, bool auto_start = yes, bool fast_load = yes);
};


class Plus2aTapeRecorder : public TapeRecorder
{
public:
	Plus2aTapeRecorder(Machine*, bool auto_start = yes, bool fast_load = yes);
};


class TS2020 : public TapeRecorder
{
public:
	TS2020(Machine*, bool auto_start = yes, bool fast_load = yes);
};


class Walkman : public TapeRecorder
{
public:
	Walkman(Machine*, bool auto_start = yes, bool fast_load = yes);
};
