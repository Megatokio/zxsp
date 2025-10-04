// Copyright (c) 2004 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/*	Macro M_ISA(A,B,C):

	A = isa_ID of the IsaObject subclass
	B = isa_ID of the nominal parent class.
		normally this is the actual parent class,
		but in rare cases it may be the grandparent class.
	C = Objects name. Some objects get their actual name from other sources;
		e.g. Machines get their name from model_info[].name.
		(many Machine classes are used for multiple models.)
*/

#if !defined(ISA_ID_H) || defined(M_ISA)
  #define ISA_ID_H

  #ifndef M_ISA
enum isa_id {
	#define M_ISA(A, B, C) A
	#define M_ISA_A
  #endif

	// clang-format off

	M_ISA(	isa_none=0,					isa_none,			"None" ),				// free / unset
	M_ISA(	isa_unknown,				isa_none,			"Unknown" ),			// failure / error

	M_ISA(	isa_TapeFile,				isa_none,			"TapeFile" ),			// note: not an Item
	M_ISA(	isa_CswBuffer,				isa_none,			"CswBuffer" ),			// note: not an Item
	M_ISA(	isa_TapeData,				isa_none,			"TapeData" ),			// note: not an Item
	M_ISA(		isa_TapData,			isa_TapeData,		"TapData" ),
	M_ISA(		isa_TzxData,			isa_TapeData,		"TzxData" ),
	M_ISA(		isa_RlesData,			isa_TapeData,		"RlesData" ),			// 1-bit audio data, run length encoded
	M_ISA(		isa_AudioData,			isa_TapeData,		"AudioData" ),			// Hifi mono/stereo audio
	M_ISA(		isa_O80Data,			isa_TapeData,		"O80Data" ),			// .80 / .o / .81 / .p81 / .p

	M_ISA(	isa_Overlay,				isa_none,			"Overlay" ),			// note: not a IsaObject
	M_ISA(		isa_RzxOverlay,			isa_Overlay,		"RzxOverlay"),
	M_ISA(		isa_JoystickOverlay,	isa_Overlay,		"JoystickOverlay" ),
	M_ISA(		isa_OverlayTimeline,	isa_Overlay,		"Timeline Overlay" ),
	M_ISA(		isa_OverlaySingleStep,	isa_Overlay,		"SingleStep Overlay" ),

	M_ISA(	isa_Screen,					isa_none,			"OpenGL Screen" ),		// note: not a IsaObject
	M_ISA(		isa_ScreenZxsp,			isa_Screen,			"ScreenZxsp" ),
	M_ISA(			isa_ScreenTc2048,	isa_ScreenZxsp,		"ScreenTc2048" ),
	M_ISA(			isa_ScreenSpectra,	isa_ScreenZxsp,		"ScreenSpectra" ),
	M_ISA(		isa_ScreenMono,			isa_Screen,			"ScreenMono" ),

	M_ISA(	isa_Renderer,				isa_none,			"Screen Renderer" ),
	M_ISA(		isa_ZxspRenderer,		isa_Renderer,		"Zxsp Screen Renderer" ),
	M_ISA(			isa_Tc2048Renderer,	isa_ZxspRenderer,	"Tc2048 Screen Renderer" ),
	M_ISA(			isa_SpectraRenderer,isa_ZxspRenderer,	"SPECTRA Screen Renderer" ),
	M_ISA(		isa_MonoRenderer,		isa_Renderer,		"Monochrome Screen Renderer" ),

	M_ISA(	isa_GifWriter,				isa_none,			"Gif File Writer" ),
	M_ISA(		isa_ZxspGifWriter,		isa_GifWriter,		"Zxsp Gif Writer" ),
	M_ISA(			isa_Tc2048GifWriter,isa_ZxspGifWriter,	"Tc2048 Gif Writer" ),
	M_ISA(			isa_SpectraGifWriter,isa_ZxspGifWriter,	"SPECTRA Gif Writer" ),
	M_ISA(		isa_MonoGifWriter,		isa_GifWriter,		"Monochrome Gif Writer" ),

