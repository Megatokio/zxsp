// Copyright (c) 2002 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#define LOGLEVEL 0
#include "Machine.h"
#include "Application.h"
#include "Audio/TapeFile.h"
#include "Ay/Ay.h"
#include "Ay/AySubclasses.h"
#include "Ay/FullerBox.h"
#include "CurrahMicroSpeech.h"
#include "Dsp.h"
#include "Fdc/DivIDE.h"
#include "Fdc/FdcBeta128.h"
#include "Fdc/FdcD80.h"
#include "Fdc/FdcJLO.h"
#include "Fdc/FdcPlus3.h"
#include "Fdc/FdcPlusD.h"
#include "Files/RzxFile.h"
#include "Files/Z80Head.h"
#include "Grafpad.h"
#include "IcTester.h"
#include "Inspector/Inspector.h"
#include "Joy/CursorJoy.h"
#include "Joy/DktronicsDualJoy.h"
#include "Joy/InvesJoy.h"
#include "Joy/KempstonJoy.h"
#include "Joy/SinclairJoy.h"
#include "Joy/Tc2048Joy.h"
#include "Joy/Tc2068Joy.h"
#include "Joy/Tk85Joy.h"
#include "Joy/ZxIf2.h"
#include "KempstonMouse.h"
#include "Keyboard.h"
#include "Libraries/kio/TestTimer.h"
#include "MachineController.h"
#include "MachineJupiter.h"
#include "MachineTc2048.h"
#include "MachineTc2068.h"
#include "MachineZx128.h"
#include "MachineZx80.h"
#include "MachineZx81.h"
#include "MachineZxPlus2a.h"
#include "MachineZxPlus3.h"
#include "MachineZxsp.h"
#include "Multiface/Multiface1.h"
#include "Multiface/Multiface128.h"
#include "Multiface/Multiface3.h"
#include "Printer/PrinterAerco.h"
#include "Printer/PrinterLprint3.h"
#include "Printer/PrinterPlus3.h"
#include "Printer/PrinterTs2040.h"
#include "Printer/ZxPrinter.h"
#include "Qt/Settings.h"
#include "Ram/Cheetah32kRam.h"
#include "Ram/Jupiter16kRam.h"
#include "Ram/Memotech64kRam.h"
#include "Ram/Zx16kRam.h"
#include "Ram/Zx3kRam.h"
#include "Screen/Screen.h"
#include "SpectraVideo.h"
#include "TapeRecorder.h"
#include "Templates/NVPtr.h"
#include "ToolWindow.h"
#include "Ula/Mmu.h"
#include "Ula/Mmu128k.h"
#include "Ula/MmuInves.h"
#include "Ula/MmuJupiter.h"
#include "Ula/MmuPlus3.h"
#include "Ula/MmuTc2048.h"
#include "Ula/MmuTc2068.h"
#include "Ula/MmuTk85.h"
#include "Ula/MmuTs1500.h"
#include "Ula/MmuZx80.h"
#include "Ula/MmuZx81.h"
#include "Ula/MmuZxsp.h"
#include "Ula/Ula128k.h"
#include "Ula/UlaInves.h"
#include "Ula/UlaJupiter.h"
#include "Ula/UlaPlus3.h"
#include "Ula/UlaZx80.h"
#include "Ula/UlaZx81.h"
#include "Ula/UlaZxsp.h"
#include "WindowMenu.h"
#include "Z80/Z80.h"
#include "Z80/Z80opcodes.h"
#include "ZxIf1.h"
#include "ZxInfo.h"
#include "unix/FD.h"


class MachineList : private Array<volatile Machine*>
{
	using Array<volatile Machine*>::cnt;
	using Array<volatile Machine*>::data;

public:
	PLock _lock;

	void append(volatile Machine* m)
	{
		PLocker z(_lock);
		Array::append(m);
	}
	void remove(volatile Machine* m)
	{
		PLocker z(_lock);
		// Array::removeitem(m);
		for (uint i = 0; i < cnt; i++)
		{
			if (m == data[i])
			{
				Array::remove(i);
				return;
			}
		}
	}

	uint			  count() { return cnt; }
	volatile Machine* operator[](uint i)
	{
		assert(i < cnt);
		return data[i];
	}
};


static MachineList machine_list;
volatile void*	   front_machine = nullptr;


void runMachinesForSound()
{
	PLocker z(machine_list._lock);

	for (uint i = 0; i < machine_list.count(); i++)
	{
		NVPtr<Machine> machine(machine_list[i]);
		if (machine->isPowerOn()) // not suspended by controller
		{
			if (machine->isRunning()) // cpu not suspended for debugger
			{
				machine->runForSound();
				if (machine->cpu_clock <= 100000) machine->drawVideoBeamIndicator();
			}
			else { machine->drawVideoBeamIndicator(); }
		}
	}
}


// ########################################################################
// Utilities:
// ########################################################################


#define ForAllItems(WHAT)                                                                                              \
  {                                                                                                                    \
	Item* p = lastitem;                                                                                                \
	do {                                                                                                               \
	  p->WHAT;                                                                                                         \
	}                                                                                                                  \
	while ((p = p->prev()));                                                                                           \
  }


// ########################################################################
// Create & Kill:
// ########################################################################


/*  CREATOR:
	create the Machine with 'model'
	Creates Ram and Rom
	but does not create all other itmes! (CPU, Ula, Mmu, Keyboard, Joysticks and AY sound chip)
	adds the incomplete machine to the machine_list[]
	the machine is not powered on.
	the machine is not suspended.
*/
Machine::Machine(gui::MachineController* parent, Model model, isa_id id) :
	IsaObject(parent, id, isa_Machine), _lock(),								  //_lock(PLock::recursive),
	controller(parent), model(model), model_info(&zx_info[model]), cpu_options(), // s.u.
	break_ptr(nullptr), audio_in_enabled(yes), // default. Child class and MachineController will override
	is_power_on(no), is_suspended(no),		   // must be initialized before memory
	rzx_file(nullptr), overlay_rzx_play(nullptr), overlay_rzx_record(nullptr),
	rom(this, "Internal Rom", model_info->rom_size), ram(this, "Internal Ram", model_info->ram_size), cpu(nullptr),
	ula(nullptr), mmu(nullptr), keyboard(nullptr), ay(nullptr), joystick(nullptr), taperecorder(nullptr), fdc(nullptr),
	printer(nullptr), crtc(nullptr), lastitem(nullptr), total_frames(0), // information: accumulated frames until now
	total_cc(0),	   // information: accumulated cpu T cycles until now
	total_buffers(0),  // information: accumulated dsp buffers until now
	total_realtime(0), // information: accumulated time[sec] until now
	cpu_clock(model_info->cpu_cycles_per_second),
	tcc0(0.0),	 // realtime t (offset to dsp buffer start) at frame start (cc=0)  --> powerOn()
	beam_cnt(0), // video beam indicator
	beam_cc(0)
{
	xlogIn("new Machine");

	// assert(cpu_clock*cpu_clock_predivider==ula_clock);

	cpu_options = model_info->has_zx80_bus ? cpu_crtc_zx81 : cpu_crtc;
	if (model_info->tape_load_routine) cpu_options |= cpu_patch;

	// Rom:
	load_rom();

	// flag Contended Ram:
	init_contended_ram();

	// flag Videoram:
	//	done by ULA

	// Items:
	//	items are added by subclass c'tor

	// add machine to machine_list[]:
	machine_list.append(this);
}

