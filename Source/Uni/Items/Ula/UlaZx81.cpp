/*	Copyright  (c)	Günter Woigk 2008 - 2018
					mailto:kio@little-bat.de

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Permission to use, copy, modify, distribute, and sell this software and
	its documentation for any purpose is hereby granted without fee, provided
	that the above copyright notice appear in all copies and that both that
	copyright notice and this permission notice appear in supporting
	documentation, and that the name of the copyright holder not be used
	in advertising or publicity pertaining to distribution of the software
	without specific, written prior permission.  The copyright holder makes no
	representations about the suitability of this software for any purpose.
	It is provided "as is" without express or implied warranty.

	THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
	EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
	CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
	DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
	PERFORMANCE OF THIS SOFTWARE.

	ZX81 ULA emulation
	------------------

	IN A,(FE)	only A0 decoded
				aktiviert das SYNC-Signal (idR für VSYNC)
				aktiviert den MIC_OUT Ausgang
				resettet den Zeilenzähler LCNTR.
				liest Tastaturmatrix
				liest 50/60 Hz framerate jumper
				liest EAR_IN Eingang
				schaltet den HSYNC-Generator AUS, wenn der NMI-Generator aus ist	(??)

			Erata: [kio 2008-06-12]
				Ich glaube, es gibt keinen separaten HSYNC-Generator, sondern nur den kombinieren NMI/HSYNC-Zähler.
					Entweder läuft der ununterbrochen und es kann nur ein NMI-Enable-Bit geschaltet werden,
					oder er lässt sich anhalten / resetten mittels out(FE), out(FD).
					Ich glaube, der HSYNC wird immer erzeugt, er ist nur bei explizit aktiviertem SYNC nach in(FE) verdeckt.

	OUT (FF),A	no address bit decoded: any OUT will do it! - and also IRPTACK!
				deaktiviert das SYNC-Signal
				deaktiviert den MIC_OUT Ausgang
				Schaltet den HSYNC-Generator EIN	(??)

	IRPTACK		aktiviert das SYNC-Signal bis zum übernächsten M1-Zyklus ( ~18 CPU-Takte ~ 5.5 µsec )
				MIC_OUT-Ausgang entsprechend

	OUT (FE),A	Schaltet NMI-Generator EIN

	OUT (FD),A	Schaltet NMI-Genarator AUS


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

#include "UlaZx81.h"				// TODO: this is currently a UlaZx80
#include "Machine.h"
#include "Z80/Z80.h"
#include "Qt/Screen/ScreenMono.h"
#include "Dsp.h"
#include "ZxInfo.h"
#include "Keyboard.h"
#include "TapeRecorder.h"



#define o_addr  	"----.----.----.----"		// übliche Adresse: $FF
// MIC_OUT ist das SYNC-Signal


#define i_addr		"----.----.----.---0"		// übliche Adresse: $FE
#define FRAMERATE_BIT	6						// 50/60 Hz Wahlschalter (Lötbrücke)
#define EAR_IN_BIT		7
#define EAR_IN_MASK		(1<<7)
// bits 0-4: 5 keys from keyboard (low=pressed), row selected by A8..A15
// bit  6:   50/60 Hz framerate jumper


// colored pixel octets:
#define black_pixels	0x00
#define white_pixels	0xFF


// sync parameters:
#define	cc_hsync_on_after_irpt_ack		0
#define	cc_hsync_off_after_irpt_ack		18

#define	cc_hsync_on_after_nmi			-18-12
#define	cc_hsync_off_after_nmi			0-12

#define	cc_sync_on_after_in_fe			-12
#define	cc_sync_off_after_out			-12


// for logging cc diffs
static int32 cc0=0;


UlaZx81::~UlaZx81()
{}

UlaZx81::UlaZx81(Machine* m)
:
	UlaMono(m, isa_UlaZx81, o_addr, i_addr)
{}

void UlaZx81::powerOn(int32 cc)
{
	xlogIn("UlaZx81:init");
	UlaMono::powerOn(cc);

	assert( int32(bytes_per_row_max)   <= UlaMono::max_bytes_per_row );
	assert( int32(bytes_per_frame_max) <= UlaMono::max_bytes_per_frame );

	frame_w		= bytes_per_row;	// frame width [bytes] == address offset per scan line
	screen_w	= 32;				// screen width [bytes]
	screen_x0	= 11;				// hor. screen offset inside frame scan lines [bytes]

	reset();
}

void UlaZx81::reset ( Time t, int32 cc )
{
	xlogIn("UlaZx81::reset");
	UlaMono::reset(t,cc);
	reset();
}

void UlaZx81::reset()
{
	tv_sync_state		= off;
	tv_cc_sync			= 0;
	tv_cc_row_start		= 0;
	tv_cc_frame_start	= 0;
	tv_idx				= 0;
	tv_row				= 0;

//	mic_out_enabled		= no;
	beeper_volume		= 0.0025;
	beeper_current_sample = -beeper_volume;

	ula_lcntr			= 0;		// 3 bit low line counter [0..7] of ula
	ula_sync_state		= off;		// sync as set by in(FE) / out(FF)

	ula_nmi_enabled		= no;		// set by out(FD) / out(FE)
	ula_cc_next_nmi		= 0;

	set60Hz(is60hz);
}

void UlaZx81::set60Hz(bool is60hz)
{
	bool machine_is60hz = machine->model_info->frames_per_second > 55;
	info = machine_is60hz == is60hz ? machine->model_info : &zx_info[is60hz?ts1000:zx81];

	cc_per_line			= info->cpu_cycles_per_line;
	lines_before_screen = info->lines_before_screen;
	lines_after_screen  = info->lines_after_screen;
	lines_per_frame = lines_before_screen + lines_in_screen + lines_after_screen;

	Ula::set60Hz(is60hz);
}

void UlaZx81::enableMicOut( bool f )
{
	beeper_volume = f ? 0.05 : 0.0025;
	beeper_current_sample = beeper_current_sample>0.0 ? beeper_volume : -beeper_volume;
}

inline void UlaZx81::mic_out( Time now, Sample s )
{
	Dsp::outputSamples( beeper_current_sample, beeper_last_sample_time, now );
	beeper_last_sample_time = now;
	beeper_current_sample = s;
}


/*	bool	tv_sync_state;			// stores current sync state. req. for tv_update()
	int32	tv_cc_sync;				// cc of last sync state change
	int32	tv_cc_row_start;		// cc when current row stated: sync_off or self-triggered
	int32	tv_cc_frame_start;		// cc when frame started: 0 for this frame or cc of next frame after ffb
	int32	tv_idx;					// fbu[idx] for next byte
	int32	tv_row;					// current row
*/