	M_ISA(	isa_Joystick,				isa_none,			"Realworld Joystick" ),	// note: a physical joystick interface
	M_ISA(		isa_UsbJoystick,		isa_Joystick,		"USB Joystick" ),
	M_ISA(		isa_KbdJoystick,		isa_Joystick,		"Keyboard Emulation Joystick" ),

	M_ISA(	isa_Machine,				isa_none,			"Machine" ),
	M_ISA(		isa_MachineJupiter,		isa_Machine,		"Jupiter ACE" ),
	M_ISA(		isa_MachineZx80,		isa_Machine,		"Sinclair ZX80" ),
	M_ISA(		isa_MachineZx81,		isa_Machine,		"Sinclair ZX81" ),
	M_ISA(			isa_MachineTk85,	isa_MachineZx81,	"Microdigital TK85 (Brazil)" ),
	M_ISA(			isa_MachineTs1000,	isa_MachineZx81,	"Timex Sinclair 1000 (USA)" ),
	M_ISA(			isa_MachineTs1500,	isa_MachineZx81,	"Timex Sinclair 1500 (USA)" ),
	M_ISA(		isa_MachineZxsp,		isa_Machine,		"ZX Spectrum" ),
	M_ISA(			isa_MachineTk90x,	isa_MachineZxsp,	"Microdigital TK90X (Brazil)" ),
	M_ISA(			isa_MachineTk95,	isa_MachineZxsp,	"Microdigital TK95 (Brazil)" ),
	M_ISA(			isa_MachineInves,	isa_MachineZxsp,	"Inves Spectrum+ (Spain)" ),
	M_ISA(			isa_MachineTc2048,	isa_MachineZxsp,	"Timex Computer 2048 (Portugal)" ),
	M_ISA(			  isa_MachineTc2068,  isa_MachineTc2048,"Timex Computer 2068 (Portugal)" ),
	M_ISA(				isa_MachineTs2068,isa_MachineTc2068,"Timex Sinclair 2068 (USA)" ),
	M_ISA(				isa_MachineUnipol,isa_MachineTc2068,"Unipolbrit Komputer 2086 (Poland)" ),
	M_ISA(			isa_MachineZx128,	isa_MachineZxsp,	 "Sinclair ZX Spectrum+ 128K" ),
	M_ISA(				isa_MachinePentagon128,	isa_MachineZx128,	"Pentagon 128k" ),
	M_ISA(				isa_MachineZxPlus2,		isa_MachineZx128,	"ZX Spectrum +2" ),
	M_ISA(				isa_MachineZxPlus2a,	isa_MachineZx128,	"ZX Spectrum +2A" ),
	M_ISA(					isa_MachineZxPlus3,	isa_MachineZxPlus2a,"ZX Spectrum +3" ),

	M_ISA(	isa_MemHex,					isa_none,		"Memory Hex View" ),		/* virtual group id for tool windows */
	M_ISA(	isa_MemDisass,				isa_none,		"Memory Disassembler" ),	/* virtual group id for tool windows */
	M_ISA(	isa_MemGraphical,			isa_none,		"Memory Graphical View" ),	/* virtual group id for tool windows */
	M_ISA(	isa_MemAccess,				isa_none,		"Memory Access" ),			/* virtual group id for tool windows */

	M_ISA(	isa_Item,					isa_none,		"Item"),				// Items:
	M_ISA(		isa_Z80,				isa_Item,		"Z80 CPU" ),