void Machine::init_contended_ram()
{
	assert(model_info->hasWaitmap() == (model_info->contended_rampages != 0));

	if (!model_info->hasWaitmap()) return;

	// set flag in cpu_options:
	cpu_options |= model_info->has_port_1ffd // TODO: new row in table
					   ?
					   cpu_waitmap // no wait cycles in JR_dis etc.
					   :
					   cpu_waitmap | cpu_ula_sinclair;

	// set flags in Contended Ram cells:
	uint32 bits = model_info->contended_rampages;
	for (uint32 j, e = 0; bits && e < ram.count(); bits <<= 1)
	{
		j = e;
		e += model_info->page_size;
		if (int32(bits) < 0)
		{
			xlogline("contended ram: 0x%05X, size: 0x%04X", j, e - j);
			while (j < e) { ram[j++] |= cpu_waitmap; }
		}
	}
}

void Machine::load_rom()
{
	try
	{
		FD fd(catstr(appl_rsrc_path, "Roms/", model_info->rom_filename)); // throws
		read_mem(fd, rom.getData(), rom.count());						  // throws
	}
	catch (AnyError& e)
	{
		logline("load rom failed: %s", e.what());
		rom[0] = HALT;
	}
}

Machine::~Machine()
{
	xlogIn("~Machine");

	if (this == front_machine) front_machine = nullptr;
	machine_list.remove(this);

	is_power_on = no;
	while (lastitem) delete lastitem; // rückwärts abbauen. QObject d'tor hat unbestimmte Reihenfolge!
}

/*	callback from Memory c'tor
 */
void Machine::memoryAdded(Memory* m)
{
	assert(isMainThread());
	assert(is_locked());

	memory.append(m);
	memoryModified(m, 0);
}

/*	callback from Memory d'tor
 */
void Machine::memoryRemoved(Memory* m)
{
	assert(isMainThread());
	assert(is_locked());

	// memory.removeitem(m);
	for (uint i = 0; i < memory.count(); i++)
	{
		if (memory[i] == m)
		{
			memory.remove(i);
			break;
		}
	}
	memoryModified(m, 1);
}

/*	callback from Memory shrink or grow:
 */
void Machine::memoryModified(Memory* m, uint how)
{
	assert(isMainThread());
	assert(is_locked());

	gui::MachineController* mc = NV(controller);
	mc->memoryModified(m, how);
}

/* callback from Item c'tor
 */
void Machine::itemAdded(Item* item)
{
	assert(isMainThread());
	assert(is_locked());

	if (item->isA(isa_Ay)) ay = AyPtr(item);
	if (item->isA(isa_Keyboard)) keyboard = KeyboardPtr(item);
	if (item->isA(isa_Joy)) joystick = JoyPtr(item);
	if (item->isA(isa_Mmu)) mmu = MmuPtr(item);
	if (item->isA(isa_Z80)) cpu = Z80Ptr(item);
	if (item->isA(isa_Fdc)) fdc = FdcPtr(item);
	if (item->isA(isa_Printer)) printer = PrinterPtr(item);
	if (item->isA(isa_Ula)) ula = UlaPtr(item);
	if (item->isA(isa_Crtc)) crtc = CrtcPtr(item);
	if (item->isA(isa_TapeRecorder)) taperecorder = TapeRecorderPtr(item);

	gui::MachineController* mc = NV(controller);
	mc->itemAdded(item);
}

/* callback from Item d'tor
 */
void Machine::itemRemoved(Item* item)
{
	assert(isMainThread());
	assert(is_locked());

	if (ay == item) ay = nullptr;
	if (keyboard == item) keyboard = nullptr;
	if (joystick == item) joystick = nullptr;
	if (mmu == item) mmu = nullptr;
	if (cpu == item) cpu = nullptr;
	if (fdc == item) fdc = nullptr;
	if (printer == item) printer = nullptr;
	if (ula == item) ula = nullptr;
	if (crtc == item) crtc = nullptr;
	if (taperecorder == item) taperecorder = nullptr;

	gui::MachineController* mc = NV(controller);
	mc->itemRemoved(item);
}

/*	resume machine from runForSound():
 */
void Machine::_resume()
{
	is_suspended = no;
	controller->machineRunStateChanged();
}

/*	resume machine:
 */
void Machine::resume() volatile
{
	PLocker z(_lock);
	assert(is_suspended);
	NV(this)->_resume();
}

/*	suspend machine from runForSound()
 */
void Machine::_suspend()
{
	is_suspended = true;
	controller->machineRunStateChanged();
}

/*	suspend machine
	returns flag, whether it was running, to be passed to resume(bool):
		1 = was running
		0 = was suspended
*/
bool Machine::suspend() volatile
{
	PLocker z(_lock);
	if (is_suspended) return no; // was not running
	NV(this)->_suspend();
	return yes; // was running
}


// 9 x virtual:
void Machine::loadO80(FD&) { showAlert("'.o' and '.80' files can only be loaded into a ZX80"); }
void Machine::saveO80(FD&) { showAlert("'.o' and '.80' files can only be saved from a ZX80"); }
void Machine::loadP81(FD&, bool) { showAlert("'.p' and '.81' files can only be loaded into a ZX81"); }
void Machine::saveP81(FD&, bool) { showAlert("'.p' and '.81' files can only be saved from a ZX81"); }
void Machine::loadSna(FD&) { showAlert("'.sna' files can only be loaded into a 48k Specci"); }
void Machine::saveSna(FD&) { showAlert("'.sna' files can only be saved from a 48k Specci"); }
void Machine::loadAce(FD&) { showAlert("'.ace' files can only be loaded into a Jupiter ACE"); }
void Machine::saveAce(FD&) { showAlert("'.ace' files can only be saved from a Jupiter ACE"); }
void Machine::loadScr(FD&) { showAlert("'.scr' files can only be loaded into a ZX Spectrum"); }
void Machine::saveScr(FD&) { showAlert("'.scr' files can only be saved from a ZX Spectrum"); }
// void Machine::loadTap(FD&)	{ showAlert("'.tap' files can only be loaded into a ZX Spectrum or Jupiter Ace");
// }


