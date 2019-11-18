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

#include <QtGui>
#include "AyInsp.h"
#include "Ay/Ay.h"
#include <QLineEdit>
#include <QLabel>
#include <QGridLayout>
#include "Uni/util.h"
#include "Machine.h"
#include "Templates/NVPtr.h"


static cstr es[] =
{
	"\\＿＿＿",     "\\＿＿＿","\\＿＿＿",   "\\＿＿＿",
	"/＿＿＿",      "/＿＿＿", "/＿＿＿",    "/＿＿＿",
	"\\\\\\\\\\\\","\\＿＿＿","\\/\\/\\/", "\\￣￣￣",
	"//////",      "/￣￣￣", "/\\/\\/\\", "/＿＿＿"
};

static const QFont ff("Monaco"/*"Andale Mono"*/, 12);


AyInsp::AyInsp(QWidget* w, MachineController* mc, volatile IsaObject* item )
:
	Inspector(w,mc,item,"/Backgrounds/light-150-s.jpg"),
	value{0,0,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}}
{
	assert( item->isA(isa_Ay) );

	clock   = new_led("0.0 MHz");
	pitch_a = new_led("0000");
	pitch_b = new_led("0000");
	pitch_c = new_led("0000");
	pitch_n = new_led("0000");
	mixer   = new_led("--------");
	vol_a   = new_led("00");
	vol_b   = new_led("00");
	vol_c   = new_led("00");
	pitch_e = new_led("0000");
	shape_e = new_led("----");
	port_a  = new_led("$00");
	port_b  = new_led("$00");

	stereo = new QComboBox(this);
	stereo->setFocusPolicy(Qt::NoFocus);
	stereo->addItem("Mono");				// Reihenfolge muss Ay::StereoMix entsprechen!
	stereo->addItem("ABC Stereo - Western Europe");
	stereo->addItem("ACB Stereo - Eastern Europe");
	value.stereo = ay()->getStereoMix();
	stereo->setCurrentIndex(value.stereo);
	connect(stereo, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
			this, [=](int i){ ay()->setStereoMix(Ay::StereoMix(i)); });

	QGridLayout* g = new QGridLayout(this);
	g->setContentsMargins(10,10,10,10);
	g->setVerticalSpacing(4);
	g->setRowStretch(13,100);
	g->setColumnStretch(0,33);
	g->setColumnStretch(1,66);

	g->addWidget(new QLabel("Clock"),     0,0,Qt::AlignRight);	g->addWidget(clock,0,1);
	g->addWidget(new QLabel("A pitch"),   1,0,Qt::AlignRight);	g->addWidget(pitch_a,1,1);
	g->addWidget(new QLabel("B pitch"),   2,0,Qt::AlignRight);	g->addWidget(pitch_b,2,1);
	g->addWidget(new QLabel("C pitch"),   3,0,Qt::AlignRight);	g->addWidget(pitch_c,3,1);
	g->addWidget(new QLabel("Noise pi."), 4,0,Qt::AlignRight);	g->addWidget(pitch_n,4,1);
	g->addWidget(new QLabel("Mixer"),     5,0,Qt::AlignRight);	g->addWidget(mixer,5,1);
	g->addWidget(new QLabel("A volume"),  6,0,Qt::AlignRight);	g->addWidget(vol_a,6,1);
	g->addWidget(new QLabel("B volume"),  7,0,Qt::AlignRight);	g->addWidget(vol_b,7,1);
	g->addWidget(new QLabel("C volume"),  8,0,Qt::AlignRight);	g->addWidget(vol_c,8,1);
	g->addWidget(new QLabel("Env pitch"), 9,0,Qt::AlignRight);	g->addWidget(pitch_e,9,1);
	g->addWidget(new QLabel("Env shape"),10,0,Qt::AlignRight);	g->addWidget(shape_e,10,1);
	g->addWidget(new QLabel("Port A"),   11,0,Qt::AlignRight);	g->addWidget(port_a,11,1);
	g->addWidget(new QLabel("Port B"),   12,0,Qt::AlignRight);	g->addWidget(port_b,12,1);

	g->addWidget(stereo,13,1);

	timer->start(1000/20);
}