	M_ISA(		isa_Keyboard,			isa_Item,		"Keyboard" ),
	M_ISA(			isa_KbdJupiter,		isa_Keyboard,	"Keyboard" ),
	M_ISA(			isa_KbdTk90x,		isa_Keyboard,	"Keyboard" ),
	M_ISA(			isa_KbdTk85,		isa_Keyboard,	"Keyboard" ),
	M_ISA(			isa_KbdTk95,		isa_Keyboard,	"Keyboard" ),
	M_ISA(			isa_KbdTs1000,		isa_Keyboard,	"Keyboard" ),
	M_ISA(			isa_KbdTs1500,		isa_Keyboard,	"Keyboard" ),
	M_ISA(			isa_KbdZx80,		isa_Keyboard,	"Keyboard" ),
	M_ISA(			isa_KbdZx81,		isa_Keyboard,	"Keyboard" ),
	M_ISA(			isa_KbdZxsp,		isa_Keyboard,	"Keyboard" ),
	M_ISA(			isa_KbdZxPlus,		isa_Keyboard,	"Keyboard" ),
	M_ISA(			isa_KbdTimex,		isa_Keyboard,	"Keyboard" ),

	M_ISA(		isa_Mmu,				isa_Item,		"Mmu" ),
	M_ISA(			isa_MmuZx80,		isa_Mmu,		"ZX80 Mmu" ),
	M_ISA(			  isa_MmuZx81,		isa_MmuZx80,	"ZX81 Mmu" ),
	M_ISA(				isa_MmuTs1500,	isa_MmuZx81,	"TS1500 Mmu" ),
	M_ISA(				isa_MmuTk85,	isa_MmuZx81,	"TK85 Mmu" ),
	M_ISA(			isa_MmuJupiter,		isa_Mmu,		"Jupiter Ace Mmu" ),
	M_ISA(			isa_MmuZxsp,		isa_Mmu,		"Zxsp 48k Mmu" ),
	M_ISA(			  isa_MmuTc2048,	isa_MmuZxsp,	"TC2048 Mmu" ),
	M_ISA(				isa_MmuTc2068,	isa_MmuTc2048,	"Timex Command Cartridge" ),
	M_ISA(				  isa_MmuTs2068,isa_MmuTc2068,	"Timex Command Cartridge" ),
	M_ISA(				  isa_MmuU2086,	isa_MmuTc2068,	"Kaseta Rom" ),
	M_ISA(			isa_Mmu128k,		isa_Mmu,		"Zxsp 128k Mmu" ),
	M_ISA(				isa_MmuPlus3,	isa_Mmu128k,	"Zxsp +2A/+3 Mmu" ),
	M_ISA(			isa_MmuInves,		isa_Mmu,		"Inves 48k Mmu" ),

	M_ISA(		isa_Crtc,				isa_Item,		"Ula" ),
	M_ISA(		  isa_Ula,				isa_Crtc,		"Crtc" ),
	M_ISA(			isa_UlaZxsp,		isa_Ula,		"ZX Spectrum Ula" ),
	M_ISA(				isa_Ula128k,	isa_UlaZxsp,	"ZX 128k Ula" ),
	M_ISA(				   isa_UlaPlus3,isa_Ula128k,	"ZX +2A/+3 Ula" ),
	M_ISA(				isa_UlaTc2048,	isa_UlaZxsp,	"TC2048 Ula" ),
	M_ISA(				  isa_UlaTc2068,isa_UlaTc2048,	"TC2068 Ula" ),
	M_ISA(				  isa_UlaTs2068,isa_UlaTc2048,	"TS2068 Ula" ),
	M_ISA(				  isa_UlaU2086, isa_UlaTc2048,	"Unipolbrit 2086 Ula" ),
	M_ISA(				isa_UlaInves,	isa_UlaZxsp,	"Inves 48k Ula" ),
	M_ISA(				isa_UlaTk90x,	isa_UlaZxsp,	"TK90X Ula" ),
	M_ISA(			isa_UlaZx80,		isa_Ula,		"ZX80 Ula" ),
	M_ISA(				isa_UlaZx81,	isa_UlaZx80,	"ZX81 Ula" ),
	M_ISA(			isa_UlaJupiter,		isa_Ula,		"Jupiter ACE Ula" ),
	M_ISA(		  isa_SpectraVideo,		isa_Crtc,		"SPECTRA Video Interface" ),