/*  save ROM:
	simply dump the rom to a file
*/
void Machine::saveRom(FD& fd)
{
	assert(isMainThread() || is_locked());

	write_mem(fd, rom.getData(), rom.count());
}

/*	load ROM:
	simply read the rom from a file.
	there must be at least rom.count() bytes available
	called from MachineController.loadSnapshot()
*/
void Machine::loadRom(FD& fd)
{
	assert(is_locked());

	read_mem(fd, rom.getData(), rom.count());

	// cpu patches entfernen:
	// TODO: rom prüfen, ob patches erhalten bleiben können
	for (uint i = 0; i < rom.count(); i++) rom[i] &= uint32(~cpu_patch);
}

void Machine::saveAs(cstr filepath)
{
	assert(is_locked());

	FD	 fd(filepath, 'w');
	cstr ext = lowerstr(extension_from_path(filepath));

	if (eq(ext, ".rom"))
	{
		saveRom(fd);
		return;
	}
	if (eq(ext, ".scr"))
	{
		saveScr(fd);
		return;
	}
	if (eq(ext, ".sna"))
	{
		saveSna(fd);
		return;
	} // MachineZxsp.cpp
	if (eq(ext, ".ace"))
	{
		saveAce(fd);
		return;
	} // MachineJupiter.cpp
	if (eq(ext, ".z80"))
	{
		saveZ80(fd);
		return;
	}
	if (eq(ext, ".o"))
	{
		saveO80(fd);
		return;
	} // MachineZx80.cpp: ZX80 must be waiting at the command prompt!
	if (eq(ext, ".80"))
	{
		saveO80(fd);
		return;
	} // MachineZx80.cpp: ZX80 must be waiting at the command prompt!
	if (eq(ext, ".p"))
	{
		saveP81(fd, 0);
		return;
	} // MachineZx81.cpp: ZX81 must be waiting at the command prompt!
	if (eq(ext, ".81"))
	{
		saveP81(fd, 0);
		return;
	} // MachineZx81.cpp: ZX81 must be waiting at the command prompt!
	if (eq(ext, ".p81"))
	{
		saveP81(fd, 1);
		return;
	} // MachineZx81.cpp: ZX81 must be waiting at the command prompt!

	if (eq(ext, ".rzx"))
	{
		if (rzx_file)
			rzx_file->writeFile(filepath);
		else
			showAlert("No rzx file in place.");
		return;
	}

	showAlert("Unsupported file format");
}


/*  Reset the Machine via Power On-Off-On
	start_cc = cpu cycle at which the power-on reset releases
	some machines won't start with an arbitrary start_cc:
	a starting value of 1000 cc seems to work for all machines
	note: start_cc for +2A:  ok: 0k..2k, 7k..14k;  boot error: 3k..6k, 15k..40k
*/
void Machine::_power_on(int32 start_cc)
{
	xlogIn("Machine:PowerOn");

	total_frames   = 0; // information: accumulated frames until now
	total_cc	   = 0; // information: accumulated cpu T cycles until now
	total_buffers  = 0; // information: accumulated dsp buffers until now
	total_realtime = 0; // information: accumulated time[sec] until now
						//	total_vtime		= 0;			// information: accumulated time[sec] until now

	tcc0 = -start_cc / cpu_clock;
	assert(t_for_cc(start_cc) == 0.0);

	crtc = CrtcPtr(findIsaItem(isa_Crtc));
	assert(crtc);
	cpu->setCrtc(crtc);

	// init all items:
	// note: guaranteed sequence: 1:cpu, 2:ula, 3:mmu, 3++:others
	for (Item* p = cpu; p; p = p->next())
	{
		xlogIn("init %s", p->name);
		p->powerOn(/*t=0,*/ start_cc);
	}

	is_power_on = true;
}

void Machine::powerOn(int32 start_cc) volatile
{
	// IF the machine is power_off, then we don't need to lock:
	assert(!is_power_on);
	NV(this)->rzxDispose();
	NV(this)->_power_on(start_cc);
}

void Machine::_power_off()
{
	xlogIn("Machine:PowerOff");
	is_power_on = false;
}

bool Machine::powerOff() volatile
{
	if (isPowerOff()) return false; // wasn't running
	// power off and lock machine to wait for runForSound():
	NVPtr<Machine>(this)->_power_off();
	return true; // was running
}

/*	power-cycle the machine for reset
	does not change the suspend state
*/
void Machine::powerCycle() volatile
{
	powerOff();
	powerOn();
}


/*	press the reset button
	reset all items:
	sequence: cpu .. lastitem
	timing is as for input / output:
		Time may overshoot dsp buffer but is limited to fit in dsp buffer + stitching
		(this is only a concern if the machine is running veerryy slowwllyy)
		cc may overshoot cc_per_frame a few cycles
*/
void Machine::reset()
{
	xlogIn("Machine: reset");
	assert(is_locked());

	rzxDispose();

	int32 cc = current_cc(); // current cc
	Time  t	 = now_lim();	 // current time, but limited to dsp buffer + stitching
	assert(t >= 0.0);

	for (Item* p = cpu; p; p = p->next()) { p->reset(t, cc); }
}


void Machine::nmi()
{
	xlogIn("Machine:Nmi");
	lastitem->triggerNmi();
	if (rzxIsRecording()) rzx_store_snapshot();
}


