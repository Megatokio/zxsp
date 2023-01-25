// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include <QGridLayout>
#include <QLabel>
#include <QPalette>
#include <QCheckBox>
#include <QPushButton>
#include <QTimer>
#include "UlaInsp.h"
#include "Ula/Ula.h"
#include "Ula/UlaZxsp.h"
#include "Ula/Mmu.h"
#include "Ula/MmuPlus3.h"
#include "Ula/Mmu128k.h"
#include "Machine.h"
#include "MachineController.h"
#include "Qt/Screen/Screen.h"
#include "ZxInfo.h"
#include "Uni/util.h"


#define l60     45      // most left column line edits
#define r20     15
#define r60     48
#define r100    90     // ula & cpu clock
#define r160    120


static QFont ff("Monaco"/*"Andale Mono"*/,12);

static QLineEdit* new_led( cstr s, uint width )
{
	QLineEdit* e = new QLineEdit(s);
	e->setAlignment(Qt::AlignHCenter);
	e->setFrame(0);
	e->setReadOnly(1);
	e->setFixedWidth(width);
	e->setFixedHeight(16);
	e->setFont(ff);
	return e;
}

UlaInsp::UlaInsp( QWidget* w, MachineController* mc, volatile Machine* m )
: Inspector(w,mc,m->ula,"/Backgrounds/light-150-s.jpg")
{
	uint width  = 300;
	uint height = 230;

	mmu = m->mmu;
	assert(mmu->isA(isa_Mmu));
	assert(object->isA(isa_Ula));

	QGridLayout* g = new QGridLayout(this);
	g->setContentsMargins(10,10,10,10);
	g->setVerticalSpacing(4);
	g->setColumnStretch(0,50);
	g->setColumnStretch(1,0);
	g->setColumnStretch(2,0);
	g->setColumnStretch(3,0);
	g->setColumnStretch(4,50);


// Clocks:

	int row=0;
	values.cpu_clock            = 0;

	inputs.cpu_clock_overdrive	= new_led("",l60);
	inputs.ula_clock            = new_led("",r100);
	inputs.cpu_clock_predivider = new_led("",l60);
	inputs.cpu_clock            = new_led("",r100);

	g->addWidget(new QLabel("ULA clock"), row,0,Qt::AlignRight);
	g->addWidget(inputs.cpu_clock_overdrive,row,1);
	g->addWidget(inputs.ula_clock,row,3,1,2);
	row++;

	g->addWidget(new QLabel("CPU clock"), row,0,Qt::AlignRight);
	g->addWidget(inputs.cpu_clock_predivider,row,1);
	g->addWidget(inputs.cpu_clock,row,3,1,2);
	row++;

//TODO: Divider

// Screen Geometry & timing:

	values.top_rows             = 0;            inputs.top_rows             = new_led("0",l60);
	values.screen_rows          = 192;          inputs.screen_rows          = new_led("192",l60);
	values.bottom_rows          = 0;            inputs.bottom_rows          = new_led("0",l60);
	values.screen_columns       = 256;          inputs.screen_columns       = new_led("256",r60);
	values.bytes_per_row        = 0;            inputs.bytes_per_row        = new_led("0",l60);
	values.cpu_cycles_per_row   = 0;            inputs.cpu_cycles_per_row   = new_led("0",r60);
	values.frames_per_second    = 0.0f;         inputs.frames_per_second    = new_led("0",l60);
	values.cpu_cycles_per_frame = 0;            inputs.cpu_cycles_per_frame = new_led("0",r60);

	g->addWidget(new QLabel("top rows"), row,0,Qt::AlignRight);
	g->addWidget(inputs.top_rows,row,1);
	row++;

	g->addWidget(new QLabel("scrn rows"), row,0,Qt::AlignRight);
	g->addWidget(inputs.screen_rows,row,1);
	//TODO: 'x'
	g->addWidget(inputs.screen_columns,row,3);
	g->addWidget(new QLabel("cols"), row,4,Qt::AlignLeft);
	row++;

	g->addWidget(new QLabel("bott rows"), row,0,Qt::AlignRight);
	g->addWidget(inputs.bottom_rows,row,1);
	row++;

	g->addWidget(new QLabel("bytes/row"), row,0,Qt::AlignRight);
	g->addWidget(inputs.bytes_per_row,row,1);
	g->addWidget(inputs.cpu_cycles_per_row,row,3);
	g->addWidget(new QLabel("cpu cycles"), row,4,Qt::AlignLeft);
	row++;

	g->addWidget(new QLabel("frames/s"), row,0,Qt::AlignRight);
	g->addWidget(inputs.frames_per_second,row,1);
	g->addWidget(inputs.cpu_cycles_per_frame,row,3);
	g->addWidget(new QLabel("cpu cycles"), row,4,Qt::AlignLeft);
	row++;

//TODO: Divider

// Contended Video Ram:

	inputs.checkbox_enable_cpu_waitcycles = nullptr;
	inputs.waitmap_offset = nullptr;
	inputs.waitmap = nullptr;
	if(ula()->isA(isa_UlaZxsp) && UlaZxspPtr(ula())->hasWaitmap())
	{
		values.checkbox_enable_cpu_waitcycles = yes;
		inputs.checkbox_enable_cpu_waitcycles = new QCheckBox("cpu waitcycles (contended video ram)");
	  //inputs.checkbox_enable_cpu_waitcycles->setEnabled(no);
		values.waitmap_offset       = 0;            inputs.waitmap_offset       = new_led("0",l60);
		values.waitmap              = 0x00;         inputs.waitmap              = new_led("0-0-0-0-0-0-0-0",r160);

		g->addWidget(inputs.checkbox_enable_cpu_waitcycles,row,0,1,5);
		row++;

		g->addWidget(new QLabel("offset"), row,0,Qt::AlignRight);
		g->addWidget(inputs.waitmap_offset,row,1);
		g->addWidget(inputs.waitmap,row,3,1,2);
		row++;
	}

//TODO: Divider

// The ULA Byte: Border, Era & Mic:

	values.border_color         = -1;           inputs.border_color         = new_led("",l60);
	values.mic_bit              = 0;            inputs.mic_bit              = new_led("0",r20);
	values.ear_bit              = 0;            inputs.ear_bit              = new_led("0",r20);
	values.frames_hit           = 100.0f;       inputs.frames_hit           = new_led("100%",l60);

	g->addWidget(new QLabel("border"), row,0,Qt::AlignRight);
	g->addWidget(inputs.border_color,row,1);
	QHBoxLayout* earmicbox = new QHBoxLayout();
	earmicbox->setContentsMargins(0,0,0,0);
	earmicbox->setSpacing(4);
	earmicbox->addWidget(inputs.mic_bit);
	earmicbox->addWidget(new QLabel("Mic "));
	earmicbox->addWidget(inputs.ear_bit);
	earmicbox->addWidget(new QLabel("Ear"),100);
	g->addLayout(earmicbox,row,3,1,2);
	row++;


//TODO: Divider

// mmu port 7ffd:

	inputs.port_7ffd = nullptr;
	inputs.port_1ffd = nullptr;
	inputs.page_c000 = nullptr;
	inputs.page_8000 = nullptr;
	inputs.page_4000 = nullptr;
	inputs.page_0000 = nullptr;
	inputs.video_page = nullptr;
	inputs.checkbox_mmu_locked = nullptr;
	inputs.checkbox_ram_only = nullptr;
	inputs.printer_strobe = nullptr;
	inputs.disc_motor = nullptr;

	bool has_7ffd = mmu->hasPort7ffd();
	bool has_1ffd = mmu->hasPort1ffd();

	if(has_7ffd)
	{
		height += 100;
		if(has_1ffd) height += 20;

		values.port_7ffd = 0x00;    inputs.port_7ffd = new_led("$00",l60);
		values.page_c000 = 0;       inputs.page_c000 = new_led("page 0",r60);
		values.page_8000 = 0;       inputs.page_8000 = new_led("page 0",r60);
		values.page_4000 = 0;       inputs.page_4000 = new_led("page 0",r60);
		values.page_0000 = 0;       inputs.page_0000 = new_led("page 0",r60);
		values.video_page = 0;      inputs.video_page = new_led("page 0",r60);
		inputs.checkbox_mmu_locked = new QCheckBox("mmu port locked");
		if(has_1ffd)
		{
			values.port_1ffd = 0x00; inputs.port_1ffd = new_led("$00",l60);
			inputs.checkbox_ram_only = new QCheckBox("ram only");
			values.printer_strobe=0; inputs.printer_strobe = new_led("0",r20);
			values.disc_motor=0; inputs.disc_motor = new_led("0",r20);
		}

		g->addWidget(new QLabel("port $7ffd"), row,0,Qt::AlignRight);
		g->addWidget(inputs.port_7ffd,row,1);
		g->addWidget(inputs.page_c000,row,3);
		g->addWidget(new QLabel("ram $c000"), row,4);
		row++;

		if(has_1ffd) g->addWidget(new QLabel("port $1ffd"), row,0,Qt::AlignRight);
		if(has_1ffd) g->addWidget(inputs.port_1ffd,row,1);
		g->addWidget(inputs.page_8000,row,3);
		g->addWidget(new QLabel("ram $8000"), row,4);
		row++;

		g->addWidget(inputs.checkbox_mmu_locked,row,0,1,2);
		g->addWidget(inputs.page_4000,row,3);
		g->addWidget(new QLabel("ram $4000"), row,4);
		row++;

		if(has_1ffd) g->addWidget(inputs.checkbox_ram_only,row,0,1,2);
		g->addWidget(inputs.page_0000,row,3);
		g->addWidget(inputs.label_rom0000=new QLabel("rom $0000"), row,4);
		row++;

		g->addWidget(inputs.video_page,row,3);
		g->addWidget(new QLabel("video ram"), row,4);
		row++;

		if(has_1ffd)
		{
			g->addWidget(new QLabel("disc motor"), row,0,Qt::AlignRight);
			g->addWidget(inputs.disc_motor,row,1);
			g->addWidget(inputs.printer_strobe,row,3,Qt::AlignRight);
			g->addWidget(new QLabel("lprnt strobe"), row,4);
			row++;
		}
	}

// Restore values button:
// & Frame hit:

	g->addWidget(new QLabel("frames hit"), row,0,Qt::AlignRight);
	g->addWidget(inputs.frames_hit,row,1);
	btn_restore_defaults = new QPushButton("Restore defaults");
	g->addWidget(btn_restore_defaults,row,3,1,2,Qt::AlignRight);
	g->setRowStretch(row,100);
	row++;

	background = background.scaled(width,height);
	this->setFixedSize( background.size() );

	timer->start(1000/20);
}