inline void UlaZx81::tv_pad_pixels( int32 cc, uint8 c )
{
	// pad frame buffer with black or white pixels

	int32 a = tv_idx;
	int32 e = tv_row*bytes_per_row + (cc-tv_cc_row_start)/4;
	if(e>a)
	{
		assert( e<max_bytes_per_frame );
		xxlog("[%i-%i:%i]",int(a),int(e),int(c!=0));
		memset( frame_data+a, c, e-a );
		tv_idx = e;
	}
}

void UlaZx81::tv_self_trigger ( int32 cc )
{
	while( cc >= cc_row_end_max() )				// self-trigger hsyncs:
	{
		tv_row += 1;
		tv_cc_row_start += /*tv_sync_state==on ? cc_per_row :*/ cc_per_row_max;
	}

	if( cc >= cc_frame_end_max() )				// self-trigger vsync:
	{
		if(lines_per_frame != tv_row)
		{
			lines_per_frame = tv_row;
			lines_after_screen = lines_per_frame - lines_before_screen - lines_in_screen;
		}
		tv_cc_row_start   =
		tv_cc_frame_start = cc_frame_end_max();
	}
}

inline void UlaZx81::tv_store_byte( uint8 c )
{
	// store one pixel byte now and increment time.

	assert( tv_idx<max_bytes_per_frame-1 );
	xxlog("%2hhx",c);
	frame_data[ tv_idx++ ] = c;
}

void UlaZx81::tv_sync_on( int32 cc )
{
	// activate sync.	assumes: sync was off

	if( cc >= cc_row_end_max() ) tv_self_trigger(cc);
	tv_pad_pixels( cc, white_pixels );
	tv_sync_state = on;
	tv_cc_sync = cc;
}

void UlaZx81::tv_sync_off( int32 cc )
{
	// deactivate sync.	assumes: sync was on

	if( cc >= cc_row_end_max() ) tv_self_trigger(cc);

	if( cc-tv_cc_sync >= cc_for_vsync_min && cc >= cc_frame_end_min() )
	{
		xxlog("^^%i ", int(cc-cc0)); cc0 = cc;

		if(lines_per_frame != tv_row)
		{
			lines_per_frame = tv_row;
			lines_after_screen = lines_per_frame - lines_before_screen - lines_in_screen;
		}
		tv_cc_row_start   =
		tv_cc_frame_start = cc;
	}
	else
	{
		if( cc >= cc_row_end_min() )
		{
			xxlog("^%i ", int(cc-cc0)); cc0 = cc;

			tv_row += 1;
			tv_cc_row_start = cc;
		}
	}

	tv_pad_pixels( cc, black_pixels );
	tv_sync_state = off;
	tv_cc_sync = cc;
}