Item* Machine::addExternalItem(isa_id id)
{
	assert(isMainThread());
	assert(is_locked()); // note: complex items / with memory must actually be PowerOff
	assert(cpu);

	Item* i = findItem(id);
	if (i) return i;

	switch (id)
	{
	case isa_KempstonJoy: i = new KempstonJoy(this); break;
	case isa_ZxIf1: i = new ZxIf1(this); break;
	case isa_ZxIf2: i = new ZxIf2(this); break;
	case isa_ZxPrinter: i = new ZxPrinter(this); break;
	case isa_KempstonMouse: i = new KempstonMouse(this); break;
	case isa_ZonxBox: i = new ZonxBox(this); break;
	case isa_ZonxBox81: i = new ZonxBox81(this); break;
	case isa_DidaktikMelodik: i = new DidaktikMelodik(this); break;
	case isa_FdcBeta128: i = new FdcBeta128(this); break;
	case isa_FdcD80: i = new FdcD80(this); break;
	case isa_FdcJLO: i = new FdcJLO(this); break;
	case isa_FdcPlusD: i = new FdcPlusD(this); break;
	case isa_CursorJoy: i = new CursorJoy(this); break;
	case isa_DktronicsDualJoy: i = new DktronicsDualJoy(this); break;
	case isa_ProtekJoy: i = new ProtekJoy(this); break;
	case isa_PrinterAerco: i = new PrinterAerco(this); break;
	case isa_PrinterLprint3: i = new PrinterLprint3(this); break;
	case isa_PrinterTs2040: i = new PrinterTs2040(this); break;
	case isa_FullerBox: i = new FullerBox(this); break;
	case isa_GrafPad: i = new GrafPad(this); break;
	case isa_IcTester: i = new IcTester(this); break;
	case isa_Multiface1: i = new Multiface1(this); break;
	case isa_Multiface128: i = new Multiface128(this); break;
	case isa_Multiface3: i = new Multiface3(this); break;
	case isa_SpectraVideo: i = new SpectraVideo(this); break;
	case isa_CurrahMicroSpeech: i = new CurrahMicroSpeech(this); break;

	case isa_DivIDE:
	case isa_Cheetah32kRam:
	case isa_Jupiter16kRam:
	case isa_Zx16kRam:
	case isa_Memotech16kRam:
	case isa_Memotech64kRam:
	case isa_Stonechip16kRam:
	case isa_Zx3kRam:
	case isa_Ts1016Ram: IERR();

	default: IERR();
	}

	i->powerOn(cpu->cpuCycle());
	return i;
}

ExternalRam* Machine::addExternalRam(isa_id id, uint options)
{
	assert(isPowerOff());
	assert(findIsaItem(isa_ExternalRam) == nullptr);

	switch (id)
	{
	case isa_Jupiter16kRam: return new Jupiter16kRam(this);
	case isa_Zx16kRam: return new Zx16kRam(this);
	case isa_Stonechip16kRam: return new Stonechip16kRam(this);
	case isa_Ts1016Ram: return new Ts1016Ram(this);
	case isa_Memotech16kRam: return new Memotech16kRam(this);
	case isa_Cheetah32kRam: return new Cheetah32kRam(this);

	case isa_Zx3kRam:
		if (options != 1 kB && options != 2 kB) options = 3 kB; // safety
		return new Zx3kRam(this, options);

	case isa_Memotech64kRam:
		if (!Memotech64kRam::isValidDipSwitches(options)) options = Memotech64kRam::E;
		return new Memotech64kRam(this, options);

	default: IERR();
	}
}

DivIDE* Machine::addDivIDE(uint ramsize, cstr romfile)
{
	assert(isPowerOff());

	if (Item* item = findItem(isa_DivIDE))
		return DivIDEPtr(item);
	else
		return new DivIDE(this, ramsize, romfile);
}

/*	add or remove SpectraVideo interface
	Spectra is set as crtc in the CPU
*/
SpectraVideo* Machine::addSpectraVideo(bool f)
{
	assert(isMainThread());
	assert(is_locked());
	assert(isA(isa_MachineZxsp));

	if (f) // attach
	{
		crtc->attachToScreen(nullptr);
		crtc = CrtcPtr(addExternalItem(isa_SpectraVideo));
		crtc->attachToScreen(controller->getScreen());
		cpu->setCrtc(crtc);
		Z80::c2c(ula->getVideoRam(), crtc->getVideoRam(), 0x4000);
		crtc->setBorderColor(ula->getBorderColor());
		return SpectraVideoPtr(crtc);
	}
	else // remove
	{
		if (crtc != ula) removeItem(crtc);
		crtc = ula;
		crtc->attachToScreen(controller->getScreen());
		cpu->setCrtc(crtc);
		return nullptr;
	}
}


// #########################################################################
//						Run The Machine:
// #########################################################################


void Machine::installRomPatches(bool f)
{
	uint addr = model_info->tape_load_routine;
	if (addr)
	{
		assert(addr < rom.count());
		if (f)
			rom[addr] |= cpu_patch;
		else
			rom[addr] &= ~cpu_patch;
	}

	addr = model_info->tape_save_routine;
	if (addr)
	{
		assert(addr < rom.count());
		if (f)
			rom[addr] |= cpu_patch;
		else
			rom[addr] &= ~cpu_patch;
	}

	addr = model_info->tape_load_ret_addr;
	if (addr)
	{
		assert(addr < rom.count());
		if (f)
			rom[addr] |= cpu_patch;
		else
			rom[addr] &= ~cpu_patch;
	}
}

/*  handle rom patch
	called with all z80 registers stored
	return new opcode to execute, which may be the one passed in
*/
uint8 Machine::handleRomPatch(uint16 pc, uint8 opcode)
{
	opcode = lastitem->handleRomPatch(pc, opcode);

	CoreByte* instrptr = cpu->rdPtr(pc);

	if (instrptr == rom.getData() + model_info->tape_load_routine)
	{
		if (taperecorder->isLoaded() && taperecorder->isNotRecordDown())
		{
			if (taperecorder->instant_load_tape && handleLoadTapePatch()) return cpu->peek(cpu->getRegisters().pc);
			if (taperecorder->auto_start_stop_tape) taperecorder->autoStart(cpu->cpuCycle());
		}
	}

	else if (instrptr == rom.getData() + model_info->tape_save_routine)
	{
		if (taperecorder->isLoaded() && taperecorder->isRecordDown())
		{
			if (taperecorder->instant_load_tape && handleSaveTapePatch()) return cpu->peek(cpu->getRegisters().pc);
			if (taperecorder->auto_start_stop_tape) taperecorder->autoStart(cpu->cpuCycle());
		}
		if (ula->isA(isa_UlaZx80))
			UlaZx80Ptr(ula)->enableMicOut(1);
		else if (ula->isA(isa_UlaZx81))
			UlaZx81Ptr(ula)->enableMicOut(1);
	}

	else if (instrptr == rom.getData() + model_info->tape_load_ret_addr)
	{
		if (taperecorder->isLoaded() && taperecorder->isPlayDown())
		{
			if (taperecorder->auto_start_stop_tape) taperecorder->autoStop(cpu->cpuCycle());
		}
		if (ula->isA(isa_UlaZx80))
			UlaZx80Ptr(ula)->enableMicOut(0);
		else if (ula->isA(isa_UlaZx81))
			UlaZx81Ptr(ula)->enableMicOut(0);
	}

	return opcode; // maybe handled
}


/*	CPU callback: output instruction
	lastitem linked list:
	• item 1 = cpu:  no i/o
*/
void Machine::outputAtCycle(int32 cc, uint16 addr, uint8 byte)
{
	xxlogline("OUT $%04x,$%02x ", uint(addr), uint(byte));

	cc		 = ula->addWaitCycles(cc, addr);
	Time now = t_for_cc_lim(cc);

	for (Item* p = ula; p; p = p->next())
	{
		if (p->matchesOut(addr)) p->output(now, cc, addr, byte);
	};
}


