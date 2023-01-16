/*	Copyright  (c)	Günter Woigk 2008 - 2019
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


	ZX80 ULA emulation
	------------------

	Input address:	0xFE  -  only A0 decoded
	Output address:	0xFF  -  no address bit decoded - any OUT will do it! - and also IRPTACK!

	IN A,(FE)	aktiviert das SYNC-Signal (idR für VSYNC)
				aktiviert den MIC_OUT Ausgang
				resettet den Zeilenzähler LCNTR.
				liest Tastaturmatrix
				liest 50/60 Hz framerate jumper
				liest EAR_IN Eingang

	OUT (FF),A	deaktiviert das SYNC-Signal
				deaktiviert den MIC_OUT Ausgang

	IRPTACK		aktiviert das SYNC-Signal bis zum übernächsten M1-Zyklus ( ~24 CPU-Takte ~ 5 µsec )
				MIC_OUT-Ausgang entsprechend


	Lesen eines Befehls im Bereich 0x8000 .. 0xFFFF	ergibt NOP wenn im Befehlscode D6=0
	Gleichzeitig wird der Code in der ULA gespeichert
	Wird der Code ausgeführt (D6=1) wird vermutlich das Charactercoderegister in der ULA nicht gelesen sondern gelöscht. ((TODO))

	Im Refreshzyklus gibt die CPU das I-Register auf A9-A15 aus
		und die ULA die 6 Datenbits des Zeichencodes auf A3-A8
		und den internen Zeilenzähler LCNTR auf A0-A2
	Das so adressierte Byte (im Characterset des ROMs) wird in den Videoshifter geladen
		und in den nächsten 8 XTAL-Takten (4 CPU Takte) ausgegeben.
		War Bit 7 des Bytes aus dem VRAM gesetzt, werden die Pixel invertiert.

	Das letzte Zeichen einer Zeile im VRAM ist normalerweise der HALT Opcode,
		so dass die CPU hier 'stehen bleibt' und nur noch weiße Pixelbytes in das Shiftregister geladen werden.

	Sobald das R-Register 0x80 oder 0x00 erreicht - A6=0 - wird ein INT erzeugt
		und die CPU führt keine weiteren HALTs mehr aus sondern nimmt die Interruptbehandlung
		außerhalb des Bereiches 0x8000 - 0xFFFF auf.  ((außer der Interrupt ist gesperrt...))

	Interrupts werden auch immer erzeugt, wenn im R-Register A6=0 ist.
	Deshalb ist es außerhalb der Bildschirmroutine vermutlich nie sinnvoll, den Interrupt zuzulassen.

----------------------------------

	Da das Videoram nicht durch die HW fest verdrahtet ausgelesen wird, sondern INT und VRAM read durch die CPU erfolgt,
	haben wir effektiv kein stabiles Videobild.

	inputs:		Input()		-> Sync ON
				Output()	-> Sync OFF
				IrptAck()	-> Sync ON/OFF
				UlaRead()	-> read pixel byte
				DoFrameFlyback() -> setup frame data for oglRenderer

	outputs:	GetCpuCyclesPerFrame()	-> wann DoFrameFlyback() aufgerufen wird
				frame_w, frame_h	-> oglRenderer
				screen_w, screen_h	-> oglRenderer
				screen_x0,screen_y0	-> oglRenderer
				frame_data			-> oglRenderer (swapped with old buffer)

	Es wird versucht, ein TV Set nachzubilden:

				vsync_cc_min, vsync_cc_max:		Fangbereich für VSYNC
				hsync_cc_min, hsync_cc_max:		Fangbereich für HSYNC
				während SYNC=ON werden schwarze Pixel gespeichert
				während SYNC_OFF weiße Pixel bzw. Videopixel, wenn UlaRead() aufgerufen wird

*/

#include "UlaZx80.h"
#include "Machine.h"
#include "Z80/Z80.h"
#include "Qt/Screen/ScreenMono.h"
#include "Dsp.h"
#include "ZxInfo.h"
#include "Keyboard.h"
#include "info.h"
#include "TapeRecorder.h"



// MIC_OUT ist das SYNC-Signal


