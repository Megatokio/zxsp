#pragma once
/*	Copyright  (c)	Günter Woigk 2002 - 2019
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

#include <math.h>
#include "kio/kio.h"
#include "cpp/cppthreads.h"
#include "zxsp_types.h"
#include "StereoSample.h"
#include "globals.h"
#include "Ula/Ula.h"
#include "Ula/UlaZx81.h"
#include "ZxInfo/ZxInfo.h"
#include "IsaObject.h"
#include "Z80/Z80.h"
#include "Templates/RCPtr.h"
#include "Memory.h"
#include "Overlays/Overlay.h"
#include "Files/RzxFile.h"


class Machine : public IsaObject
{
	friend void runMachinesForSound();
	friend class MachineController;
	friend class Item;
	friend class Z80;

public:
	PLock			_lock;

	volatile MachineController*	controller;

// general info
	Model			model;
	ZxInfo const*	model_info;			// generic model info

// options
	uint32			cpu_options;		// cpu_waitmap | cpu_crtc | cpu_break_sp | cpu_break_rwx
	CoreByte*		break_ptr;			// Z80options.h
	bool			audio_in_enabled;

private:
	bool			is_power_on;
	bool			is_suspended;

	class RzxFile*	rzx_file;			// Rzx Replay and Recording
	Overlay*		overlay_rzx_play;
	Overlay*		overlay_rzx_record;
	void			show_overlay_play();
	void			show_overlay_record();
	void			hide_overlay_play();
	void			hide_overlay_record();
	void			rzx_load_snapshot(int32& cc_final, int32& ic_end);
	void			rzx_store_snapshot();

public:
// all memory in the machine:
	Array<Memory*>	memory;				// updated by memoryAdded() / memoryRemoved()
	void			memoryAdded(Memory*);
	void			memoryRemoved(Memory*);
	void			memoryModified(Memory*, uint how=2); // 0=added, 1=removed, 2=modified

// Built-in components:
	MemoryPtr		rom;				// rom pages
	MemoryPtr		ram;				// ram pages
	class Z80*		cpu;				// the cpu: always first in list of items
	Ula*			ula;
	Mmu*			mmu;
	Keyboard*		keyboard;
	Ay*				ay;
	Joy*			joystick;
	TapeRecorder*	taperecorder;
	Fdc*			fdc;
	Printer*		printer;
	Crtc*			crtc;				// mostly same as ula
	Item*			lastitem;			// list of all items: maintained by Item

// virtual machine time:
//	 there are two scales: cpu T cycle count cc and time in seconds.
//	 cc are biased to the current video frame start
//	 time is biased to the current dsp audio buffer start
//	 => absolute times are not needed internally.
//	 time advances when the cpu runs and increments it's cycle counter.
	int32			total_frames;		// information: accumulated frames until now
	double			total_cc;			// information: accumulated cpu T cycles until now
	int32			total_buffers;		// information: accumulated dsp buffers until now
	double			total_realtime;		// information: accumulated time[sec] until now

	Frequency		cpu_clock;			// cpu T cycles per second: cpu speed and conversion cc <-> time
	//int32			cpu->cc;			// now[cc] = total_cc + cpu->cc
	double			tcc0;				// realtime t (biased to total_realtime) at current frame start (cc=0)

	uint			beam_cnt;			// für drawVideoBeamIndicator()
	int32			beam_cc;			// ""

protected:
	Time			t_for_cc			(int32 cc)		{ return tcc0 + cc / cpu_clock; }
	Time			t_for_cc_lim		(int32 cc)		{ return min( t_for_cc(cc), seconds_per_dsp_buffer_max()); }
	//int32			cc					()				{ return cpu->cpuCycle(); }
	double			cc_for_t			(Time t)		{ return (t-tcc0) * cpu_clock; }
	int32			cc_dn_for_t			(Time t)		{ return int32(floor(cc_for_t(t))); }
	int32			cc_up_for_t			(Time t)		{ return int32(ceil(cc_for_t(t))); }

// machine state
	void		clear_break_ptr		();
	uint8		handleRomPatch      (uint16,uint8);		// Z80options.h
	void		outputAtCycle       (int32 cc, uint16, uint8);
	uchar		inputAtCycle        (int32 cc, uint16);
	uint8		interruptAtCycle	(int32 cc, uint16 addr)	{ return ula->interruptAtCycle(cc,addr); }
VIR	int32		nmiAtCycle			(int32 /*cc_nmi*/)		{ return 0x7FFFFFFF; }
VIR	bool		handleLoadTapePatch	()=0;
VIR bool		handleSaveTapePatch	()=0;
	uint8		readMemMappedPort	(int32 cc, uint16, uint8);	// for memory mapped i/o
	void		writeMemMappedPort	(int32 cc, uint16, uint8);	// for memory mapped i/o

	bool		rzxIsLoaded()		volatile const	{ return rzx_file!=NULL; }
	bool		rzxIsPlaying()		const			{ return rzx_file!=NULL && rzx_file->isPlaying(); }
	bool		rzxIsRecording()	const			{ return rzx_file!=NULL && rzx_file->isRecording(); }

	void		rzxPlayFile(RzxFile*);	// take-over and play the supplied RzxFile
	void		rzxDispose();
	void		rzxStartRecording(cstr msg=NULL, bool yellow=no);
	void		rzxStopRecording(cstr msg=NULL, bool yellow=no);
	void		rzxStopPlaying(cstr msg=NULL, bool yellow=no)		{ (void)msg; (void)yellow; TODO(); }
	void		rzxOutOfSync(cstr msg, bool alert=no);

	Machine( MachineController*, Model, isa_id );