/*	CPU callback: input instruction
	lastitem linked list:
	• item 1 = cpu:  no i/o
	• item 2 = ula: last item called => ula can add "idle bus bytes"
*/
uint8 Machine::inputAtCycle(int32 cc, uint16 addr)
{
	if (XXLOG && (cc > 2000 || (addr & 0xff) != 0xfe)) logline("IN $%04x ", uint(addr));

	cc		  = ula->addWaitCycles(cc, addr);
	Time  now = t_for_cc_lim(cc);
	uchar c	  = 0xff; // input byte, 0-bits override colliding 1-bits
	uchar m	  = 0x00; // bits actually set by items

	for (Item* p = ula; p; p = p->next())
	{
		if (p->matchesIn(addr)) p->input(now, cc, addr, c, m);
	};

	if (m != 0xff) c &= ula->getFloatingBusByte(cc);


	// handle rzx file playback / recording:
	if (rzx_file)
	{
		if (rzx_file->isPlaying())
		{
			int rval = rzx_file->getInput();
			if (rval >= 0)
				return rval;
			else
			{
				// -1 = EndOfFrame = OutOfSync.
				//		if the user wishes to resume playing at this position,
				//		the caller must startRecording() in the current frame and then
				//		record the actual input byte with storeInput(byte).

				rzxOutOfSync("Input beyond list: I'm sorry, but machine and file are out of sync.");

				// TODO: halt CPU

				// TODO: optionally switch to recording:
				//		 unexpected mode switch must be handled in runForSound() too!
			}
		}
		else if (rzx_file->isRecording()) { rzx_file->storeInput(c); }
	}

	return c;
}


// for memory mapped i/o
uint8 Machine::readMemMappedPort(int32 cc, uint16 addr, uint8 byte)
{
	return lastitem->readMemory(t_for_cc_lim(cc), cc, addr, byte);
}

// for memory mapped i/o
void Machine::writeMemMappedPort(int32 cc, uint16 addr, uint8 byte)
{
	lastitem->writeMemory(t_for_cc_lim(cc), cc, addr, byte);
}