inline void UlaZx81::tv_update( int32 cc )
{
	// update frame buffer up to cycle. self-trigger sync as needed.

	if( cc >= cc_row_end_max() ) tv_self_trigger(cc);
	tv_pad_pixels( cc, tv_sync_state==on ? black_pixels : white_pixels );
}

inline void UlaZx81::tv_do_frame_flyback( )
{
	if( tv_cc_frame_start == 0 )		// ffb not yet triggered => force it!
	{
		int32 cc = cc_frame_end_max();
		tv_self_trigger( cc );
		tv_pad_pixels( cc, tv_sync_state==on ? black_pixels : white_pixels );
		assert( tv_cc_frame_start!=0 );
	}

	int32 bytes_per_frame = lines_per_frame*bytes_per_row;	// size of framebuffer
	assert( tv_idx >= bytes_per_frame );

	tv_idx -= bytes_per_frame;
	tv_row -= lines_per_frame;

	assert( bytes_per_frame+tv_idx<max_bytes_per_frame );
	if(tv_idx>0) memcpy( frame_data, frame_data+bytes_per_frame, tv_idx ); else tv_idx=0;
}


// ####################### ULA ###########################


void UlaZx81::ula_run_counters( int32 cc )
{
	if( ula_nmi_enabled )
	{
		while( ula_cc_next_nmi <= cc )
		{
			if( ula_sync_state==off )
			{
				tv_sync_on ( ula_cc_next_nmi+cc_hsync_on_after_nmi );
				tv_sync_off( ula_cc_next_nmi+cc_hsync_off_after_nmi );
			}
			ula_cc_next_nmi += cc_per_row;
			ula_lcntr = ( ula_lcntr+1 ) & 0x0007;
		}
	}
}

void UlaZx81::output( Time now, int32 cc, uint16 addr, uint8 /*byte*/ )
{
	xxlog("out_%2x ", uint(uchar(addr)));

// update upto now:
	ula_run_counters(cc);

//	any out()	deactivates the SYNC signal
//				deactivates the MIC_OUT output
//				activates the HSYNC generator		WRONG!
//
//		Erata:
//				There is no separate HSYNC generator.
//				HSYNCs generated by the NMI generator are simply obscured while SYNC is on after in(FE)

// deactivate SYNC:
	if( ula_sync_state==on )
	{
		xxlog("sync_OFF ");
		tv_sync_off( cc + cc_sync_off_after_out );
		mic_out(now,beeper_volume);			// Achtung: ZX80 Laderoutine ist phasenempfindlich!
		if(machine->taperecorder->isRecording()) machine->taperecorder->output(cc,1);
		ula_sync_state = off;
	}

	ula_cc_next_nmi = (cc+cc_per_row) / cc_per_row * cc_per_row;

// out(FE)		activates the NMI generator
	if( (addr&1)==0 )	// FE
	{
		if( !ula_nmi_enabled )
		{
			xxlog("nmi_gen_ON ");
			ula_nmi_enabled = yes;
			machine->cpu->setNmi( ula_cc_next_nmi );
		}
	}

// out(FD)		deactivates NMI generator
	if( (addr&2)==0 )	// FD
	{
		if( ula_nmi_enabled )
		{
			xxlog("nmi_gen_OFF ");
			ula_nmi_enabled = no;
			machine->cpu->clearNmi();
		}
	}
}

void UlaZx81::input( Time now, int32 cc, uint16 addr, uint8& byte, uint8& mask )
{
	assert(~addr&0x0001);		// not my address

	xxlog("in_%4x ", uint(addr));

	mask = 0xff;		// to be tested

// update upto now:
	ula_run_counters(cc);

// in(FE)	activates the SYNC signal
//			deactivates the HSYNC generator if the NMI generator is off			WRONG!
//			activates the MIC_OUT output
//			resets the LCNTR
//			reads keyboard
//			reads framerate jumper
//			reads EAR_IN input
//
//		Erata:																	kio 2008-06-12
//			there is no separate HSYNC generator:
//			HSYNCs generated by the NMI generator are simply obscured after SYNC is switched on with in(FE)

	if( ula_nmi_enabled==off )
	{
		if( ula_sync_state==off )
		{
			xxlog("sync_ON ");
			tv_sync_on( cc + cc_sync_on_after_in_fe );
			mic_out(now,-beeper_volume);	// Achtung: ZX80 Laderoutine ist phasenempfindlich!
			if(machine->taperecorder->isRecording()) machine->taperecorder->output(cc,0);
			ula_sync_state = on;
		}
		ula_lcntr = 0;
	}

// merge in the keys
	byte &= readKeyboard(addr);	// Ähh.. pull ups?

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
		{				// Achtung: ZX80 Laderoutine ist phasenempfindlich!
			if(Dsp::audio_in_buffer[a]<=threshold) byte &= ~EAR_IN_MASK;
		}
	}
	else byte &= ~EAR_IN_MASK;
}

