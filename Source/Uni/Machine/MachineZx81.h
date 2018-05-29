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
*/


#ifndef MACHINEZX81_H
#define MACHINEZX81_H

#include "Machine.h"
#include "ZxInfo/info.h"
#include "Ula/UlaZx81.h"
#include "Ula/MmuZx81.h"


class MachineZx81 : public Machine
{
protected:
	MachineZx81(MachineController* parent, isa_id id, Model);

    bool handleSaveTapePatch() override;
    bool handleLoadTapePatch() override;

public:
	explicit MachineZx81(MachineController*);

	int32	nmiAtCycle(int32 cc_nmi) override		{ return UlaZx81Ptr(ula)->nmiAtCycle(cc_nmi); }

    void	loadP81(FD&, bool p81) throws override;
    void	saveP81(FD&, bool p81) throws override;
};

#endif