/* ----	The Main Thing ----

	Run the machine until the end time of the dsp sample buffer is reached
	Unavoidable overshoot is allowed:
		DSP_SAMPLES_STITCHING provides the (fully handled) overshoot buffer
		DSP_BUFFER_GUARDZONE  provides an additional (discarded) overshoot zone for safety only!

		standard values: 4 samples @ 44.1kHz  <=>  90.7 µs  <=> 317 T @ 3.5 MHz  <=>  90 T @ 1 MHz  <=>  1 T @ 11 kHz

	real-world and virtual time are 'connected' at:
		real-world time:	this->sampletime_dsp_start
			this is the exact time when the cpu finished last runForSound, including any unavoidable overshoot.
			it is not the start time for the current dsp buffer (in which case it would always be 0.0)
			but is slightly ahead (-> overshoot).

		virtual time:		this->cc0
			taken from cpu->CpuCycle()
			this is the cpu cycle when the cpu finished last runForSound, including any unavoidable overshoot.
			it corresponds not with the start time for the current dsp buffer (in which case it would be fractional)
			but is slightly ahead (-> overshoot).
*/
void Machine::runForSound(int32 cc_final)
{
	xxlogIn("Machine:runForSound");
	assert(this->is_locked());

	TT; // Test Timer

	if (!cc_final) cc_final = cc_up_for_t(seconds_per_dsp_buffer());
	int	   result = 0;
	int32& cc	  = cpu->cpuCycleRef();

	if (rzx_file && rzx_file->isPlaying())
	{
		int32& ic	  = cpu->instrCountRef();
		int32  ic_end = rzx_file->getIcount();

		do {
			if (ula->isA(isa_UlaZxsp))
			{
				UlaZxsp* crtc_zxsp = UlaZxspPtr(ula);

				int32 cc_end = min(cc_final, crtc_zxsp->getWaitmapStart());
				if (cc < cc_end && ic < ic_end)
					result = cpu->run(cc_end, ic_end, cpu_options & ~(cpu_waitmap | cpu_crtc));

				cc_end = min(cc_final, crtc_zxsp->getWaitmapEnd());
				if (result == 0 && cc < cc_end && ic < ic_end) result = cpu->run(cc_end, ic_end, cpu_options);

				if (result == 0 && cc < cc_final && ic < ic_end)
					result = cpu->run(cc_final, ic_end, cpu_options & ~(cpu_waitmap | cpu_crtc));
			}
			else // TODO: video handling for other models
			{
				if (cc < cc_final && ic < ic_end) result = cpu->run(cc_final, ic_end, cpu_options);
			}

			if (!rzx_file->isPlaying()) goto a; // OutOfSync

			if (ic >= ic_end) // next rzx frame:
			{
				xlogline("next frame");

				// test for overshoot:
				// this may happen if the encoder finishes a frame right after an EI opcode:
				if (ic > ic_end)
				{
					xlogline("rzx: ic = ic_end + %u", ic - ic_end);

					if (ic > ic_end + 2) // solle unmöglich sein, außer nach ill. multiple PFX_IX / PFX_IY
					{					 // TODO: IX CB xx
						rzxOutOfSync(usingstr("rzx: ic = ic_end + %u", ic - ic_end));
						_suspend();
						return;
					}

					// probable cause: recorder finished frame right after EI
					// wir haben die folgende Instruction bereits ausgeführt
					// Nach dieser Instruction muss der Interrupt akzeptiert werden:
					//		außer diese war DI
					//		außer diese endete nach der INT-Dauer
					// Dann ist der nächste Frame nur genau diese eine Instr lang!
					// Sonst müsste wieder ein voller Frame folgen.
					// Dadurch, dass ic>0 sollte im nächsten Frame die ic wieder stimmen
				}

				// get icount for next frame:
				int32 next_ic_end = rzx_file->nextFrame();

				if (next_ic_end < 0) // rval = -1 and state = Playing  -> EndOfFile
				{					 // rval = -1 and state = Snapshot -> call getSnapshot()
					if (rzx_file->isSnapshot())
					{
						rzx_load_snapshot(cc_final, ic_end); // 	Snapshot -> Playing | EndOfFile | OutOfSync
						if (rzx_file->isOutOfSync()) goto a;
						if (rzx_file->isPlaying()) continue;
					}

					if (controller->isRzxAutostartRecording()) { rzxStartRecording(); }
					else // no autoStartRecording => OutOfSync
					{
						rzxStopPlaying("RZX file playback ended.");
					}
					goto a;
				}

				// do ffb:

				assert(rzx_file->isPlaying());
				ic -= ic_end; // reset instr. count: ic = 0
				ic_end = next_ic_end;

				if (cc > 64)
				{
					if (cc < 60000) xlogline("WARNING: ffb at cc = %u", cc);

					int32 cc_per_frame = crtc->doFrameFlyback(cc); // finish drawing, get cc_per_frame
					// cc/frame varies with the rzx source
					// cc sollte jetzt etwas größer sein als cc_per_frame
					// legal expected values: 1 .. 48
					if (cc < cc_per_frame) // note/todo: my Z80 accepts INT at +0, should be from +1 only
					{
						if (cpu_clock == model_info->cpu_cycles_per_second)
							xlogline("rzx: cc = cc/frame - %u", cc_per_frame - cc);
						cc_per_frame = cc - 4;
					}
					else if (cc > cc_per_frame + 23)
					{
						if (cpu_clock == model_info->cpu_cycles_per_second)
							xlogline("rzx: cc = cc/frame + %u", cc - cc_per_frame);
						cc_per_frame = cc - 18;
					}

					ForAllItems(videoFrameEnd(cc_per_frame)); // announce cc shift
					cc_final -= cc_per_frame;				  // shift cc for lvars
					tcc0 += cc_per_frame / cpu_clock;		  // shift start of current frame
					total_frames += 1;						  // info
					total_cc += cc_per_frame;				  // info
															  // cc -= cc_per_frame;			// done by Item Z80
				}
				else { xlogline("late interrupt"); }

				if (ic == 0) cpu->setInterrupt(cc, cc + 3); // darf nur direkt nach rzx-frame-start feuern
			}
		}
		while (cc < cc_final && result == 0);
	}

	else // no rzx file attached or rzx.recording:
	{
	a:
		int32		cc_ffb	  = ula->cpuCycleOfFrameFlyback();
		const int32 unlimited = 1 << 30;

		do {
			if (ula->isA(isa_UlaZxsp))
			{
				UlaZxsp* crtc_zxsp = UlaZxspPtr(ula);
				int32	 cc_end	   = min(cc_final, crtc_zxsp->getWaitmapStart());
				if (cc < cc_end) result = cpu->run(cc_end, unlimited, cpu_options & ~(cpu_waitmap | cpu_crtc));

				cc_end = min(cc_final, crtc_zxsp->getWaitmapEnd());
				if (result == 0 && cc < cc_end) result = cpu->run(cc_end, unlimited, cpu_options);

				cc_end = min(cc_final, cc_ffb);
				if (result == 0 && cc < cc_end)
					result = cpu->run(cc_end, unlimited, cpu_options & ~(cpu_waitmap | cpu_crtc));
			}
			else // TODO: video handling for other models
			{
				int32 cc_end = min(cc_final, cc_ffb);
				if (cc < cc_end) result = cpu->run(cc_end, unlimited, cpu_options);
			}

			if (cc >= cc_ffb)
			{
				int32 cc_per_frame = crtc->doFrameFlyback(cc); // finish drawing, get cc_per_frame
				ForAllItems(videoFrameEnd(cc_per_frame));	   // announce cc shift
				cc_final -= cc_per_frame;					   // shift cc for lvars
				tcc0 += cc_per_frame / cpu_clock;			   // shift start of current frame
				total_frames += 1;							   // info
				total_cc += cc_per_frame;					   // info

				if (rzx_file && rzx_file->isRecording())
				{
					rzx_file->endFrame(cpu->instrCount());
					rzx_file->startFrame(cc);
				}
				cpu->setInstrCount(0); // muss auch ohne rzx alle ~30 Minuten resettet werden!
			}
		}
		while (cc < cc_final && result == 0);
	}

	switch (result)
	{
	case cpu_exit_sp: // exit forced by macro Z80_INFO_POP (stack breakpoint)
		_suspend();	  // used by step over, step out
		cpu_options &= ~cpu_break_sp;
		break;

	case cpu_exit_r: // exit forced by macro PEEK (options&cpu_break_r)
		_suspend();
		break_ptr = cpu->rdPtr(cpu->break_addr);
		showInfo("CPU stopped at 'read' breakpoint at $%04X", cpu->break_addr);
		break;

	case cpu_exit_w: // exit forced by macro POKE (options&cpu_break_w)
		_suspend();
		break_ptr = cpu->wrPtr(cpu->break_addr);
		showInfo("CPU stopped at 'write' breakpoint at $%04X", cpu->break_addr);
		break;

	case cpu_exit_x: // exit forced by macro GET_INSTR (options&cpu_break_x)
		_suspend();
		break_ptr = cpu->rdPtr(cpu->break_addr);
		showInfo("CPU stopped at 'exec' breakpoint at $%04X", cpu->break_addr);
		break;
	}

	// shift time: this does not change the current time:
	Time t = t_for_cc(cc) >= seconds_per_dsp_buffer() ? seconds_per_dsp_buffer() // completed
														:
														t_for_cc(cc); // breaked

	assert(this->is_locked());
	ForAllItems(audioBufferEnd(t)); // announce time shift, force audio output
	TTest(2e-3, "Machine.runForSound()");

	total_buffers += 1;
	total_realtime += t;
	tcc0 -= t;
}


/* Laufe für (mindestens) cc cpu cycles.
 */
void Machine::runCpuCycles(int32 cc)
{
	assert(isSuspended() || is_locked());

	if (cc > 0) runForSound(cpu->cpuCycle() + cc);
}


void Machine::clear_break_ptr()
{
	break_ptr =
		cpu_options & cpu_break_x & *cpu->rdPtr(cpu->getRegisters().pc) ? cpu->rdPtr(cpu->getRegisters().pc) : nullptr;
}