void UlaZx81::crtcRead( int32 cc, uint byte )
{
// an instruction was read at an address with A15=1 and returned an opcode with A6=0
// => the Ula reads a video byte and fakes a NOP for the CPU
//	  the NOP is already handled in the Z80 macro

//	Im Refreshzyklus gibt die CPU das I-Register auf A9-A15 aus
//		und die ULA die 6 Datenbits des Zeichencodes auf A3-A8
//		und den internen Zeilenzähler LCNTR auf A0-A2
//	Das so adressierte Byte (im Characterset des ROMs) wird in den Videoshifter geladen
//		War Bit 7 des Bytes aus dem VRAM gesetzt, werden die Pixel invertiert.

	if( ula_sync_state==on ) return;				// SYNC => BLACK!

	assert( cc < cc_per_frame_max );

	ula_run_counters(cc-4);

	//if(!XXXLOG) { xxlog("."); }

	Z80* cpu = machine->cpu;
	uint  ir = cpu->getRegisters().ir;		// i register				// note: r nicht aktualisiert
	uchar  b = cpu->peek( (ir&0xfe00)|((byte<<3)&0x01f8)|ula_lcntr );	// TODO: Sichtbarkeit für ULA evtl. anders!

	tv_update(cc-4);
	tv_store_byte( byte&0x0080 ? b : ~b );
}

int32 UlaZx81::nmiAtCycle( int32 cc_nmi )
{
	// special callback for ZX81 screen update:

	// update upto now:
	ula_run_counters(cc_nmi);		// generates the syncs

	xxlog("nmi ");

	// NMI triggered at cc:
	//		SYNC is activated for approx. 5 µsec.
	//		if SYNC is forced on with in(FE) then no visible effect
	//		SYNC generation by nmi generator is handled in ula_run_counters()

	return ula_cc_next_nmi;		// re-shedule nmi
}

uint8 UlaZx81::interruptAtCycle( int32 cc, uint16 /*pc*/ )
{
// update upto now:
	ula_run_counters(cc);

	xxlog("int ");

// Interrupt acknowledge:
// ZX80 compatible mode:
//		IORQ, WR and M1 are active at the same time
//		=> the SYNC signal is activated for approx. 5 µsec. (next-plus-one M1 cycle)
//

	// TODO: disabled while nmi_generator_enabled==on ?
	//if( ula_nmi_enabled==off )
	{
		if( ula_sync_state==off )
		{
			tv_sync_on ( cc + cc_hsync_on_after_irpt_ack );
			tv_sync_off( cc + cc_hsync_off_after_irpt_ack );
			ula_lcntr = ( ula_lcntr+1 ) & 0x0007;
		}
	}
	return 0xff;		// bus byte read by cpu in irpt ack cycle
}

int32 UlaZx81::updateScreenUpToCycle( int32 cc )
{
	xlogIn("UlaZx81::updateScreenUpToCycle");

	ula_run_counters(cc);
	tv_update(cc);
	return cc;		// when next
}

int32 UlaZx81::doFrameFlyback( int32 cc )
{
	assert( cc < cc_per_frame_max+cc_per_row );

	ula_run_counters(cc);
	xlogline("ffb");
	tv_do_frame_flyback();
	bool new_buffer_in_use = ScreenMonoPtr(screen)->ffb_or_vbi(frame_data,frame_w*8,lines_per_frame,
										screen_w*8,lines_in_screen,screen_x0*8,lines_before_screen,0);
	if(new_buffer_in_use) std::swap(frame_data,frame_data2);

	cc_per_frame = tv_cc_frame_start;
	return cc_per_frame;
}

void UlaZx81::drawVideoBeamIndicator(int32 /*cc*/)
{
	// TODO
}

void UlaZx81::videoFrameEnd( int32 cc )
{
	assert(cc==tv_cc_frame_start);

	ula_cc_next_nmi -= cc;
	tv_cc_sync -= cc;				// cc of last sync state change
	tv_cc_row_start -= cc;			// cc when current row stated: sync_off or self-triggered
	tv_cc_frame_start -= cc;		// cc when frame started: 0 for this frame or cc of next frame after ffb
	cc0 -= cc;
}

void UlaZx81::saveToFile( FD& fd ) const throws
{
	UlaMono::saveToFile(fd);
	TODO();
}

void UlaZx81::loadFromFile( FD& fd ) throws
{
	UlaMono::loadFromFile(fd);
	TODO();
}











































