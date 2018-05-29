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

#include "TapeFileDataBlock.h"
#include "TzxData.h"
#include "O80Data.h"
#include "TapData.h"
#include "globals.h"


void TapeFileDataBlock::purge()
{
	mode = stopped;
	cswdata->purge();
	if(tapdata!=tapedata) delete tapdata; tapdata = NULL;
	if(o80data!=tapedata) delete o80data; o80data = NULL;
	if(tzxdata!=tapedata) delete tzxdata; tzxdata = NULL;
    delete tapedata; tapedata = NULL;
    delete[] major_block_info; major_block_info = NULL;
    delete[] minor_block_info; minor_block_info = NULL;
}

void TapeFileDataBlock::purgeBlock()
{
	purge();
	calc_block_infos();
}

TapeFileDataBlock::~TapeFileDataBlock()
{
	purge();
    delete cswdata;
}


TapeFileDataBlock::TapeFileDataBlock(CswBuffer* p)
:
	cswdata(p),
	tapedata(NULL),
	tapdata(NULL),
	o80data(NULL),
	tzxdata(NULL),
	major_block_info(NULL),
	minor_block_info(NULL),
	mode(stopped)
{
	assert(cswdata);
	calc_block_infos();
}


TapeFileDataBlock::TapeFileDataBlock(TapeData* q, uint32 ccps)
:
	cswdata(new CswBuffer(*q,ccps)),
	tapedata(q),
	tapdata(q->isaId()==isa_TapData ? TapDataPtr(q) : NULL),
	o80data(q->isaId()==isa_O80Data ? O80DataPtr(q) : NULL),
	tzxdata(q->isaId()==isa_TzxData ? TzxDataPtr(q) : NULL),
	major_block_info(NULL),
	minor_block_info(NULL),
	mode(stopped)
{
	calc_block_infos();
}

TapeFileDataBlock::TapeFileDataBlock(TapeData* q, CswBuffer* csw)
:
	cswdata(csw),
	tapedata(q),
	tapdata(q->isaId()==isa_TapData ? TapDataPtr(q) : NULL),
	o80data(q->isaId()==isa_O80Data ? O80DataPtr(q) : NULL),
	tzxdata(q->isaId()==isa_TzxData ? TzxDataPtr(q) : NULL),
	major_block_info(NULL),
	minor_block_info(NULL),
	mode(stopped)
{
	assert(cswdata);
	calc_block_infos();
}


/*	convert TapeFileDataBlock to standard O80/P81 file data block for ROM LOAD routine
	caller must check o80data.trust_level
*/
O80Data* TapeFileDataBlock::getO80Data() noexcept
{
	if(o80data) return o80data;		// ist schon
	if(tapdata && tapdata->trust_level>=TapeData::conversion_success) return NULL;	// no chance
	return o80data = new O80Data(*cswdata);
}

/*	convert TapeFileDataBlock to standard TAP file data block for ROM LOAD routine
	caller must check tapdata.trust_level
*/
TapData* TapeFileDataBlock::getTapData() noexcept
{
	if(tapdata) return tapdata;		// ist schon
	if(o80data && o80data->trust_level>=TapeData::conversion_success) return NULL;	// no chance
	return tapdata = new TapData(*cswdata);
}


void TapeFileDataBlock::calcBlockInfos()
{
	delete[] major_block_info; major_block_info = NULL;
	delete[] minor_block_info; minor_block_info = NULL;
	calc_block_infos();
}

void TapeFileDataBlock::calc_block_infos()
{
	if(major_block_info) return;

	if(isEmpty())
	{
		major_block_info = newcopy("Empty block");
		return;
	}

a:	if(tapdata && tapdata->trust_level>=TapeData::truncated_data_error)
	{
		major_block_info = newcopy(tapdata->calcMajorBlockInfo());
		minor_block_info = newcopy(tapdata->calcMinorBlockInfo());
		return;
	}
	if(o80data && o80data->trust_level>=TapeData::truncated_data_error)
	{
		major_block_info = newcopy(o80data->calcMajorBlockInfo());
		minor_block_info = newcopy(o80data->calcMinorBlockInfo());
		return;
	}
	if(tzxdata)
	{
		major_block_info = newcopy(tzxdata->getMajorBlockInfo());
		minor_block_info = newcopy(tzxdata->getMinorBlockInfo());
		if(major_block_info) return;
	}
	if(!tapdata)
	{
		getTapData(); if(tapdata) goto a;
	}
	if(!o80data)
	{
		getO80Data(); if(o80data) goto a;
	}
}


void TapeFileDataBlock::videoFrameEnd(uint32 cc)
{
	assert(cswdata);
	assert(mode!=stopped);

	// note: der letzte in|out opcode kann direkt vor dem ffb gelegen haben
	//		 und hatte evtl. schon cc > cc_ffb
	//		=> dann kein seek, wg. abort

	if(cc > cswdata->cc_pos + cswdata->cc_offset)
		cswdata->seekCc(cc);
}


void TapeFileDataBlock::startPlaying (uint32 cc)
{
    assert(cswdata);
    assert(cswdata->ccPerSecond());
    assert(mode==stopped);

    mode = playing;
    cswdata->seekCc(cc);
}


/*	start recording into this block
	• purge block
	• start recording at cc (should be 0)
*/
void TapeFileDataBlock::startRecording (uint32 cc)
{
    assert(cswdata);
    assert(cswdata->ccPerSecond());

	purge();
    mode = recording;
    cswdata->startRecording(cc);
}


void TapeFileDataBlock::stop(CC cc)
{
    assert(cswdata);
    assert(mode!=stopped);

    if(mode==recording)
    {
        cswdata->stopRecording(cc);
        calcBlockInfos();
    }
    else
    {
        cswdata->seekCc(cc);
    }

    mode = stopped;
}




