void Machine::stepOver()
{
	xlogIn("Machine:stepOver");
	assert(isSuspended());

	clear_break_ptr();

	uint16 pc = cpu->getRegisters().pc; // pc
	uint8  o  = cpu->peek(pc);			// Opcode
	uint8  f  = cpu->getRegisters().f;	// Flags

	switch (o)
	{
	case CALL_NZ:
		if (f & Z_FLAG)
			goto s;
		else
			goto c;
	case CALL_Z:
		if (f & Z_FLAG)
			goto c;
		else
			goto s;
	case CALL_NC:
		if (f & C_FLAG)
			goto s;
		else
			goto c;
	case CALL_C:
		if (f & C_FLAG)
			goto c;
		else
			goto s;
	case CALL_PO:
		if (f & P_FLAG)
			goto s;
		else
			goto c;
	case CALL_PE:
		if (f & P_FLAG)
			goto c;
		else
			goto s;
	case CALL_P:
		if (f & S_FLAG)
			goto s;
		else
			goto c;
	case CALL_M:
		if (f & S_FLAG)
			goto c;
		else
			goto s;

	case RST00:
	case RST08:
	case RST10:
	case RST18:
	case RST20:
	case RST28:
	case RST30:
	case RST38:
	case CALL:
	c:
		cpu->setStackBreakpoint(cpu->getRegisters().sp);
		cpu_options |= cpu_break_sp;
		resume();
		break;

	case HALT:
		// Bis Interrupt abarbeiten, halt bei Interrupt auch wenn Interrupts gesperrt
		// Halt wenn Breakpoint aktiviert wird ((kann nur ein cpu_break_x genau hier auf pc sein))
		if (cpu->cpuCycle() >= ula->cpuCycleOfIrptEnd()) runCpuCycles(ula->cpuCycleOfFrameFlyback() - cpu->cpuCycle());
		if (break_ptr == nullptr) runCpuCycles(ula->cpuCycleOfInterrupt() - cpu->cpuCycle());
		if (break_ptr == nullptr)
			goto s;
		else
			break;

	case PFX_ED:
		// Blockbefehle komplett abarbeiten:
		// Halt bei Interrupt, wenn der Befehl überschrieben oder ausgeblendet wird
		// Halt wenn Breakpoint aktiviert wird
		o = cpu->peek(cpu->getRegisters().pc + 1); // ED Opcode
		//	if( (o&~0x0b) != LDIR ) goto s;					// kein Blockbefehl  ((Test nicht nötig))
		do runCpuCycles(1);
		while (break_ptr == nullptr && cpu->getRegisters().pc == pc && cpu->peek(pc) == PFX_ED &&
			   cpu->peek(pc + 1) == o);
		break;

	default:
	s:
		runCpuCycles(1);
		break;
	}
}


void Machine::stepIn()
{
	xlogIn("Machine:stepIn");
	assert(isSuspended());

	/*	Bei "Step In" wird immer exakt ein Befehl ausgeführt
		(oder ausnahmsweise Interrupt/NMI-Behandlung gestartet)

		Dazu wird runForSound so aufgerufen, dass runForSound nur einen Befehl lang läuft.

		Die Maschine wird in den Audio-Out-Puffer schreiben. Da der DSP-Interrupt aber auf
		höherer Priorität läuft, wird er unsere Maschine unterbrechen, den Audio-Out-Puffer
		wie immer löschen, die restlichen Maschinen und den Kassettenrekorder abarbeiten
		und den Puffer in den OS-Puffer kopieren. Von unserem Sound bleibt da nichts übrig. Hoffentlich.

		Das ist insbes. wichtig, weil bei extrem niedergetakteter Maschine für einen Schritt evtl.
		erst mehrmals Run() aufgerufen werden muss.

		Der Single-Stepper funktioniert logischerweise nur, wenn die Cpu angehalten ist.
	*/
	clear_break_ptr();
	runCpuCycles(1);
}

void Machine::stepOut()
{
	xlogIn("Machine:stepOut");
	assert(isSuspended());

	clear_break_ptr();

	cpu->setStackBreakpoint(cpu->getRegisters().sp + 2);
	cpu_options |= cpu_break_sp;
	resume();
}

void Machine::set60HzNeu(bool is60hz)
{
	// set machine to 100% speed and select 50 or 60 Hz setup.
	// called from speed menu and Machine50x60Inspector

	assert(is_locked());

	setSpeedFromCpuClock(model_info->cpu_cycles_per_second);
	ula->set60Hz(is60hz);
}

void Machine::setSpeedFromCpuClock(Frequency new_cpu_clock)
{
	// accelerate or slow down virtual world:
	// set machine speed from cpu clock.
	// called from Z80Inspector

	assert(is_locked());

	limit(1.0, new_cpu_clock, 28.65e6);
	new_cpu_clock = round(new_cpu_clock);
	if (new_cpu_clock == cpu_clock) return; // no change

	int32 cc = cpu->cpuCycle();
	tcc0 += cc / cpu_clock - cc / new_cpu_clock;

	cpu_clock = new_cpu_clock;

	// bei extrem niedergetakteter Maschine würde runForSound einige Aufrufe lang nichts tun:
	while (now() >= seconds_per_dsp_buffer())
	{
		Time t = seconds_per_dsp_buffer();
		ForAllItems(audioBufferEnd(t)); // announce time shift, force audio output
		tcc0 -= t;
	}
}

void Machine::speedupTo60fps()
{
	/*	accelerate virtual world:
		set machine speed to real-world framerate 60Hz
		called from speed menu for 50Hz models to run at ~120% speed  */

	assert(is_locked());
	assert(model_info->frames_per_second < 55.0 && !model_info->has_50_60hz_switch);
	// assert(ula->is50Hz());

	Frequency fps  = model_info->frames_per_second;
	Frequency ccps = model_info->cpu_cycles_per_second;
	setSpeedFromCpuClock(ccps * 60.0 / fps);

	int lines_after = model_info->lines_after_screen;
	ula->setLinesAfterScreen(lines_after);
}

void Machine::setSpeedAnd60fps(double factor)
{
	/* accelerate virtual world
	   but adjust real-world framerate to 60Hz
	   called from speed menu  */

	assert(is_locked());

	Frequency ccps = round(model_info->cpu_cycles_per_second * factor);
	setSpeedFromCpuClock(ccps);

	uint ccpl	   = model_info->cpu_cycles_per_line;
	int	 scanlines = int(round(ccps / ccpl / 60.0));

	int lines_before	= ula->getLinesBeforeScreen();
	int lines_in_screen = ula->getLinesInScreen();
	int lines_after		= scanlines - lines_before - lines_in_screen;
	ula->setLinesAfterScreen(lines_after);
}


void Machine::drawVideoBeamIndicator()
{
	/*	Wenn die Maschine angehalten ist oder zu stark gedrosselt ist,
		wird statt bzw. zusätzlich zu runForSound() auch noch
		drawVideoBeamIndicator() aufgerufen, um eben diesen darzustellen  */

	// if(current_cc() == beam_cc) return;  	// nicht weiter gelaufen => keine Änderung
	if (++beam_cnt < 4)
		return; // nur jeden 4. Sound-Interrupt ~ ca. 50 Mal / Sek.
				// (Hängt von AudioBufferSize und SampleFreq ab)
	beam_cnt = 0;
	beam_cc	 = current_cc();
	crtc->drawVideoBeamIndicator(beam_cc);
}


