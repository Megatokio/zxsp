#pragma once
// Copyright (c) 2002 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Files/RzxFile.h"
#include "IsaObject.h"
#include "Memory.h"
#include "Overlays/Overlay.h"
#include "StereoSample.h"
#include "Templates/RCPtr.h"
#include "Ula/Ula.h"
#include "Ula/UlaZx80.h"
#include "Z80/Z80.h"
#include "ZxInfo/ZxInfo.h"
#include "cpp/cppthreads.h"
#include "globals.h"
#include "kio/kio.h"
#include "zxsp_types.h"
#include <math.h>


class Machine : public IsaObject
{
	friend void runMachinesForSound();
	friend class gui::MachineController;
	friend class Item;
	friend class Z80;

public:
	PLock _lock;

	volatile gui::MachineController* controller;

	// general info
	Model		  model;
	const ZxInfo* model_info; // generic model info

	// options
	uint32	  cpu_options; // cpu_waitmap | cpu_crtc | cpu_break_sp | cpu_break_rwx
	CoreByte* break_ptr;   // Z80options.h
	bool	  audio_in_enabled;

private:
	bool is_power_on;
	bool is_suspended;

	class RzxFile* rzx_file; // Rzx Replay and Recording
	gui::Overlay*  overlay_rzx_play;
	gui::Overlay*  overlay_rzx_record;
	void		   show_overlay_play();
	void		   show_overlay_record();
	void		   hide_overlay_play();
	void		   hide_overlay_record();
	void		   rzx_load_snapshot(int32& cc_final, int32& ic_end);
	void		   rzx_store_snapshot();

public:
	// all memory in the machine:
	Array<Memory*> memory; // updated by memoryAdded() / memoryRemoved()
	void		   memoryAdded(Memory*);
	void		   memoryRemoved(Memory*);
	void		   memoryModified(Memory*, uint how = 2); // 0=added, 1=removed, 2=modified

	// Built-in components:
	MemoryPtr	  rom; // rom pages
	MemoryPtr	  ram; // ram pages
	class Z80*	  cpu; // the cpu: always first in list of items
	Ula*		  ula;
	Mmu*		  mmu;
	Keyboard*	  keyboard;
	Ay*			  ay;
	Joy*		  joystick;
	TapeRecorder* taperecorder;
	Fdc*		  fdc;
	Printer*	  printer;
	Crtc*		  crtc;		// mostly same as ula
	Item*		  lastitem; // list of all items: maintained by Item

	// virtual machine time:
	//	 there are two scales: cpu T cycle count cc and time in seconds.
	//	 cc are biased to the current video frame start
	//	 time is biased to the current dsp audio buffer start
	//	 => absolute times are not needed internally.
	//	 time advances when the cpu runs and increments it's cycle counter.
	int32  total_frames;   // information: accumulated frames until now
	double total_cc;	   // information: accumulated cpu T cycles until now
	int32  total_buffers;  // information: accumulated dsp buffers until now
	double total_realtime; // information: accumulated time[sec] until now

	Frequency cpu_clock; // cpu T cycles per second: cpu speed and conversion cc <-> time
	// int32			cpu->cc;			// now[cc] = total_cc + cpu->cc
	double tcc0; // realtime t (biased to total_realtime) at current frame start (cc=0)

	uint  beam_cnt; // für drawVideoBeamIndicator()
	int32 beam_cc;	// ""

protected:
	Time t_for_cc(int32 cc) { return tcc0 + cc / cpu_clock; }
	Time t_for_cc_lim(int32 cc) { return min(t_for_cc(cc), seconds_per_dsp_buffer_max()); }
	// int32			cc					()				{ return cpu->cpuCycle(); }
	double cc_for_t(Time t) { return (t - tcc0) * cpu_clock; }
	int32  cc_dn_for_t(Time t) { return int32(floor(cc_for_t(t))); }
	int32  cc_up_for_t(Time t) { return int32(ceil(cc_for_t(t))); }

	// machine state
	void		  clear_break_ptr();
	uint8		  handleRomPatch(uint16, uint8); // Z80options.h
	void		  outputAtCycle(int32 cc, uint16, uint8);
	uchar		  inputAtCycle(int32 cc, uint16);
	uint8		  interruptAtCycle(int32 cc, uint16 addr) { return ula->interruptAtCycle(cc, addr); }
	virtual int32 nmiAtCycle(int32 /*cc_nmi*/) { return 0x7FFFFFFF; }
	virtual bool  handleLoadTapePatch() = 0;
	virtual bool  handleSaveTapePatch() = 0;
	uint8		  readMemMappedPort(int32 cc, uint16, uint8);  // for memory mapped i/o
	void		  writeMemMappedPort(int32 cc, uint16, uint8); // for memory mapped i/o

	bool rzxIsLoaded() const volatile { return rzx_file != nullptr; }
	bool rzxIsPlaying() const { return rzx_file != nullptr && rzx_file->isPlaying(); }
	bool rzxIsRecording() const { return rzx_file != nullptr && rzx_file->isRecording(); }

	void rzxPlayFile(RzxFile*); // take-over and play the supplied RzxFile
	void rzxDispose();
	void rzxStartRecording(cstr msg = nullptr, bool yellow = no);
	void rzxStopRecording(cstr msg = nullptr, bool yellow = no);
	void rzxStopPlaying(cstr msg = nullptr, bool yellow = no)
	{
		(void)msg;
		(void)yellow;
		TODO();
	}
	void rzxOutOfSync(cstr msg, bool alert = no);