#define o_addr  	"----.----.----.----"		// übliche Adresse: $FF
#define i_addr		"----.----.----.---0"		// übliche Adresse: $FE
#define FRAMERATE_BIT	6						// 50/60 Hz Wahlschalter (Lötbrücke)
#define EAR_IN_BIT		7
#define EAR_IN_MASK		(1<<7)
// bits 0-4: 5 keys from keyboard (low=pressed), row selected by A8..A15
// bit  6:   50/60 Hz framerate jumper


// colored pixel octets:
#define black_pixels	0x00
#define white_pixels	0xFF


UlaZx80::~UlaZx80()
{
}


UlaZx80::UlaZx80 (Machine* m)
:
	UlaMono(m,isa_UlaZx80,o_addr,i_addr)
{}


void UlaZx80::powerOn(int32 cc)
{
	xlogIn("UlaZx80:init");
	UlaMono::powerOn(cc);

	frame_w		= bytes_per_row;	// frame width [bytes] == address offset per scan line
	screen_w	= 32;				// screen width [bytes]
	screen_x0	= 10;				// hor. screen offset inside frame scan lines [bytes]

	assert( bytes_per_row_max   <= UlaMono::max_bytes_per_row );
	assert( bytes_per_frame_max <= UlaMono::max_bytes_per_frame );

	reset();
}


void UlaZx80::reset ( Time t, int32 cc )
{
	xlogIn("UlaZx80::reset");
	UlaMono::reset(t,cc);
	reset();
}

void UlaZx80::reset()
{
//	mic_out_enabled		= no;
	beeper_volume		= 0.0025;
	beeper_current_sample = -beeper_volume;

	//cc_frame_start = 0;	// cc of current frame start
	cc_frame_end	 = 0;	// cc of current frame end (after ffb successfully triggered, else 0)
	cc_row_start	 = 0;	// cc of current row start
	cc_sync_on		 = 0;	// cc of last sync on switching
	cc_sync_off		 = 0;	// cc of last sync off switching
	current_row		 = 0;	// crt raster row
	current_lcntr	 = 0;	// 3 bit low line counter [0..7] of ula
	fbu_idx			 = 0;	// current pixel byte deposition index in frame_buffer[] dep. on cc_sync_on or cc_sync_off

	set60Hz(is60hz);
}

void UlaZx80::set60Hz(bool is60hz)
{
	bool machine_is60hz = machine->model_info->frames_per_second > 55;
	info = machine_is60hz == is60hz ? machine->model_info : &zx_info[is60hz ? ts1000 : zx80];

	cc_per_line			= info->cpu_cycles_per_line;
	lines_before_screen = info->lines_before_screen;
	lines_after_screen  = info->lines_after_screen;
	lines_per_frame = lines_before_screen + lines_in_screen + lines_after_screen;

	Ula::set60Hz(is60hz);
}


void UlaZx80::enableMicOut( bool f )
{
	beeper_volume = f ? 0.05 : 0.0025;
	beeper_current_sample = beeper_current_sample>0.0 ? beeper_volume : -beeper_volume;
}


inline
void UlaZx80::mic_out( Time now, Sample s )
{
	Dsp::outputSamples( beeper_current_sample, beeper_last_sample_time, now );
	beeper_last_sample_time = now;
	beeper_current_sample = s;
}


inline
void UlaZx80::self_trigger_hsync ( int32 cc )
{
	while( cc >= cc_for_row_end_max() )			// hsync self triggered?
	{
		cc_row_start += cc_per_row_max;
		current_row  += 1;
	}
}


inline
void UlaZx80::pad_video_bytes ( int32 cc, uchar byte )
{
// frame buffer mit schwarzen o. weißen pixeln auffüllen:
	int i = fbu_idx;
	int e = fbu_idx_for_cc(cc);
	assert(e<frame_data_alloc);				// really real limit
	while( i<e ) frame_data[i++] = byte;
	fbu_idx = i;
}


inline
void UlaZx80::pad_video_bytes ( int32 cc )
{
	pad_video_bytes( cc, sync_on() ? black_pixels : white_pixels );
}