	M_ISA(		isa_Joy,				isa_Item,		"Joystick Interface" ),
	M_ISA(			isa_Tk85Joy,		isa_Joy,		"TK85 Joystick Port" ),
	M_ISA(			isa_KempstonJoy,	isa_Joy,		"Kempston Joystick Interface" ),
	M_ISA(				isa_Tc2048Joy,	isa_KempstonJoy,"Internal Joystick Port" ),
	M_ISA(				isa_InvesJoy,	isa_KempstonJoy,"Internal Joystick Port" ),
	M_ISA(			isa_Tc2068Joy,		isa_Joy,		"Internal Joystick Ports" ),
	M_ISA(				isa_Ts2068Joy,	isa_Tc2068Joy,	"Internal Joystick Ports" ),
	M_ISA(				isa_U2086Joy,	isa_Tc2068Joy,	"Internal Joystick Ports" ),
	M_ISA(			isa_SinclairJoy,	isa_Joy,		"Sinclair Joystick Interface" ),
	M_ISA(				isa_ZxPlus2Joy,	isa_SinclairJoy,"Internal Joystick Ports" ),
	M_ISA(				isa_ZxPlus2AJoy,isa_SinclairJoy,"Internal Joystick Ports" ),
	M_ISA(				isa_ZxPlus3Joy,	isa_SinclairJoy,"Internal Joystick Ports" ),
	M_ISA(				isa_ZxIf2,		isa_SinclairJoy,"Sinclair Interface 2" ),
	M_ISA(				isa_Tk90xJoy,	isa_SinclairJoy,"Internal Joystick Port" ),
	M_ISA(				isa_Tk95Joy,	isa_SinclairJoy,"Internal Joystick Port" ),
	M_ISA(			isa_CursorJoy,		isa_Joy,		"Cursor Joystick Interface" ),
	M_ISA(				isa_ProtekJoy,	isa_CursorJoy,	"Protek Joystick Interface" ),
	M_ISA(			isa_DktronicsDualJoy,isa_Joy,		"dk'tronics Dual Joystick Interface" ),

	M_ISA(		isa_MassStorage,		isa_Item,		"Mass Storage Extension" ),
	M_ISA(		   isa_DivIDE,			isa_MassStorage,"DivIDE Hard Disc Interface" ),
	M_ISA(		   isa_SmartSDCard,		isa_MassStorage,"SMART SD Card Interface" ),
	M_ISA(		   isa_Fdc,				isa_MassStorage,"Floppy Disc Controller" ),
	M_ISA(			  isa_Fdc765,		isa_Fdc,		"FDC µPD765A" ),
	M_ISA(				 isa_FdcPlus3,	isa_Fdc765,		"Internal Floppy Disc" ),
	M_ISA(			 isa_FdcBeta128,	isa_Fdc,		"Beta 128 Disc Interface" ),		// WD1793
	M_ISA(			 isa_FdcPlusD,		isa_Fdc,		"Plus D Disc Controller" ),			// WD1772
	M_ISA(			 isa_FdcD80,		isa_Fdc,		"D80 Disc Interface" ),
	M_ISA(			 isa_FdcJLO,		isa_Fdc,		"JLO Disc Interface" ),
	M_ISA(			 isa_OpusDiscovery,	isa_Fdc,		"Opus Discovery Disc Interface" ),	// WD1770
	M_ISA(			 isa_Disciple,		isa_Fdc,		"DISCiPLE Disc Interface" ),		// VL1772

	M_ISA(		isa_Ay,					isa_Item,		"AY-3-8912" ),
	M_ISA(			isa_ZonxBox,		isa_Ay,			"Bi-Pak ZON X" ),
	M_ISA(			isa_ZonxBox81,		isa_Ay,			"Bi-Pak ZON X-81" ),
	M_ISA(			isa_DidaktikMelodik,isa_Ay,			"Didaktik Melodik" ),
	M_ISA(			isa_FullerBox,		isa_Ay,			"Fuller Box" ),
	M_ISA(			isa_InternalAy,		isa_Ay,			"Internal AY-3-8912" ),
	M_ISA(			isa_ZaxonAyMagic,	isa_Ay,			"Zaxon AY Magic" ),