void UlaInsp::updateWidgets()
{
	xxlogIn("UlaInsp::updateWidgets");

	if(!object) { timer->stop(); return; }
	if(!mmu) { timer->stop(); return; }

	volatile Ula* ula = this->ula();
	bool f;
	uint i;

	if(inputs.port_1ffd!=nullptr)
	{
		if(values.port_1ffd != mmu->getPort1ffd())
		{
			values.port_1ffd = mmu->getPort1ffd();
			inputs.port_1ffd->setText(catstr("$",hexstr(values.port_1ffd,2)));
		}

		volatile MmuPlus3* mmu = MmuPlus3Ptr(this->mmu);

		if(inputs.checkbox_ram_only->isChecked()!=(f=mmu->isRamOnlyMode()))
		{
			inputs.checkbox_ram_only->setChecked(f);
			inputs.label_rom0000->setText(f?"ram $0000":"rom $0000");
		}
		if(values.disc_motor != (f=mmu->getDiscMotorState()))
		{
			values.disc_motor = f;
			inputs.disc_motor->setText(values.disc_motor?"1":"0");
		}
		if(values.printer_strobe != (f=mmu->getPrinterStrobe()))
		{
			values.printer_strobe = f;
			inputs.printer_strobe->setText(values.printer_strobe?"1":"0");
		}
	}

	if(inputs.port_7ffd!=nullptr)
	{
		if(values.port_7ffd != mmu->getPort7ffd())
		{
			values.port_7ffd = mmu->getPort7ffd();
			inputs.port_7ffd->setText(catstr("$",hexstr(values.port_7ffd,2)));
		}

		volatile Mmu128k* mmu = Mmu128kPtr(this->mmu);

		if(values.page_c000 != (i=mmu->getPageC000()))
		{
			values.page_c000 = i;
			inputs.page_c000->setText(catstr("page ",tostr(values.page_c000)));
		}
		if(values.page_8000 != (i=mmu->getPage8000()))
		{
			values.page_8000 = i;
			inputs.page_8000->setText(catstr("page ",tostr(values.page_8000)));
		}
		if(values.page_4000 != (i=mmu->getPage4000()))
		{
			values.page_4000 = i;
			inputs.page_4000->setText(catstr("page ",tostr(values.page_4000)));
		}
		if(values.page_0000 != (i=mmu->getPage0000()))
		{
			values.page_0000 = i;
			inputs.page_0000->setText(catstr("page ",tostr(values.page_0000)));
		}
		if(values.video_page!= (i=mmu->getVideopage()))
		{
			values.video_page = i;
			inputs.video_page->setText(catstr("page ",tostr(values.video_page)));
		}
		if(inputs.checkbox_mmu_locked->isChecked() != (f=mmu->port7ffdIsLocked()))
		{
			inputs.checkbox_mmu_locked->setChecked(f);
		}

	}

	// if cpu_clock changes then update:
	// cpu_clock, ula_clock, cpu_clock_predivider and cpu_clock_overdrive
	//
	if(values.cpu_clock != machine->cpu_clock)
	{
		uint32 cpu_clock = values.cpu_clock = machine->cpu_clock;
		float64 overdrive = (float64)cpu_clock / machine->model_info->cpu_cycles_per_second;

		uint32 predivider = machine->model_info->cpuClockPredivider();
		for(uint o = uint(overdrive+0.2); o>1; o>>=1) { if(predivider>1) predivider >>= 1; }	// speed menu: 200%, 400%, 800%

		uint32 ula_clock = cpu_clock * predivider;
		overdrive *= 100;		// percent

		xlogline("UlaInsp: overdrive = %5g%%",		double(overdrive));
		xlogline("UlaInsp: cpu_clock = %u",		uint(cpu_clock));
		xlogline("UlaInsp: clock_predivider = %u", uint(predivider));
		xlogline("UlaInsp: ula_clock = %u",		uint(ula_clock));

		inputs.cpu_clock->setText(MHzStr(cpu_clock));

		cstr s = tostr(overdrive); if(strchr(s,'e')) s = "0.000";
		s = leftstr(s,5); if(lastchar(s)=='.') s = leftstr(s,4);
		inputs.cpu_clock_overdrive->setText(catstr(s,"%"));

		inputs.ula_clock->setText(MHzStr(ula_clock));

		inputs.cpu_clock_predivider->setText(catstr("1/",tostr(predivider)));
	}

	if(values.top_rows != uint(ula->getLinesBeforeScreen()))
	{
		values.top_rows = ula->getLinesBeforeScreen();
		inputs.top_rows->setText(tostr(values.top_rows));
		xlogline("UlaInsp: top_rows = %u", uint(values.top_rows));
	}

//    if(values.screen_rows)
//    {
//    }
//    if(values.screen_columns)
//    {
//    }

	if(values.bottom_rows != uint(ula->getLinesAfterScreen()))
	{
		values.bottom_rows = ula->getLinesAfterScreen();
		inputs.bottom_rows->setText(tostr(values.bottom_rows));
		xlogline("UlaInsp: bottom_rows = %u", uint(values.bottom_rows));
	}

	if(values.bytes_per_row != uint(ula->getBytesPerLine()))
	{
		values.bytes_per_row = ula->getBytesPerLine();
		inputs.bytes_per_row->setText(tostr(values.bytes_per_row));
		xlogline("UlaInsp: bytes_per_row = %u", uint(values.bytes_per_row));
	}

	if(values.cpu_cycles_per_row != uint(ula->getCcPerLine()))
	{
		values.cpu_cycles_per_row = ula->getCcPerLine();
		inputs.cpu_cycles_per_row->setText(tostr(values.cpu_cycles_per_row));
		xlogline("UlaInsp: cpu_cycles_per_row = %u", uint(values.cpu_cycles_per_row));
	}

	if(values.cpu_cycles_per_frame != uint(ula->getCcPerFrame()))
	{
		values.cpu_cycles_per_frame = ula->getCcPerFrame();
		inputs.cpu_cycles_per_frame->setText(tostr(values.cpu_cycles_per_frame));
		xlogline("UlaInsp: cpu_cycles_per_frame = %u", uint(values.cpu_cycles_per_frame));
	}

	if(values.frames_per_second != (float)values.cpu_clock/ula->getCcPerFrame())
	{
		values.frames_per_second = (float)values.cpu_clock/ula->getCcPerFrame();
		char* s = tempstr(15);
		sprintf ( s, "%.4g", (double)values.frames_per_second );
		inputs.frames_per_second->setText(s);
		xlogline("UlaInsp: frames_per_second = %s",s);
	}

	if(ula->isA(isa_UlaZxsp))
	{
		UlaZxsp* zxula = (UlaZxsp*)ula;


		if(zxula->hasWaitmap())
		{
			if(inputs.checkbox_enable_cpu_waitcycles->isChecked() != zxula->hasWaitmap())
			{
				inputs.checkbox_enable_cpu_waitcycles->setChecked(zxula->hasWaitmap());
			}
			if(values.waitmap_offset != uint(zxula->getWaitmapStart())-zxula->getScreenStart())
			{
				values.waitmap_offset = zxula->getWaitmapStart()-zxula->getScreenStart();
				inputs.waitmap_offset->setText(tostr(int(values.waitmap_offset)));
				xlogline("UlaInsp: waitmap_offset = %i", int(values.waitmap_offset));
			}

			if(values.waitmap != machine->model_info->waitmap)
			{
				uint8 b = values.waitmap = machine->model_info->waitmap;
				char s[16] = "0-0-0-0-0-0-0-0";
				for(int i=0;i<8;i++) { if(b&0x80) s[i+i]='1'; b+=b; }
				inputs.waitmap->setText(s);
				xlogline("UlaInsp: waitmap = %s",s);
			}
		}

		if(values.border_color != zxula->getBorderColor())
		{
			uint b = values.border_color = zxula->getBorderColor();
		//  inputs.border_color->setAutoFillBackground(true);
			cstr s = usingstr("QLineEdit { background-color: rgba(%i,%i,%i); }",b&2?222:22,b&4?222:22,b&1?222:22);
			inputs.border_color->setStyleSheet(s);
			inputs.border_color->setText(tostr(values.border_color));
			xxlogline("UlaInsp: border_color = %u",b);
		}

		if( values.mic_bit != zxula->getMicOutState() )
		{
			values.mic_bit = zxula->getMicOutState();
			cstr s = values.mic_bit ? "1" : "0";
			inputs.mic_bit->setText(s);
			xxlogline("UlaInsp: mic_bit = %s",s);
		}

		if(values.ear_bit != zxula->getEarOutState())
		{
			values.ear_bit = zxula->getEarOutState();
			cstr s = values.ear_bit ? "1" : "0";
			inputs.ear_bit->setText(s);
			xxlogline("UlaInsp: ear_bit = %s",s);
		}
	}

	if(values.frames_hit != machine->controller->getScreen()->getFramesHit())
	{
		values.frames_hit = machine->controller->getScreen()->getFramesHit();
		inputs.frames_hit->setText(catstr(tostr(values.frames_hit),"%"));
	}
}