void UlaZx80::sync_on ( int32 cc )
{
	assert( sync_off() );
	xxlog("OFF:%i\n",int(cc-cc_sync_off));

// add self triggered rows:
	self_trigger_hsync(cc);

// frame buffer mit weißen pixeln auffüllen:
	pad_video_bytes(cc,white_pixels);

	cc_sync_on = cc;
	current_lcntr = ( current_lcntr+1 ) & 0x0007;
}

void UlaZx80::send_frame_to_screen(int32 cc)
{
	cc_frame_end = cc;
	if(lines_per_frame != current_row)
	{
		lines_per_frame = current_row;
		lines_after_screen = lines_per_frame - lines_before_screen - lines_in_screen;
	}
	cc_row_start = cc;						// Versuchsweise...

	int frame_w = this->frame_w * 8;
	int frame_h = lines_per_frame;
	int screen_w = this->screen_w * 8;
	int screen_h = lines_in_screen;
	int screen_x0 = this->screen_x0 * 8;
	int screen_y0 = lines_before_screen;

	bool new_buffer_in_use = ScreenMonoPtr(screen)->ffb_or_vbi( frame_data, frame_w, frame_h, screen_w, screen_h,
													  screen_x0, screen_y0, 0);
	if(new_buffer_in_use) std::swap(frame_data,frame_data2);

	fbu_idx		= 0;
	current_row	= 0;
}

void UlaZx80::sync_off ( int32 cc )
{
	assert( sync_on() );
	xxlog("ON: %i\n",int(cc-cc_sync_on));

// add self triggered rows:
	self_trigger_hsync(cc);

// hsync:
	if( cc >= cc_for_row_end_min() )			// if at end of row
	{
		cc_row_start = cc;
		current_row  += 1;
	}

// vsync:
	if( cc >= cc_for_frame_end_min() &&			// if at frame end and long enough
		cc >= cc_sync_on+cc_for_vsync_min )
	{
		send_frame_to_screen(cc);
	}

// frame buffer mit schwarzen pixeln auffüllen:
	pad_video_bytes(cc,black_pixels);

	cc_sync_off = cc;
}

void UlaZx80::output ( Time now, int32 cc, uint16 /*addr*/, uint8 /*byte*/ )
{
//	any out() deactivates the SYNC signal
	if( sync_on() )
	{
		sync_off(cc);							// Achtung: ZX80 Laderoutine ist phasenempfindlich!
		mic_out(now,beeper_volume);				// TODO: müsste eigentlich in sync_off()
		if(machine->taperecorder->isRecording()) machine->taperecorder->output(cc,1);
	}
}

void UlaZx80::input ( Time now, int32 cc, uint16 addr, uint8& byte, uint8& mask )
{
	assert(~addr&0x0001);		// not my address

	mask = 0xff;

// any in(FE) activates the SYNC signal and resets the LCNTR of the ULA
// insbes. auch IN(7FFE) weil das wird in der SAVE-Routine benutzt!
	if( sync_off() )
	{											// Achtung: ZX80 Laderoutine ist phasenempfindlich!
		mic_out(now,-beeper_volume);			// TODO: müsste eigentlich in sync_on()
		if(machine->taperecorder->isRecording()) machine->taperecorder->output(cc,0);
		sync_on(cc);
	}
	current_lcntr = 0;

// insert bits from keyboard:
	byte &= readKeyboard(addr);	// Ähh.. TODO: wird auch high getrieben?

// D6: framerate jumper:
	if(is60hz)
	{
		byte &= ~0x40;
	}

// insert bit from mic socket
// signal from EAR input is read into bit 5.
	if(machine->taperecorder->isPlaying())
	{
		if(!machine->taperecorder->input(cc)) byte &= ~EAR_IN_MASK;
	}
	else if(machine->audio_in_enabled)
	{
		Sample const threshold = 0.01;			// to be verified
		uint32 a = uint32(now*samples_per_second);
		if( a>=DSP_SAMPLES_PER_BUFFER+DSP_SAMPLES_STITCHING )
		{
			assert(int32(a)>=0);
			showAlert( "Sample input beyond dsp buffer: +%i\n", int(a-DSP_SAMPLES_PER_BUFFER) );
			if(0.0<=threshold) byte &= ~EAR_IN_MASK;
		}
		else
		{										// Achtung: ZX80 Laderoutine ist phasenempfindlich!
			if(Dsp::audio_in_buffer[a]<=threshold) byte &= ~EAR_IN_MASK;
		}
	}
	else byte &= ~EAR_IN_MASK;
}

