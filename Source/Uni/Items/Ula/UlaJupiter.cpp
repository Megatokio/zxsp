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
*/

#include "UlaJupiter.h"
#include "Machine.h"
#include "Z80/Z80.h"
#include "Qt/Screen/ScreenMono.h"
#include "Dsp.h"
#include "ZxInfo.h"
#include "TapeRecorder.h"


/*	io adress and bits and mic_out and beeper:
	verified with circuit diagram kio 2008-05-31

	The whole Jupiter Ace has only one io address which decodes A0 only.
	Reading returns keys from the keyboard and the ear_in state in D5.
	Writing sets the mic_out state from bit D3.

	Reading port A0 switches off the speaker and writing switches it on.

	note: there is a small capacitor between speaker and mic socket,
	which might result in a faint signal from the speaker
	beeing coupled into mic-out. or vive versa.	to be investigated...

	The speaker and the mic_out line are set by different methods and may differ.
	But zxsp supports only a common audio-out path for audio and tape output.
	There are 3 modes of audio-out handling:
		1 - mic_out_only
		2 - speaker_only
		3 - mixed_audio

	mixed_audio:

	mixed_audio works like this:
		out($FE) toggles the audio output state
		in($FE)	 resets the oudio output state

	this works if out() is only issued to toggle the state
	and in() occurs only at the same phase of the audio signal.
	this should work with the rom beep and rom tape save routine.

	beep:
		the rom routine repeatedly issues in() and out()
		=>	in() resets, out() toggles, thus sets, the next in() resets and so forth.

	save:
		the audio-out state is assumed to be low at subroutine entry:
		the last io action must have been an in(), e.g. from a keyboard scan.
		then repeated pairs of out() for each bit toggle audio-out high and back low again.
		the in() for the user break test comes after each full byte, and thus occurs
		when audio-out is low: the audio-out state is not changed by this in().

		limitations:
		- if the audio-out state is not low at the start
		- if an odd number of pulses is generated for the pilot sequence
		- if an odd number of pulses is used for the sync pulse
		- if bits are not saved as pairs of out()
		- if multiple out()s are done for the same mic_out state
		- if the break test in() occurs in the middle of a bit
		then mixed_audio mode will miss or insert an edge and produce a buggy signal.

	Timing:
		XTAL		= 6.5 MHz
		PIXEL.CLK	= XTAL
		CPU.CLK		= CNT0	= XTAL÷2
		CHAR.CLK	= CNT2	= XTAL÷8
					  CLK3 to CLK7 serve as vram column address

		CNT3		= CPU.CLK÷8
		CNT4		= CPU.CLK÷16
		Z10A.CLR	= CNT5 & CNT7 & CNT8 = 1+4+8 = 13
		CNT8		= CNT4÷12 = CPU.CLK÷(16*13) = CHAR.CLK÷(4*13) = CHAR.CLK÷(32+8+4+8)
		ROW.CLK		= CNT8 = CHAR.CLK÷52 = CPU.CLK÷208 = XTAL÷416
					  CNT9 to CNT11 serve as cram row address
					  CNT12 to CNT16 serve as vram rom address

		CNT11		= ROW.CLK÷8
		Z11AB.CLR	= CNT12+CNT13+CNT14+CNT17 = 1+2+4+32 = 39
		FRAME.CLK	= CNT17 = CNT11÷39 = ROW.CLK÷(8*39) = ROW.CLK÷312

	Interrupt:
		INT			= CNT12+CNT13+CNT14+CNT15+CNT16
		INT			= CHAR.ROW 1+2+4+8+16 = CHAR.ROW 31: from start to end: 8*52*4 CPU.CLKs


; MEMORY MAP
;
; $0000 +======================================================+
;       |                                                      |
;       |                   ROM 8K                             |
;       |                                     v $2300          |
; $2000 +======================================================+ - - - - - -
;       |       copy of $2400                 |0|<  cassette  >|
; $2400 +-------------------------------------+-+--------------+
;       |       VIDEO MEMORY 768 bytes        |0| PAD 254 bytes| 1K RAM
; $2800 +-------------------------------------+-+--------------+
;       |       copy of $2c00                 ^ $2700          |
; $2C00 +------------------------------------------------------+
;       |       CHARACTER SET - Write-Only                     | 1K RAM
; $3000 +------------------------------------------------------+
;       |       copy of $3c00                                  |
; $3400 +------------------------------------------------------+
;       |       copy of $3c00                                  |
; $3800 +------------------------------------------------------+
;       |       copy of $3c00                                  |
; $3C00 +-------+----------------------------------------------+
;       |SYSVARS| DICT {12} DATA STACK ->         <- RET STACK | 1K RAM
; $4000 +=======+==============================================+ - - - - - -
;
; The 768 bytes of video memory is accessed by the ROM using addresses
; $2400 - $26FF. This gives priority to the video circuitry which also needs
; this information to build the TV picture. The byte at $2700 is set to zero
; so that it is easy for the ROM to detect when it is at the end of the screen.
; The 254 bytes remaining are the PAD - the workspace used by FORTH.
; This same area is used by the tape recorder routines to assemble the tape
; header information but since, for accurate tape timing, the FORTH ROM needs
; priority over the video circuitry, then the ROM uses addresses $2301 - $23FF.
;
; Similarly the Character Set is written to by the ROM (and User) at the 1K
; section starting at $2C00. The video circuitry accesses this using addresses
; $2800 - $2BFF to build the TV picture. It is not possible for the ROM or User
; to read back the information from either address.
*/

