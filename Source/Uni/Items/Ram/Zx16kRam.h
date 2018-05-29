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

#ifndef ZX16KRAM_H
#define ZX16KRAM_H

#include "ExternalRam.h"


//  Speicher-Erweiterungen für den Sinclair ZX80 und ZX81
//  erweitert den ZX81 auf 16k

class Zx16kRam : public ExternalRam
{
public:
    explicit Zx16kRam(Machine*);
	virtual ~Zx16kRam();

protected:
    Zx16kRam(Machine*,isa_id);
};


class Memotech16kRam : public Zx16kRam
{
public:
    explicit Memotech16kRam(Machine* m)		:Zx16kRam(m,isa_Memotech16kRam){}
};


class Stonechip16kRam : public Zx16kRam		// TODO: konnte auf 32k erweitert werden.
{
public:
    explicit Stonechip16kRam(Machine* m)	:Zx16kRam(m,isa_Stonechip16kRam){}
};


class Ts1016Ram : public Zx16kRam			// Timex Sinclair
{
public:
    explicit Ts1016Ram(Machine* m)			:Zx16kRam(m,isa_Ts1016Ram){}
};


#endif