void AyInsp::updateWidgets()
{
	xxlogIn("AyInsp::update");

	if(!object) { timer->stop(); return; }
	volatile Ay* ay = this->ay();
	uint8 volatile const* regs = ay->getRegisters();

	if( value.clock != ay->getClock() && !clock->hasFocus() )
	{
		value.clock = ay->getClock();
		clock->setText(MHzStr(value.clock));
	}

	if( value.stereo!= ay->getStereoMix() )
	{
		value.stereo = ay->getStereoMix();
		stereo->setCurrentIndex(value.stereo);
	}

	for(int i=0;i<16;i++)
	{
		if(value.regs[i]==regs[i]) continue;
		value.regs[i] = regs[i];
		switch(i)
		{
		case 0: if(!pitch_a->hasFocus()) value.regs[1] = regs[1]; i++;
		case 1:	if(!pitch_a->hasFocus()) pitch_a->setText(tostr(value.regs[0]+256*value.regs[1])); break;
		case 2:	if(!pitch_b->hasFocus()) value.regs[3] = regs[3]; i++;
		case 3: if(!pitch_b->hasFocus()) pitch_b->setText(tostr(value.regs[2]+256*value.regs[3])); break;
		case 4:	if(!pitch_c->hasFocus()) value.regs[5] = regs[5]; i++;
		case 5: if(!pitch_c->hasFocus()) pitch_c->setText(tostr(value.regs[4]+256*value.regs[5])); break;
		case 6: if(!pitch_n->hasFocus()) pitch_n->setText(tostr(value.regs[6])); break;
		case 7: if(!mixer->hasFocus())   mixer->setText(binstr(value.regs[7],"iicbaCBA","oo------")); break;
		case 8: if(!vol_a->hasFocus())	 vol_a->setText(value.regs[8]&0x10?"Envelope":tostr(value.regs[8])); break;
		case 9: if(!vol_b->hasFocus())	 vol_b->setText(value.regs[9]&0x10?"Envelope":tostr(value.regs[9])); break;
		case 10:if(!vol_c->hasFocus())	 vol_c->setText(value.regs[10]&0x10?"Envelope":tostr(value.regs[10])); break;
		case 11:if(!pitch_e->hasFocus()) value.regs[12] = regs[12]; i++;
		case 12:if(!pitch_e->hasFocus()) pitch_e->setText(tostr(value.regs[11]+256u*value.regs[12])); break;
		case 13:if(!shape_e->hasFocus()) shape_e->setText(es[value.regs[13]&0x0f]); break;
		case 14:if(!port_a->hasFocus())	 port_a->setText(catstr("$",hexstr(value.regs[14],2))); break;	// value written. TODO: read?
		case 15:if(!port_b->hasFocus())	 port_b->setText(catstr("$",hexstr(value.regs[15],2))); break;	// value written. TODO: read?
		}
	}
}

QLineEdit* AyInsp::new_led( cstr s )
{
	QLineEdit* e = new QLineEdit(s);
	e->setAlignment(Qt::AlignHCenter);
	e->setFrame(0);
	e->setFont(ff);
	connect(e, &QLineEdit::returnPressed, this, [=]{handle_return_in_led(e);});
	return e;
}

void AyInsp::set_register(uint r, uint n)
{
	NVPtr<Ay>(ay())->setRegister(r,n);
	value.regs[r] = n;
}

static int32 int_value_for_volume( cstr s )
{
	if ((s[0]|0x20)=='e') return 16;	// envelope
	else return intValue(s) & 0x0f;
}