#define o_addr		"----.----.----.---0"		// übliche Adresse: $FE
#define	MIC_OUT_BIT	3
#define	MIC_OUT_MASK (1<<3)
// bits 0-4: 5 keys from keyboard (low=pressed), row selected by A8..A15
// bits 6+7: floating

#define i_addr		"----.----.----.---0"		// übliche Adresse: $FE
#define EAR_IN_BIT	5
#define EAR_IN_MASK	(1<<5)
// read port: switch off spkr
// write port: switch on spkr




UlaJupiter::~UlaJupiter()
{}


UlaJupiter::UlaJupiter(Machine* m, uint fps)
:
	UlaMono(m,isa_UlaJupiter,o_addr, i_addr)
{
	audio_mode = mixed_audio;		// or: read from prefs?
	border_color = 0x00;			// black

	frame_w	  = 32+8+4+8;			// frame width [bytes] == address offset per scan line
	screen_w  = 32;					// screen width [bytes]
	screen_x0 = 0;					// hor. screen offset inside frame scan lines [bytes]

	set60Hz(fps==60);
}

void UlaJupiter::set60Hz(bool is60hz)
{
	//				BILD + VOR + SYNC + NACH
	//	ZEILEN:		24   +  7  +  1   + 7          (50 Hz)
	//				24   +  4  +  1   + 4          (60 Hz)
	//	ZEICHEN:	32   +  8  +  4   + 8

	lines_before_screen = is60hz ? 32 : 56;
	//lines_in_screen  	= 192;
	lines_after_screen	= is60hz ? 40 : 64;
	lines_per_frame		= lines_before_screen + lines_in_screen + lines_after_screen;
	cc_per_line			= info->cpu_cycles_per_line;
	lines_per_frame = lines_before_screen + lines_in_screen + lines_after_screen;

	Ula::set60Hz(is60hz);
}

void UlaJupiter::powerOn(int32 cc)
{
	xlogIn("UlaJupiter:init");
	UlaMono::powerOn(cc);
}

void UlaJupiter::reset ( Time t, int32 cc )
{
	xlogIn("UlaJupiter::reset");
	UlaMono::reset(t,cc);
}

int32 UlaJupiter::updateScreenUpToCycle ( int32 cc )
{
	/*	currently cycle-precise vram access is not implemented
		and updateScreenUpToCycle() and markVideoRam() are nops.
		the whole screen is read in doFrameFlyback().
	*/
	return cc;
}

void UlaJupiter::markVideoRam()
{
	/*	currently cycle-precise vram access is not implemented
		and updateScreenUpToCycle() and markVideoRam() are nops.
		the whole screen is read in doFrameFlyback().
	*/
	return;
}

