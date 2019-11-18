/*	Copyright  (c)	Günter Woigk 2012 - 2019
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
#include "util.h"
#include "unix/FD.h"
//#include "ZxInfo/ZxInfo.h"
//#include "Files/Z80Head.h"
//#include "Asm/Z80Assembler.h"


cstr MHzStr( Frequency f )
{
	str s = tempstr(16);
	if( f>=1000000 ) sprintf(s,"%.5g MHz",f/1000000.0); else
	if( f>=1000    ) sprintf(s,"%.5g kHz",f/1000.0); else
					 sprintf(s,"%.5g Hz", f);
	return s;
}


#if 0
void Malloc()
{
	ptr p[18];

	for(uint i=0;i<18;i++)
	{
		p[i] = new char[1<<i];
		memset(p[i],0,1<<i);
	}
	for(uint i=0;i<18;i++)
	{
		delete[] p[i];
	}
}
#endif


/*	clears and sets errno
*/
int32 intValue(cstr s)
{
	errno = ok;
	if(s[0]==0)	  { errno=EINVAL; return 0; }
	if(s[0]=='$') return int32(strtol(s+1,NULL,16));
	if(s[0]=='%') return int32(strtol(s+1,NULL,2));
	else		  return int32(strtol(s,NULL,10));
}


double mhzValue( cstr s )
{
	str vs = lowerstr(s);
	double f = 1.0;
	ptr p;
	if( (p=find(vs,"mhz")) ) { *p=0; f=1e6; } else		// 12.34 MHz
	if( (p=find(vs,"khz")) ) { *p=0; f=1e3; } else		// 12.34 kHz
	if( (p=find(vs,"hz")) )  { *p=0; }					// 12.34 Hz

	double v = strtod(vs,&p); while(*p==' ') p++; if(*p!=0) return -1;
	return v * f;
}


int32 intValue(QString s)
{
	return intValue(s.toUtf8().data());
}

double mhzValue(QString s)
{
	return mhzValue(s.toUtf8().data());
}


uint16 printablechar(uint8 c)
{
	return c<0x20u ? 0xB7 : c<0x7Fu ? c : c<=0xA0u||c==0xADu ? 0xB7 : c;		// unprintable -> middle-dot
}




