	M_ISA(		isa_Printer,			isa_Item,		"Printer Interface" ),
	M_ISA(			isa_ZxPrinter,		isa_Printer,	"Sinclair ZX Printer" ),
	M_ISA(			isa_PrinterAerco,	isa_Printer,	"Aerco Centronics Interface" ),
	M_ISA(			isa_PrinterLprint3, isa_Printer,	"ZX LPrint III Printer Interface" ),
	M_ISA(			isa_PrinterPlus3,	isa_Printer,	"Internal Printer Port" ),
	M_ISA(			isa_PrinterTs2040,	isa_Printer,	"Timex Sinclair TS2040 Printer" ),

	M_ISA(		isa_ExternalRam,		isa_Item ,		"External Ram Extension"),
	M_ISA(			isa_Cheetah32kRam,	isa_ExternalRam,"Cheetah 32K rampack" ),
	M_ISA(			isa_Jupiter16kRam,	isa_ExternalRam,"Jupiter 16K RAM" ),
	M_ISA(			isa_Zx16kRam,		isa_ExternalRam,"Sinclair ZX 16K RAM" ),
	M_ISA(			isa_Ts1016Ram,		isa_ExternalRam,"Timex Sinclair 1016" ),
	M_ISA(			isa_Stonechip16kRam,isa_ExternalRam,"Stonechip 16K Expandable Ram" ),
	M_ISA(			isa_Memotech16kRam, isa_ExternalRam,"MEMOPAK 16K" ),
	M_ISA(			isa_Memotech64kRam, isa_ExternalRam,"MEMOPAK 64k" ),
	M_ISA(			isa_Zx3kRam,		isa_ExternalRam,"Sinclair ZX80 1-3K BYTE RAM PACK" ),

	M_ISA(		isa_TapeRecorder,		isa_Item,		 "Tape Recorder" ),
	M_ISA(			isa_Walkman,		isa_TapeRecorder,"External Tape Recorder"),
	M_ISA(			isa_Plus2Tapedeck,	isa_TapeRecorder,"Internal Tape Recorder"),
	M_ISA(			isa_Plus2aTapedeck,	isa_TapeRecorder,"Internal Tape Recorder"),
	M_ISA(			isa_TS2020,			isa_TapeRecorder,"TS2020"),

	M_ISA(		isa_MGT,				isa_Item,		"M.G.T. Interface" ),
	M_ISA(		isa_Multiface,			isa_Item,		"Multiface Interface" ),
	M_ISA(			isa_Multiface1,		isa_Multiface,	"Multiface 1" ),
	M_ISA(			isa_Multiface128,	isa_Multiface,	"Multiface 128" ),
	M_ISA(			isa_Multiface3,		isa_Multiface,	"Multiface 3" ),

	M_ISA(		isa_ZxIf1,				isa_Item,		"Sinclair ZX Interface 1" ),
	M_ISA(		isa_WafaDrive,			isa_Item,		"Rotronics Wafadrive" ),
	M_ISA(		isa_IcTester,			isa_Item,		"Kio's IC Tester" ),
	M_ISA(		isa_Mouse,				isa_Item,		"Mouse Interface" ),
	M_ISA(			isa_KempstonMouse,	isa_Mouse,		"Kempston Mouse Interface" ),
	M_ISA(			isa_AmxMouse,		isa_Mouse,		"AMX Mouse Interface" ),
	M_ISA(		isa_GrafPad,			isa_Item,		"GrafPad" ),
	M_ISA(		isa_CurrahMicroSpeech,	isa_Item,		"Currah µSpeech" ),

// clang-format on

  #undef M_ISA

  #ifdef M_ISA_A
};
	#undef M_ISA_A
  #endif

#endif // include guard