int32 UlaJupiter::doFrameFlyback( int32 /*cc*/ )
{
	CoreByte* vram = machine->ram.getData();	// videoram = 1st 1K page (768 bytes = 24*32 char used); bit 7: inverse
	CoreByte* cram = vram+1024;					// charram  = 2nd 1K page (1K = 128 char * 8 bytes/char )

	uint lines_per_frame = min(uint(this->lines_per_frame), uint(max_rows_per_frame));

	memset(frame_data, 0x00, frame_w*lines_per_frame);

	uint8* z = frame_data + screen_x0 + lines_before_screen * frame_w;	// first byte of screen$ data

	for( int charrow=0; charrow<24; charrow++ )
	{
		for( int y = 0; y<8; y++ )
		{
			for( int x=0; x<32; x++ )
			{
				CoreByte c = vram[charrow*32+x];	// current character  -  note: data byte is in low byte of uint16
				char   b = cram[(c&0x007F)*8+y];	// current pixel octet from character glyph
				*z++ = (char(c)>>7) ^ b;
			}
			z += frame_w - 32;
		}
	}

	machine->cpu->setInterrupt( 0, 8 * cc_per_line );

	bool new_buffer_in_use = ScreenMonoPtr(screen)->ffb_or_vbi(frame_data,frame_w*8,lines_per_frame,screen_w*8,
												lines_in_screen,screen_x0*8,lines_before_screen,0);
	if(new_buffer_in_use) std::swap(frame_data,frame_data2);

	return cpuCycleOfFrameFlyback();			// cc_per_frame for last frame
}

void UlaJupiter::drawVideoBeamIndicator(int32 /*cc*/)
{
	//	TODO();
}

void UlaJupiter::output( Time now, int32 cc, uint16 addr, uint8 byte )
{
	assert(~addr&0x0001);		// not my address

// out($FE) switches on the beeper current
// bit 3 sets MIC_OUT state

// TAPE:
	if(machine->taperecorder->isRecording())
	{
		machine->taperecorder->output(cc,byte&MIC_OUT_MASK);
	}

// BEEPER:
// switch on beeper current:
//		mixed_audio:	toggle audio-out
//		speaker_only:	set audio-out to ON
//		mic_out_only:	set audio-out acc. to D3
	Sample new_sample = audio_mode==mixed_audio ? beeper_volume-beeper_current_sample
					  : audio_mode==speaker_only || (byte&MIC_OUT_MASK) ? beeper_current_sample : 0.0;
	if( new_sample != beeper_current_sample )
	{
		Dsp::outputSamples( beeper_current_sample, beeper_last_sample_time, now );
		beeper_last_sample_time = now;
		beeper_current_sample = new_sample;
	}

	ula_out_byte = byte;
}

void UlaJupiter::input( Time now, int32 cc, uint16 addr, uint8& byte, uint8& mask )
{
	assert(~addr&0x0001);		// not my address

// in($FE) switches off the beeper current
// bits 0-4 come from the keyboard
// bit  5	is the signal from the ear socket
// bits 6+7 are (probably) always 1
	mask = 0xff; // to be verified

	byte &= readKeyboard(addr);	// Ähh.. TODO: wird auch high getrieben?

// BEEPER:
// switch off beeper current:
//		mic_out_only:	in(FE) does not affect audio-out
//		speaker_only:	set audio-out to OFF
//		mixed_audio:	set audio-out to OFF
	if( beeper_current_sample!=0.0 && audio_mode != mic_out_only )
	{
		Dsp::outputSamples( beeper_current_sample, beeper_last_sample_time, now );
		beeper_last_sample_time = now;
		beeper_current_sample = 0.0;
	}

// insert bit from mic socket
// signal from EAR input is read into bit 5.

	if(machine->taperecorder->isPlaying())
	{
		if(machine->taperecorder->input(cc)) byte |= EAR_IN_MASK; else byte &= ~EAR_IN_MASK;
	}
	else if(machine->audio_in_enabled)
	{
		Sample const threshold = 0.01;		// to be verified
		uint32 a = uint32(now*samples_per_second);
		if( a>=DSP_SAMPLES_PER_BUFFER+DSP_SAMPLES_STITCHING )
		{
			assert(int32(a)>=0);
			showAlert( "Sample input beyond dsp buffer: +%i\n", int(a-DSP_SAMPLES_PER_BUFFER) );
			if(0.0<threshold) byte &= ~EAR_IN_MASK;
		}
		else
		{
			if(Dsp::audio_in_buffer[a]<threshold) byte &= ~EAR_IN_MASK;
		}
	}
	else byte &= ~EAR_IN_MASK;
}

void UlaJupiter::saveToFile( FD& fd ) const throws
{
	UlaMono::saveToFile(fd);
	fd.write(audio_mode);
}

void UlaJupiter::loadFromFile( FD& fd ) throws
{
	UlaMono::loadFromFile(fd);
	fd.read(audio_mode);
}




