gui::OverlayJoystick* Machine::addOverlay(Joystick* joy, cstr idf, gui::Overlay::Position pos)
{
	gui::OverlayJoystick* o = new gui::OverlayJoystick(controller->getScreen(), joy, idf, pos);
	controller->getScreen()->addOverlay(o);
	return o;
}

void Machine::removeOverlay(gui::Overlay* o)
{
	if (o) controller->getScreen()->removeOverlay(o);
}


void Machine::hide_overlay_play()
{
	removeOverlay(overlay_rzx_play);
	overlay_rzx_play = nullptr;
}

void Machine::hide_overlay_record()
{
	removeOverlay(overlay_rzx_record);
	overlay_rzx_record = nullptr;
}

void Machine::show_overlay_play()
{
	hide_overlay_record();
	if (overlay_rzx_play) return;
	overlay_rzx_play = new gui::OverlayPlay(controller->getScreen());
	controller->getScreen()->addOverlay(overlay_rzx_play);
}

void Machine::show_overlay_record()
{
	hide_overlay_play();
	if (overlay_rzx_record) return;
	overlay_rzx_record = new gui::OverlayRecord(controller->getScreen());
	controller->getScreen()->addOverlay(overlay_rzx_record);
}

/*	Snapshot -> Playing | EndOfFile | OutOfSync
	OutOfSync --> warning dialog displayed, controller informed
	EndOfFile --> caller must display message and MUST change rzx state to Recording or OutOfSync!
	Playing   --> no state change
*/
void Machine::rzx_load_snapshot(int32& cc_final, int32& ic_end)
{
	cstr   filename = rzx_file->getSnapshot();
	cstr   ext		= lowerstr(extension_from_path(filename));
	int32& cc		= cpu->cpuCycleRef();
	int32& ic		= cpu->instrCountRef();
	int32  old_cc	= cc;

	if (eq(ext, ".z80"))
	{
		try
		{
			FD	  fd(filename);
			Model id = modelForZ80(fd);
			if (id == unknown_model) throw DataError("illegal model in file");
			if (model != id) throw DataError("snapshot requires different model.");

			loadZ80(fd);
			_resume();
		}
		catch (AnyError& e)
		{
			cc = old_cc;
			rzxOutOfSync(usingstr("RZX file: load .z80 snapshot: %s", e.what()));
			return;
		}

		cc_final += cc - old_cc;
		old_cc = cc;

		if (rzx_file->isEndOfFile()) return; // file ended with a snapshot

		assert(rzx_file->isPlaying());

		if (rzx_file->getStartCC())
		{
			int32 dcc = rzx_file->getStartCC() - cc;
			tcc0 -= dcc / cpu_clock;
			cc += dcc;
		}
		cc_final += cc - old_cc;
		old_cc = cc;

		ic_end = rzx_file->getIcount();
		ic	   = 0;
	}
	else
	{
		rzxOutOfSync(usingstr("RZX file: load %s snapshot: TODO", ext));
		return;
	}
}

/*	store snapshot at current position
	EndOfFile --> Recording | OutOfSync
*/
void Machine::rzx_store_snapshot()
{
	assert(rzx_file);
	assert(rzx_file->isEndOfFile());

	try
	{
		cstr snafilepath = catstr("/tmp/zxsp/", tostr(now()), ".z80");
		saveAs(snafilepath);
		rzx_file->storeSnapshot(snafilepath);
		cpu->setInstrCount(0);
		rzx_file->startBlock(cpu->cpuCycle());
	}
	catch (AnyError& e)
	{
		rzxOutOfSync(catstr("recording rzx: store snapshot: ", e.what()), yes);
	}
}

void Machine::rzxOutOfSync(cstr msg, bool red)
{
	assert(rzx_file);
	if (msg) (red ? showAlert : showWarning)(msg);

	rzx_file->setOutOfSync();
	hide_overlay_play();
	hide_overlay_record();
	controller->rzxStateChanged();
}

void Machine::rzxDispose()
{
	if (!rzx_file) return;
	delete rzx_file;
	rzx_file = nullptr;

	hide_overlay_play();
	hide_overlay_record();
	controller->rzxStateChanged();
}

/*	store the passed RzxFile and start playing
	eventually immediately switch to recording, if the file is at end
*/
void Machine::rzxPlayFile(RzxFile* rzx)
{
	if (rzxIsLoaded()) rzxDispose();
	rzx_file = rzx;

	rzx->rewind(); // --> Snapshot | EndOfFile | OutOfSync
	switch (rzx->state)
	{
	default: return rzxOutOfSync("Bummer: unexpected state after rewinding the rzx file", yes);
	case RzxFile::OutOfSync: return rzxOutOfSync("I'm sorry, but the rzx file is unusable.");
	case RzxFile::EndOfFile: return rzxStartRecording("The rzx file is empty. rzx recording started!");
	case RzxFile::Snapshot: break;
	}

	int32 cc_final, ic_end;				 // dummy
	rzx_load_snapshot(cc_final, ic_end); // --> Playing | EndOfFile | OutOfSync
	switch (rzx->state)
	{
	default: return rzxOutOfSync("Bummer: unexpected state after loading the starter snapshot", yes);
	case RzxFile::OutOfSync: return; // message already displayed
	case RzxFile::EndOfFile: return rzxStartRecording("The rzx file only contained a snapshot. rzx recording started!");
	case RzxFile::Playing:
		if (rzx->getStartCC())
		{
			int32& cc  = cpu->cpuCycleRef();
			int32  dcc = rzx->getStartCC() - cc;
			tcc0 -= dcc / cpu_clock;
			cc += dcc;
		}
		show_overlay_play();
		controller->rzxStateChanged();
		return;
	}
}

/*	start recording
	!file | Playing | Recording | EndOfFile --> Recording
*/
void Machine::rzxStartRecording(cstr msg, bool yellow)
{
	if (msg) (yellow ? showWarning : showInfo)(msg);

	if (!rzx_file) rzx_file = new RzxFile(); // --> EndOfFile

	switch (rzx_file->state)
	{
	default: return rzxOutOfSync(usingstr("Start RZX recording: unexpected file state %i", rzx_file->state), yes);

	case RzxFile::Playing: rzx_file->startRecording(); break;

	case RzxFile::Recording: return;

	case RzxFile::EndOfFile:
		rzx_store_snapshot();
		if (rzx_file->isOutOfSync()) return;
		break;
	}

	show_overlay_record();
	controller->rzxStateChanged();
}

void Machine::rzxStopRecording(cstr msg, bool yellow)
{
	assert(rzx_file);

	if (msg) (yellow ? showWarning : showInfo)(msg);
	rzxOutOfSync(nullptr);
}