void UlaZx80::crtcRead ( int32 cc, uint byte )
{
// an instruction was read at an address with A15=1 and returned an opcode with A6=0
// => the Ula reads a video byte and fakes a NOP for the CPU
//	  the NOP is already handled in the Z80 macro

//	Im Refreshzyklus gibt die CPU das I-Register auf A9-A15 aus
//		und die ULA die 6 Datenbits des Zeichencodes auf A3-A8
//		und den internen Zeilenzähler LCNTR auf A0-A2
//	Das so adressierte Byte (im Characterset des ROMs) wird in den Videoshifter geladen
//		War Bit 7 des Bytes aus dem VRAM gesetzt, werden die Pixel invertiert.

	if( sync_on() ) return;					// SYNC => BLACK!

	assert( cc <= cc_per_frame_max );

	xxlog("CHR:%i\n",int(cc-cc_sync_off));

	Z80* cpu = machine->cpu;
	uint  ir = cpu->getRegisters().ir;		// i register					// note: r nicht aktualisiert
	uchar  b = cpu->peek( (ir&0xfe00)|((byte<<3)&0x01f8)|current_lcntr );	// TODO: Sichtbarkeit für ULA evtl. anders!

// add self triggered rows:
	self_trigger_hsync(cc);

// frame buffer mit weißen pixeln auffüllen
	pad_video_bytes(cc,white_pixels);

// und das pixelbyte eintragen:
	frame_data[ fbu_idx-1 ] = byte&0x0080 ? b : ~b;
}

/*	special callback for ZX80/ZX81 screen update:
*/
uint8 UlaZx80::interruptAtCycle ( int32 cc, uint16 /*pc*/ )
{
// Interrupt acknowledge:
// => IORQ, WR and M1 are active at the same time
// => the SYNC signal is activated
//	  it remains active until next-plus-one M1 cycle (~5µs)
//	  then it automatically deactivates
	if(sync_off()) sync_on(cc);
	sync_off(cc+20);
	return 0xff;
}

int32 UlaZx80::updateScreenUpToCycle ( int32 cc )
{
	xlogIn("UlaZx80::updateScreenUpToCycle");

// add self triggered rows:
	self_trigger_hsync(cc);

// frame buffer mit schwarzen o. weißen pixeln auffüllen:
	pad_video_bytes(cc);

	return cc;
}

int32 UlaZx80::doFrameFlyback ( int32 cc )
{
	assert( cc < cc_per_frame_max+cc_per_row );

	xxlog("FFB:%i\n",int(cc-cc_frame_start));

	// self-trigger vsync?
	if( cc_frame_end==0 )
	{
		// add self triggered rows:
		self_trigger_hsync(cc);

		// frame buffer mit schwarzen o. weißen pixeln auffüllen:
		pad_video_bytes( cc_row_start/*+cc_per_row*/ );

		if(sync_on()) cc_sync_on = cc; else cc_sync_off = cc;

		send_frame_to_screen(cc);
	}

	cc_per_frame		=  cc_frame_end - cc_frame_start;
	cc_sync_on			-= cc_per_frame;		// cc of last sync on switching
	cc_sync_off			-= cc_per_frame;		// cc of last sync off switching
	cc_row_start		-= cc_per_frame;		// cc of current row start
	//cc_frame_start  	-= cc_per_frame;		// cc of current frame start
	cc_frame_end		=  0;					// cc of current frame end (after ffb successfully triggered, else 0)

	//assert( current_row >= 0 && current_row < max_rows_per_frame );
	//assert( cc_sync_off >= 0 || cc_sync_on >= 0 );
	//assert( cc_row_start >= 0 );

	return cc_per_frame;
}

void UlaZx80::drawVideoBeamIndicator(int32 /*cc*/)
{
	// TODO();
}



void UlaZx80::saveToFile( FD& fd ) const throws
{
	UlaMono::saveToFile(fd);
	TODO();
}

void UlaZx80::loadFromFile( FD& fd ) throws
{
	UlaMono::loadFromFile(fd);
	TODO();
}

























