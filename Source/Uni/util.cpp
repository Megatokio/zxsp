/*	Copyright  (c)	Günter Woigk 2012 - 2018
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




















