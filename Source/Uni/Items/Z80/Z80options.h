#pragma once
/*	Copyright  (c)	Günter Woigk 1996 - 2019
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

	Z80 cpu options
*/


/* #####################################################################

				options and callback functions for zxsp

   ##################################################################### */

typedef uint32_t uint32;

#define	CPU_PAGEBITS		10


// cpu.Run() options:
enum Z80Option : uint32
{	cpu_fastest		= 0u,		// no gimmicks
	cpu_waitmap		= 1u<<8,	// add wait cycles from pg.[rw]waitmap
	cpu_ula_sinclair= 1u<<9,	// Sinclair ZX Spectrum wait style (ULA generates lots of superfluous wait cycles)
	cpu_break_x		= 1u<<10,	// debugger: option and bit position of 'break on execute'	bit in pg.[rw]_flags[]
	cpu_break_w		= 1u<<11,	// debugger: option and bit position of 'break on write'	bit in pg.[rw]_flags[]
	cpu_break_r		= 1u<<12,	// debugger: option and bit position of 'break on read'		bit in pg.[rw]_flags[]
	cpu_patch		= 1u<<13,	// misc.:    option and bit position of 'patch'				bit in pg.[rw]_flags[]
	cpu_crtc_zx81	= 1u<<14,	// ZX80/81:  option and bit position of 'force nop'			bit in pg.[rw]_flags[]
	cpu_crtc		= 1u<<15,	// screen:	 update screen before writing to ram
	cpu_r_access	= 1u<<16,	// debugger: log r access for MemoryAccessInspector
	cpu_w_access	= 1u<<17,	// debugger: log w access for MemoryAccessInspector
	cpu_x_access	= 1u<<18,	// debugger: log x access for MemoryAccessInspector
	cpu_memmapped_r	= 1u<<19,	// extensions: callback for memory mapped io only tested in normal reads (not *ip++)
	cpu_memmapped_w	= 1u<<20,	// extensions: callback for memory mapped io
	cpu_memmapped_rw= 3u<<19,	// extensions: callback for memory mapped io				for convenience
	cpu_floating_bus= 1u<<21,	// unmapped memory: CPU may see what the CRTC is reading

	// just flags, bit in CoreByte not used:
	cpu_break_sp	= 1u<<30,	// debugger: check stack_watchpoint in RET and POP
	//cpu_suspended	= 1u<<31,	// debugger: cpu is in single step mode
	cpu_access		= cpu_r_access|cpu_w_access|cpu_x_access,	// for convenience
	cpu_break_rwx	= cpu_break_r|cpu_break_w|cpu_break_x		// for convenience
};


// cpu.Run() return codes:
enum
{	cpu_cc_expired	= 0,	// T cycle counter reached cc_max (standard exit condition)
	cpu_ic_expired	= 0,	// instruction counter reached ic_max: used for .rzx replay
	cpu_exit_sp,			// exit forced by macro Z80_INFO_POP (stack breakpoint)
	cpu_exit_r,				// exit forced by macro PEEK (options&cpu_break_r)
	cpu_exit_w,				// exit forced by macro POKE (options&cpu_break_w)
	cpu_exit_x				// exit forced by macro GET_INSTR before - or GET_BYTE(RGL) after executing opcode (options&cpu_break_x)
};