	Machine(gui::MachineController*, Model, isa_id);

private:
	void init_contended_ram();
	void load_rom();

	// in Files/*.cpp:
	virtual void loadAce(FD& fd);			// MachineJupiter.cpp
	virtual void saveAce(FD& fd);			// MachineJupiter.cpp
	virtual void loadSna(FD& fd);			// MachineZxsp.cpp
	virtual void saveSna(FD& fd);			// MachineZxsp.cpp
	virtual void loadScr(FD& fd);			// MachineZxsp.cpp
	virtual void saveScr(FD& fd);			// MachineZxsp.cpp
	virtual void loadO80(FD& fd);			// MachineZx80.cpp
	virtual void saveO80(FD& fd);			// MachineZx80.cpp
	virtual void loadP81(FD& fd, bool p81); // MachineZx81.cpp
	virtual void saveP81(FD& fd, bool p81); // MachineZx81.cpp

	// virtual void		loadTap			(FD& fd);						// MachineZxsp.cpp

	void loadZ80(FD& fd); // file_z80.cpp
	void saveZ80(FD& fd); // file_z80.cpp
	void loadRom(FD& fd);
	void saveRom(FD& fd);

	void loadZ80_attach_joysticks(uint); // helper


	// ---- P U B L I C ----------------------------------------------------

public:
	static Machine* newMachine(gui::MachineController*, Model);

	~Machine() override;

	void saveAs(cstr filepath);

	// Components:
	Item* findItem(isa_id id)
	{
		for (Item* i = lastitem; i; i = i->prev())
		{
			if (i->isaId() == id) return i;
		}
		return 0;
	}
	Item* findIsaItem(isa_id id)
	{
		for (Item* i = lastitem; i; i = i->prev())
		{
			if (i->isA(id)) return i;
		}
		return 0;
	}
	// Item const* findIsaItem		(isa_id id) const {return const_cast<Machine*>(this)->findIsaItem(id); }
	Item* findInternalItem(isa_id id)
	{
		for (Item* i = firstItem(); i; i = i->next())
		{
			if (i->isA(id) && i->isInternal()) return i;
		}
		return 0;
	}
	Item*		  lastItem() volatile { return lastitem; }
	Item*		  firstItem() { return cpu; }
	ZxIf2*		  findZxIf2() { return ZxIf2Ptr(findIsaItem(isa_ZxIf2)); }
	SpectraVideo* findSpectraVideo() { return SpectraVideoPtr(findIsaItem(isa_SpectraVideo)); }
	void		  setCrtc(Crtc* c)
	{
		crtc = c;
		cpu->setCrtc(c);
	}

	Item*		  addExternalItem(isa_id);
	ExternalRam*  addExternalRam(isa_id, uint size_or_options = 0);
	void		  removeItem(Item* item) { delete item; }
	void		  removeItem(isa_id id) { delete findItem(id); }
	void		  removeIsaItem(isa_id id) { delete findIsaItem(id); }
	SpectraVideo* addSpectraVideo(uint dip_switches);
	void		  removeSpectraVideo();
	DivIDE*		  addDivIDE(uint ramsize, cstr romfile);
	Multiface1*	  addMultiface1(bool joystick_enabled);

	void itemAdded(Item*);	 // callback from Item c'tor
	void itemRemoved(Item*); // callback from Item d'tor

	gui::OverlayJoystick* addOverlay(Joystick*, cstr idf, gui::Overlay::Position);
	void				  removeOverlay(gui::Overlay*);

	// Time & Utilities:
	int32 current_cc() { return cpu->cpuCycle(); }
	Time  now() { return t_for_cc(cpu->cpuCycle()); }
	Time  now_lim() { return t_for_cc_lim(cpu->cpuCycle()); }

	// Control & Run the Machine:
	void lock() volatile { _lock.lock(); }
	void unlock() volatile { _lock.unlock(); }
	bool is_locked() volatile
	{
		if (!is_power_on) return yes;
		if (is_suspended) return yes;
		bool f = _lock.trylock();
		if (f) _lock.unlock();
		return !f;
	}

	void _power_on(/*t=0*/ int32 start_cc = 1000);
	void _power_off();
	void powerOn(/*t=0*/ int32 start_cc = 1000) volatile;
	bool powerOff() volatile;
	void powerCycle() volatile;
	bool isPowerOn() const volatile { return is_power_on; }
	bool isPowerOff() const volatile { return is_power_on == no; }
	void reset(); // at current t & cc
	void nmi();	  // at current t & cc
	void installRomPatches(bool = yes);
	void runForSound(int32 up_to_cc = 0);
	void runCpuCycles(int32 cc); // debugger

	void stepIn();
	void stepOver();
	void stepOut();
	void _suspend();
	void _resume();
	bool suspend() volatile;
	void resume() volatile;
	bool isRunning() const volatile { return !is_suspended; }
	bool isSuspended() const volatile { return is_suspended; }

	void drawVideoBeamIndicator();

	// set speed of the emulated world:
	void setSpeedFromCpuClock(Frequency realworld_cpu_clock);
	void speedupTo60fps();
	void setSpeedAnd60fps(double factor);

	void set50HzNeu() { set60HzNeu(0); } // 100%, ~50fps, stored in prefs
	void set60HzNeu(bool = 1);			 // 100%, ~60fps, stored in prefs
};
