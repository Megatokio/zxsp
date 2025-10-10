#pragma once
// Copyright (c) 2002 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Fdc/DivIDE.h"
#include "Files/RzxFile.h"
#include "Interfaces/IMachineController.h"
#include "Joy/ZxIf2.h"
#include "Memory.h"
#include "Multiface/Multiface1.h"
#include "Ram/ExternalRam.h"
#include "SpectraVideo.h"
#include "StereoSample.h"
#include "Templates/NVPtr.h"
#include "Ula/Ula.h"
#include "Ula/UlaZx80.h"
#include "Z80/Z80.h"
#include "zxsp_globals.h"
#include <math.h>


inline double samples_per_dsp_buffer() { return DSP_SAMPLES_PER_BUFFER; }
inline Time	  seconds_per_dsp_buffer() { return DSP_SAMPLES_PER_BUFFER / samples_per_second; }
inline Time	  seconds_per_dsp_buffer_max()
{
	return (DSP_SAMPLES_PER_BUFFER + DSP_SAMPLES_STITCHING - 0.01) / samples_per_second;
}


class Items : private Array<std::shared_ptr<Item>>
{
public:
	using Array::append;
	using Array::count;
	using Array::drop;
	using Array::last;
	using Array::remove;
	using Array::operator[];
	using Array::indexof;

	uint indexof(const volatile Item* item) const
	{
		uint i = count();
		while (i-- && data[i].get() != item) {}
		return i; // i = ~0u if not found
	}

	bool contains(const volatile Item* item) { return indexof(item) != ~0u; }

	void remove(Item* item)
	{
		uint i = indexof(item);
		if (i != ~0u) remove(i);
	}

	template<typename ITEM>
	ITEM* find()
	{
		for (uint i = count(); --i;)
		{
			if (ITEM* item = dynamic_cast<ITEM*>(data[i].get())) return item;
		}
		return nullptr;
	}
};


class Machine : public IsaObject
{
	NO_COPY_MOVE(Machine);

	friend class IMachineController;
	friend class Item;
	friend class Z80;

public:
	std::timed_mutex mutex;

	void lock() volatile { NV(mutex).lock(); }
	void unlock() volatile { NV(mutex).unlock(); }
	bool trylock() volatile { return NV(mutex).try_lock(); }
	bool trylock(int timeout_nsec) volatile { return NV(mutex).try_lock_for(std::chrono::nanoseconds(timeout_nsec)); }

	volatile IMachineController* controller;

	// general info
	const Model			model;
	const ZxInfo* const model_info; // generic model info

	// options
	uint32	  cpu_options; // cpu_waitmap | cpu_crtc | cpu_break_sp | cpu_break_rwx
	CoreByte* break_ptr;   // Z80options.h
	bool	  audio_in_enabled;

private:
	bool is_power_on;
	bool is_suspended;

	bool		   rzx_auto_start_recording = no; // TODO: auto start recording should be fully handled by controller
	class RzxFile* rzx_file;					  // Rzx Replay and Recording
	void		   rzxLoadSnapshot(int32& cc_final, int32& ic_end);
	void		   rzxStoreSnapshot();

public:
	// all memory in the machine:
	Array<Memory*> memory; // updated by memoryAdded() / memoryRemoved()
	MemoryPtr	   rom;	   // rom pages
	MemoryPtr	   ram;	   // ram pages

	void memoryAdded(Memory*);
	void memoryRemoved(Memory*);
	void memoryModified(Memory*, uint how = 2); // 0=added, 1=removed, 2=modified


public:
	// All components:
	Items all_items;

	// Mostly internal components:
	Z80*		  cpu; // the cpu: always first in list of items
	Ula*		  ula;
	Mmu*		  mmu;
	Keyboard*	  keyboard;
	Ay*			  ay;
	Joy*		  joystick;
	TapeRecorder* taperecorder;
	Fdc*		  fdc;
	Printer*	  printer;
	Crtc*		  crtc; // mostly same as ula. not update by addItem()/removeItem()!
	Item*		  last_item() const { return all_items.count() ? all_items.last().get() : nullptr; }

public:
	Item*		  addItem(Item*);
	Item*		  addExternalItem(isa_id);
	ExternalRam*  addExternalRam(isa_id, uint size_or_options = 0);
	SpectraVideo* addSpectraVideo(uint dip_switches);
	DivIDE*		  addDivIDE(uint ramsize, cstr romfile);
	Multiface1*	  addMultiface1(bool joystick_enabled);