static int32 int_value_for_envelope_shape( cstr e )
{
	//	Bestimme Hüllkurvennummer aus Buchstabengrafik
	//	Signifikant sind die ersten beiden Buchstaben

	int n = strlen(e);

	if(n>=4)
	{
		cstr s = leftstr(e,4);
		if(eq(s,"\\＿"))	return 9;		// ＿ = 3 bytes in utf-8
		if(eq(s,"\\–"))	return 11;		// 1.5 long dash
		if(eq(s,"\\—"))	return 11;		// 2.0 long dash
		if(eq(s,"\\￣"))	return 11;		// ￣ = 3 bytes in utf-8
		if(eq(s,"/–"))	return 13;		// 1.5 long dash
		if(eq(s,"/—"))	return 13;		// 2.0 long dash
		if(eq(s,"/￣"))	return 13;
		if(eq(s,"/＿"))	return 15;
	}

	if(n>=2)
	{
		cstr s = leftstr(e,2);
		if(eq(s,"\\\\"))return 8;
		if(eq(s,"\\_"))	return 9;
		if(eq(s,"\\/"))	return 10;
		if(eq(s,"\\-"))	return 11;
		if(eq(s,"//"))	return 12;
		if(eq(s,"/-"))	return 13;
		if(eq(s,"/\\"))	return 14;
		if(eq(s,"/_"))	return 15;
	}

	return intValue(e) & 0x0f;
}

void AyInsp::handle_return_in_led(QLineEdit* led)
{
	//	Eingabe in einem QLineEdit wurde mit 'Return' beendet:
	//	must only be called from AyInspector's QLineEdits

	cstr text = led->text().toUtf8().data();
	int n;
	int r;

	if(led==pitch_e) { n = intValue(text) & 0xffff; r = 11; goto pe; }
	if(led==pitch_b) { r = 2; goto pa; }
	if(led==pitch_c) { r = 4; goto pa; }
	if(led==pitch_a)
	{
		r = 0;
pa:		n = intValue(text) & 0x0fff;
pe:		set_register(r,n%256); r++;
		set_register(r,n/256);
		led->setText(tostr(n));
	}

	if(led==pitch_n)
	{
		n = intValue(text) & 0x001f;
		set_register(6,n);
		led->setText(tostr(n));
	}

	if(led==vol_b) { r = 9;  goto va; }
	if(led==vol_c) { r = 10; goto va; }
	if(led==vol_a)
	{
		r = 8;
va:		n = int_value_for_volume(text);
		set_register(r,n);
		led->setText(n&0x10?"Envelope":tostr(n));
	}

	if(led==port_b) { r=15; goto oa; }
	if(led==port_a)
	{
		r = 14;
oa:		n = intValue(text) & 0x00ff;
		set_register(r,n);
		led->setText(catstr("$",hexstr(uint(n),2)));
	}

	if(led==shape_e)
	{
		n = int_value_for_envelope_shape(text);
		set_register(13,n);
		led->setText(es[n]);
	}

	if(led==mixer)		// "ooabcABC"
	{
		uint n    = ay()->getRegister(7) | 0x3F;
		uint len  = strlen(text);
		uint port = 0x40;

	//  scan text for ooabcABC
	//  abc = enable noise on channel
	//  ABC = enable channel
	//		if any of abcABC is not encountered, then that channel is disabled
	//  i/o = set port to input/output.
	//		rightmost / first encountered 'i' or 'o' is for port A
	//		leftmost / second encountered 'i' or 'o' is for port B
	//		if no setting for port A or B is encountered, old state is preserved

		for( int i=len-1; i>=0; i-- )
		{
			char c = text[i];
			if (c=='A') n &= ~0x01;			// '0' == enabled
			if (c=='B') n &= ~0x02;
			if (c=='C') n &= ~0x04;
			if (c=='a') n &= ~0x08;
			if (c=='b') n &= ~0x10;
			if (c=='c') n &= ~0x20;
			if (c=='i'||c=='I') { n &= ~port; port*=2; }	// '0' == input
			if (c=='o'||c=='O') { n |=  port; port*=2; }
		}

		set_register(7,n);
		led->setText(binstr(n,"iicbaCBA","oo------"));
	}

	if(led==clock)
	{
		n = mhzValue(text);
		if(n>0) limit(1000000,n,4000000);
		else n = ay()->getClock();
		NVPtr<Ay>(ay())->setClock(n);
		led->setText(MHzStr(n));
		value.clock = n;
	}
}









