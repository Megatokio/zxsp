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

#ifndef TC2068JOY_H
#define TC2068JOY_H

#include "Joy.h"


class Tc2068Joy : public Joy
{
public:
	Tc2068Joy(Machine* m, isa_id id=isa_Tc2068Joy)	:Joy(m,id,internal,NULL,NULL,"J1","J2"){}

	// Item interface:
	void input(Time,int32,uint16,uint8&,uint8&)	{}
};


class Ts2068Joy : public Tc2068Joy
{
public:
	explicit Ts2068Joy(Machine* m)		:Tc2068Joy(m,isa_Ts2068Joy){}
};


class U2086Joy : public Tc2068Joy
{
public:
	explicit U2086Joy(Machine* m)		:Tc2068Joy(m,isa_U2086Joy){}
};


#endif