	void removeItem(Item*);
	void removeItem(isa_id id) { removeItem(findItem(id)); }
	void removeSpectraVideo();

	template<typename ITEM>
	void remove()
	{
		removeItem(find<ITEM>());
	}

	template<typename ITEM>
	ITEM* find()
	{
		return all_items.find<ITEM>(); // nullptr if not found
	}

	Item*		  findItem(isa_id id); // kind of an Item
	ZxIf2*		  findZxIf2() { return find<ZxIf2>(); }
	SpectraVideo* findSpectraVideo() { return find<SpectraVideo>(); }
	DivIDE*		  findDivIDE() { return find<DivIDE>(); }

	bool contains(const volatile Item* item) { return all_items.contains(item); }

	void setCrtc(Crtc* c) { cpu->setCrtc(crtc = c); }


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
	double	  tcc0;		 // realtime t (biased to total_realtime) at current frame start (cc=0)

	uint  beam_cnt; // for drawVideoBeamIndicator()
	int32 beam_cc;	// ""

	StereoSample*		audio_out_buffer = nullptr; //[DSP_SAMPLES_PER_BUFFER + DSP_SAMPLES_STITCHING] = {0};
	const StereoSample* audio_in_buffer	 = nullptr; //[DSP_SAMPLES_PER_BUFFER + DSP_SAMPLES_STITCHING]  = {0};

public:
	//bool isAudioInputDeviceEnabled() const volatile noexcept { return audio_input_device_enabled; }
	//bool isAudioOutputDeviceEnabled() const volatile noexcept { return audio_output_device_enabled; }
	//Sample getOutputVolume() const volatile noexcept { return audio_output_volume; }

	void outputSamples(const StereoSample&, Time start, Time end);
	void outputSamples(Sample, Time start, Time end);

	void shiftBuffer(StereoSample* bu)
	{
		for (int i = 0; i < DSP_SAMPLES_STITCHING; i++) bu[i] = bu[DSP_SAMPLES_PER_BUFFER + i];
	}
	void clearBuffer(StereoSample* bu) // preserves stitching at buffer start
	{
		for (int i = DSP_SAMPLES_STITCHING; i < DSP_SAMPLES_PER_BUFFER + DSP_SAMPLES_STITCHING; i++) bu[i] = 0.0;
	}

protected:
	Time   t_for_cc(int32 cc) { return tcc0 + cc / cpu_clock; }
	Time   t_for_cc_lim(int32 cc) { return min(t_for_cc(cc), seconds_per_dsp_buffer_max()); }
	double cc_for_t(Time t) { return (t - tcc0) * cpu_clock; }
	int32  cc_dn_for_t(Time t) { return int32(floor(cc_for_t(t))); }
	int32  cc_up_for_t(Time t) { return int32(ceil(cc_for_t(t))); }

	// machine state
	void		  clearBreakPtr();
	uint8		  handleRomPatch(uint16, uint8); // Z80options.h
	void		  outputAtCycle(int32 cc, uint16, uint8);
	uchar		  inputAtCycle(int32 cc, uint16);
	uint8		  interruptAtCycle(int32 cc, uint16 addr) { return ula->interruptAtCycle(cc, addr); }
	virtual int32 nmiAtCycle(int32 /*cc_nmi*/) { return 0x7FFFFFFF; }
	virtual bool  handleLoadTapePatch() = 0;
	virtual bool  handleSaveTapePatch() = 0;
	uint8		  readMemMappedPort(int32 cc, uint16, uint8);  // for memory mapped i/o
	void		  writeMemMappedPort(int32 cc, uint16, uint8); // for memory mapped i/o
	void		  videoFrameEnd(int32 cc);
	void		  audioBufferEnd(Time t);

public:
	bool rzxIsLoaded() const volatile { return rzx_file != nullptr; }
	bool rzxIsPlaying() const { return rzx_file != nullptr && rzx_file->isPlaying(); }
	bool rzxIsRecording() const { return rzx_file != nullptr && rzx_file->isRecording(); }