private:
	void		init_contended_ram	();
	void		load_rom			();

// in Files/*.cpp:
VIR void		loadAce			(FD& fd) throws;						// MachineJupiter.cpp
VIR void		saveAce			(FD &fd) throws;						// MachineJupiter.cpp
VIR void		loadSna			(FD &fd) throws;						// MachineZxsp.cpp
VIR void		saveSna			(FD &fd) throws;						// MachineZxsp.cpp
VIR void		loadScr			(FD& fd) throws;						// MachineZxsp.cpp
VIR void		saveScr			(FD &fd) throws;						// MachineZxsp.cpp
VIR void		loadO80			(FD &fd) throws;						// MachineZx80.cpp
VIR void		saveO80			(FD &fd) throws;						// MachineZx80.cpp
VIR void		loadP81			(FD &fd, bool p81) throws;				// MachineZx81.cpp
VIR void		saveP81			(FD &fd, bool p81) throws;				// MachineZx81.cpp
//VIR void		loadTap			(FD& fd) throws;						// MachineZxsp.cpp
	void		loadZ80			(FD& fd) throws;						// file_z80.cpp
	void		saveZ80			(FD& fd) throws;						// file_z80.cpp
	void		loadRom			(FD& fd) throws;
	void		saveRom			(FD& fd) throws;

	void		loadZ80_attach_joysticks(uint);	// helper



// ---- P U B L I C ----------------------------------------------------

public:
	virtual		~Machine();

	void		saveAs(cstr filepath) throws;

// Components:
	Item*		findItem			(isa_id id)     {for(Item*i=lastitem;i;i=i->prev()){if(i->isaId()==id)return i;} return 0;}
	Item*		findIsaItem			(isa_id id)     {for(Item*i=lastitem;i;i=i->prev()){if(i->isA(id))return i;} return 0;}
	//Item const* findIsaItem		(isa_id id) const {return const_cast<Machine*>(this)->findIsaItem(id); }
	Item*		findInternalItem	(isa_id id)     {for(Item*i=firstItem();i;i=i->next()){if(i->isA(id)&&i->isInternal())return i;} return 0;}
	Item*		lastItem			() volatile		{return lastitem;}
	Item*		firstItem			()				{return cpu;}
	ZxIf2*		findZxIf2			()				{return ZxIf2Ptr(findIsaItem(isa_ZxIf2)); }
	SpectraVideo* findSpectraVideo	()				{return SpectraVideoPtr(findIsaItem(isa_SpectraVideo)); }
	void		setCrtc				(Crtc* c)		{crtc=c; cpu->setCrtc(c);}

	Item*		addExternalItem		(isa_id);
	void		removeItem			(Item* item)    { delete item; }
	void		removeItem			(isa_id id)		{ delete findItem(id); }
	void		removeIsaItem		(isa_id id)		{ delete findIsaItem(id); }
	SpectraVideo*addSpectraVideo	(bool add=yes);

	void		itemAdded			(Item*);		// callback from Item c'tor
	void		itemRemoved			(Item*);		// callback from Item d'tor

	OverlayJoystick* addOverlay		(Joystick*, cstr idf, Overlay::Position);
	void		removeOverlay		(Overlay*);

// Time & Utilities:
	int32		current_cc			()				{ return cpu->cpuCycle(); }
	Time		now                 ()				{ return t_for_cc(cpu->cpuCycle()); }
	Time		now_lim				()				{ return t_for_cc_lim(cpu->cpuCycle()); }

// Control & Run the Machine:
	void		lock				() volatile		{ _lock.lock(); }
	void		unlock				() volatile		{ _lock.unlock(); }
	bool		is_locked			() volatile		{ if(!is_power_on) return yes;
													  if(is_suspended) return yes;
													  bool f = _lock.trylock(); if(f) _lock.unlock();
													  return !f; }

	void		_power_on			(/*t=0*/ int32 start_cc = 1000);
	void		_power_off			();
	void		powerOn				(/*t=0*/ int32 start_cc = 1000) volatile;
	bool		powerOff() volatile;
	void		powerCycle			() volatile;
	bool		isPowerOn			() volatile const	{ return is_power_on; }
	bool		isPowerOff			() volatile const	{ return is_power_on == no; }
	void		reset				();				// at current t & cc
	void		nmi					();				// at current t & cc
	void		installRomPatches   (bool=yes);
	void		runForSound			(int32 up_to_cc=0);
	void		runCpuCycles		(int32 cc);		// debugger

	void		stepIn				();
	void		stepOver			();
	void		stepOut				();
	void		_suspend			();
	void		_resume				();
	bool		suspend				() volatile;
	void		resume				() volatile;
	bool		isRunning			() const volatile	{ return !is_suspended; }
	bool		isSuspended			() const volatile	{ return  is_suspended; }

	void		drawVideoBeamIndicator();

	// set speed of the emulated world:
	void		setSpeedFromCpuClock(Frequency realworld_cpu_clock);
	void		speedupTo60fps		();
	void		setSpeedAnd60fps	(double factor);

	void		set50Hz()			{set60Hz(0);}	// 100%, ~50fps, stored in prefs
	void		set60Hz(bool=1);					// 100%, ~60fps, stored in prefs
};























