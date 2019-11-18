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

#include "kio/kio.h"
#include "Z80/Z80.h"
#include "ZxInfo/ZxInfo.h"


#define z80v1len    30
#define z80v2len    55
#define z80v3len    86
#define z80maxlen   sizeof(Z80Head)

extern Model modelForZ80(FD &fd) throws;


struct Z80Head
{
	uint8	a,f,c,b,l,h,pcl,pch,spl,sph,i,r, data,
			e,d,c2,b2,e2,d2,l2,h2,a2,f2,
			yl,yh,xl,xh,iff1,iff2,im;

	// Z80 version 2.01:
	uint8	h2lenl,h2lenh,npcl,npch,model;
	union{	uint8 port_7ffd;uint8 port_f4; };
	union{	uint8 if1paged; uint8 port_ff; };
	uint8	rldiremu;
	uint8	port_fffd, soundreg[16];

	// Z80 version 3.0:
	uint8	t_l,t_m,t_h,spectator,mgt_paged,multiface_paged,
			ram0,ram1,joy[10],stick[10],mgt_type,
			disciple_inhibit_button_status,disciple_inhibit_flag;

	// warajewo/xzx extension:
	uint8	port_1ffd;

	// zxsp extension:
	uint8	spectra_bits;
	uint8	spectra_port_7fdf;

// Member functions:
	void    clear	()              {memset(this,0,sizeof(Z80Head));}
	void    read    (FD& fd)		throws;
	void    write   (FD &fd)		throws;

	void    setRegisters(Z80Regs const&);		// put regs into Z80Head
	void    getRegisters(Z80Regs&) const;		// get regs from Z80Head

	Model	getZxspModel();
	void    setZxspModel(Model, bool if1, bool mgt);
	uint32	getRamsize();

	void    setCpuCycle(int32 cc, int cc_per_frame);
	int32   getCpuCycle(int cc_per_frame);

	bool    isVersion145()	const   {return (pch|pcl)!=0;}
	bool    isVersion201()	const	{return (pch|pcl)==0 && h2lenl<=23;}
	bool    isVersion300()	const   {return (pch|pcl)==0 && h2lenl>23;}
	bool	varyingRamsize() const	{return model>=76 && model<=83;}
};