	void rzxPlayFile(RzxFile*, bool auto_start_recording); // take-over and play the supplied RzxFile
	void rzxDispose();
	void rzxStartRecording(cstr msg = nullptr, bool yellow = no);
	void rzxStopRecording(cstr msg = nullptr, bool yellow = no);
	void rzxStopPlaying(cstr msg = nullptr, bool yellow = no);
	void rzxOutOfSync(cstr msg, bool alert = no);
	void rzxSetAutoStartRecording(bool f) volatile { rzx_auto_start_recording = f; }

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
	// virtual void loadTap(FD& fd);		// MachineZxsp.cpp

	void loadSZX(FD&);	  // file_szx.cpp
	void saveSZX(FD&);	  // file_szx.cpp
	void loadZ80(FD& fd); // file_z80.cpp
	void saveZ80(FD& fd); // file_z80.cpp
	void loadRom(FD& fd);
	void saveRom(FD& fd);

protected:
	Machine(IMachineController*, Model, isa_id);

	void showMessage(MessageStyle s, cstr text);

private:
	void init_contended_ram();
	void load_rom();
	void szx_add_joystick(uint if_id, JoystickID js_id);

	void loadZ80_attach_joysticks(uint); // helper


	// ---- P U B L I C ----------------------------------------------------

public:
	static std::shared_ptr<Machine> newMachine(IMachineController*, Model);

	~Machine() override;

	void saveAs(cstr filepath);

	// Time & Utilities:
	int32 current_cc() { return cpu->cpuCycle(); }
	Time  now() { return t_for_cc(cpu->cpuCycle()); }
	Time  now_lim() { return t_for_cc_lim(cpu->cpuCycle()); }

	// Control & Run the Machine:
	bool is_locked() volatile
	{
		if (!is_power_on) return yes;
		if (is_suspended) return yes;
		bool f = NV(mutex).try_lock();
		if (f) NV(mutex).unlock();
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
	void runForSound(const StereoBuffer audio_in_buffer, StereoBuffer audio_out_buffer, int32 up_to_cc = 0);
	void runCpuCycles(int32 cc); // debugger

	void stepIn();
	void stepOver();
	void stepOut();
	void _suspend() { is_suspended = true; }
	void _resume() { is_suspended = false; }
	bool suspend() volatile;
	void resume() volatile;
	bool isRunning() const volatile { return !is_suspended; }
	bool isSuspended() const volatile { return is_suspended; }

	void drawVideoBeamIndicator();

	// set speed of the emulated world:
	void setSpeedFromCpuClock(Frequency realworld_cpu_clock);
	void speedupTo60fps();
	void setSpeedAnd60fps(double factor);

	void set50Hz() { set60Hz(0); } // 100%, ~50fps, stored in prefs
	void set60Hz(bool = 1);		   // 100%, ~60fps, stored in prefs

	//Handle Input Devices:
	uint8		joystick_buttons[MAX_USB_JOYSTICKS + 2] = {0}; // state of real-world joysticks
	uint8		kbd_joystick_active						= 0;   // set to ff whenever read
	uint8		mouse_buttons							= 0;
	zxsp::Point mouse_position							= {0, 0};

	void keyDown(uint16 unicode, uint8 oskeycode, KeyboardModifiers);
	void keyUp(uint16 unicode, uint8 oskeycode, KeyboardModifiers);
	void allKeysAndButtonsUp();
	void updateJoystickButtons(JoystickID id, JoystickButtons btns) volatile { joystick_buttons[id] = btns; }
	void updateMouseButtons(MouseButtons btns) volatile { mouse_buttons = btns; }
	void mouseMoved(zxsp::Dist d) volatile { NV(mouse_position) += d; }

	Keymap getKeymap() const volatile;
	uint8  getJoystickButtons(JoystickID id) // FUDLR
	{
		assert_lt(uint(id), NELEM(joystick_buttons));
		if (id == kbd_joystick) kbd_joystick_active = 100;
		return joystick_buttons[id];
	}
	uint8		peekJoystickButtons(JoystickID id) const volatile { return joystick_buttons[id]; } // FUDLR
	uint8		getMouseButtons() const volatile { return mouse_buttons; }
	zxsp::Point getMousePosition() const volatile { return NV(mouse_position); }
};


// =============================================================================================
