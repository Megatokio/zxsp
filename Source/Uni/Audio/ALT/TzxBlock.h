/*	Copyright  (c)	Günter Woigk 2000 - 2015
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


#ifndef _TzxBlock_h_
#define _TzxBlock_h_

#include "kio/kio.h"


// -------------------------------------------------------------
//          Implementations for the various TZX blocks:
// -------------------------------------------------------------

class TzxBlock
{
	friend class Block21;
	friend class Block24;
	friend class Block11;

protected:
	uint		id;			// Tzx Block ID
	TzxBlock*	next;		// linked list if a TapeData block consists of more than 1 TzxBlock
	bool		has_data;	// write(CswBuffer) will (may) actually write pulses

private:
virtual	void    read        (FD&)				  =0;	// read this block from file, except id-byte
virtual void    write       (FD&)			const =0;	// write this block to file, except id-byte
virtual void    write		(CswBuffer&)	const {}	// store this block to CSW buffer

virtual cstr    get_info	()				const { return NULL; }		// get block info of this block, if any
virtual bool    is_end_block()				const { return no; }	// most times: does it end with a pause?

static	TzxBlock* read_next	(FD& fd)		throw(data_error, file_error);
		TzxBlock* last		();

public:

public:
				TzxBlock    (uint id, bool has_data);
virtual         ~TzxBlock   ();

		TzxBlock& operator+	(TzxBlock* n)	{ next = n; return *this; }

		// read a (linked list of) TzxBlocks from file
static	TzxBlock* readFromFile(FD&, uint=0)	throw(data_error, file_error);

		// write a (liked list of) TzxBlocks to file
		void writeToFile (FD&)				const throw(data_error);

		// render a (linked list of) TzxBlocks into a CswBuffer
		void storeCsw(CswBuffer&)			const;

		// Get block info from (list of) TzxBlocks, if any
		cstr getBlockInfo()					const;

		// append block at end of (list of) this block
		void append(TzxBlock* b)			{ TzxBlock* p=this; while(p->next) p=p->next; p->next=b; }

		// does this (list of) blocks generate pulses? (mostly, it should)
		bool hasData ()						const;
};



/*	Conversion:
*/
extern TzxBlock* newTzxBlock(TapData const&);
extern TzxBlock* newTzxBlock(O80Data const&);
extern TzxBlock* newTzxBlock(CswBuffer const&, TzxConversionStyle conversionstyle);


#endif
