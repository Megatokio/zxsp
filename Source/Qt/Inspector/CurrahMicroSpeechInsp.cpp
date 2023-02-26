// Copyright (c) 2015 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "CurrahMicroSpeechInsp.h"
#include "CurrahMicroSpeech.h"
#include "Qt/qt_util.h"
#include "cpp/cppthreads.h"
#include <QPainter>
#include <QRadioButton>
#include <QTimer>

namespace gui
{

/*	Allophone names for the AL2 rom:
 */
static QString names[128] = {
	"",		//	pause
	" ",	//	pause
	"  ",	//	pause
	"   ",	//	pause
	"    ", //	pause

	//	µSpeech		SP0256
	"(oy)",	 //	oy
	"(ii)",	 //	ay
	"e",	 //	eh
	"c",	 //	kk3
	"p",	 //	pp
	"j",	 //	jh
	"n",	 //	nn1
	"i",	 //	ih
	"(tt)",	 //	tt2
	"(rr)",	 //	rr1
	"u",	 //	ax
	"m",	 //	mm
	"t",	 //	tt1
	"(dth)", //	dh1
	"(ee)",	 //	iy
	"(ay)",	 //	ey		(aa), (ay)
	"d",	 //	dd1
	"(ou)",	 //	uw1
	"o",	 //	ao
	"a",	 //	aa
	"(yy)",	 //	yy2
	"(eh)",	 //	ae
	"h",	 //	hh1
	"b",	 //	bb1
	"(th)",	 //	th
	"(uh)",	 //	uh
	"(ouu)", //	uw2
	"(ow)",	 //	aw
	"(dd)",	 //	dd2
	"(ggg)", //	gg3
	"v",	 //	vv
	"g",	 //	gg1
	"(sh)",	 //	sh
	"(zh)",	 //	zh
	"r",	 //	rr2
	"f",	 //	ff
	"(ck)",	 //	kk2		(ck), (gg)
	"k",	 //	kk1
	"z",	 //	zz
	"(ng)",	 //	ng
	"l",	 //	ll
	"w",	 //	ww
	"(aer)", //	xr
	"(wh)",	 //	wh
	"y",	 //	yy1
	"(ch)",	 //	ch
	"(er)",	 //	er1
	"(err)", //	er2
	"(eau)", //	ow		(oo), (eau)
	"dh2",	 //	dh2		missing
	"s",	 //	ss
	"(nn)",	 //	nn2
	"(hh)",	 //	hh2
	"(or)",	 //	or
	"(ar)",	 //	ar
	"(ear)", //	yr
	"gg2",	 //	gg2		missing
	"(ll)",	 //	el
	"(bb)",	 //	bb2

	"",		//	pause
	" ",	//	pause
	"  ",	//	pause
	"   ",	//	pause
	"    ", //	pause

	//	µSpeech		SP0256
	"(OY)",	 //	OY
	"(II)",	 //	AY
	"E",	 //	EH
	"C",	 //	KK3
	"P",	 //	PP
	"J",	 //	JH
	"N",	 //	NN1
	"I",	 //	IH
	"(TT)",	 //	TT2
	"(RR)",	 //	RR1
	"U",	 //	AX
	"M",	 //	MM
	"T",	 //	TT1
	"(DTH)", //	DH1
	"(EE)",	 //	IY
	"(AY)",	 //	EY		(AA), (AY)
	"D",	 //	DD1
	"(OU)",	 //	UW1
	"O",	 //	AO
	"A",	 //	AA
	"(YY)",	 //	YY2
	"(EH)",	 //	AE
	"H",	 //	HH1
	"B",	 //	BB1
	"(TH)",	 //	TH
	"(UH)",	 //	UH
	"(OUU)", //	UW2
	"(OW)",	 //	AW
	"(DD)",	 //	DD2
	"(GGG)", //	GG3
	"V",	 //	VV
	"G",	 //	GG1
	"(SH)",	 //	SH
	"(ZH)",	 //	ZH
	"R",	 //	RR2
	"F",	 //	FF
	"(CK)",	 //	KK2		(CK), (GG)
	"K",	 //	KK1
	"Z",	 //	ZZ
	"(NG)",	 //	NG
	"L",	 //	LL
	"W",	 //	WW
	"(AER)", //	XR
	"(WH)",	 //	WH
	"Y",	 //	YY1
	"(CH)",	 //	CH
	"(ER)",	 //	ER1
	"(ERR)", //	ER2
	"(EAU)", //	OW		(OO), (EAU)
	"DH2",	 //	DH2		MISSING
	"S",	 //	SS
	"(NN)",	 //	NN2
	"(HH)",	 //	HH2
	"(OR)",	 //	OR
	"(AR)",	 //	AR
	"(EAR)", //	YR
	"GG2",	 //	GG2		MISSING
	"(LL)",	 //	EL
	"(BB)",	 //	BB2
};

static uint8 widths[256];
static bool	 initialized = no;


// "Monaco",12
// "Andale Mono",13
// "Lucida Grande",11
// "Arial",10
static QFont scrollfont = QFont("Lucida Grande", 11);

#define spacing 2u
#define max_gap 20u


CurrahMicroSpeechInsp::CurrahMicroSpeechInsp(
	QWidget* parent, MachineController* mc, volatile CurrahMicroSpeech* uspeech) :
	Inspector(parent, mc, uspeech, "Images/currah_microspeech.jpg"),
	uspeech(uspeech),
	rp(0),
	wp(0),
	xpos(0),
	width(0)
{
	button_8bit = new QRadioButton("8 Bit", this);
	button_hifi = new QRadioButton("Hifi", this);

	button_8bit->move(8, 10);
	button_hifi->move(8, 30);

	//	const QRgb fore = 0xffffffff;
	//	const QRgb back = 0;
	//	setColors(button8bit, fore, back);
	//	setColors(buttonHifi, fore, back);

	bool hifi = uspeech->isHifi();
	button_8bit->setChecked(!hifi);
	button_hifi->setChecked(hifi);

	connect(button_8bit, &QRadioButton::clicked, this, [this] {
		assert(validReference(this->uspeech));
		NV(this->uspeech)->setHifi(no);
	});
	connect(button_hifi, &QRadioButton::clicked, this, [this] {
		assert(validReference(this->uspeech));
		NV(this->uspeech)->setHifi(yes);
	});

	// precalculate print widths of allophone names:
	if (!initialized)
	{
		QPainter painter(this);
		painter.setFont(scrollfont);
		QFontMetrics m = painter.fontMetrics();
		for (uint i = 0; i < 128; i++)
		{
#if QT_VERSION < 0x050b00
			widths[i] = spacing + uint8(m.width(names[i]));
#else
			widths[i] = spacing + uint8(m.horizontalAdvance(names[i]));
#endif
		}
		for (uint i = 128; i < 256; i++) { widths[i] = uint8(i - 128); }
		initialized = yes;
	}

	for (uint i = 0; i < 10; i++) { scroller[wp++] = 0xA0; }
	width = 320;

	timer->start(1000 / 60);
}

void CurrahMicroSpeechInsp::updateWidgets()
{
	xxlogIn("CurrahMicroSpeechInspector::update");
	assert(validReference(this->uspeech));

	// add spoken allophones:

	const uint hmask = NELEM(uspeech->history) - 1;
	const uint smask = NELEM(scroller) - 1;

	while (uspeech->lastrp != uspeech->lastwp)
	{
		uint8 command = uspeech->history[uspeech->lastrp++ & hmask];
		if (command > 0x80 + max_gap) command = 0x80 + max_gap;
		scroller[(wp++) & smask] = command;
		width += widths[command];
	}

	// scroll the scroller, if needed:
	if (xpos + width > 320)
	{
		xpos -= (xpos + width - 320 + 59) / 30;
		int firstwidth = widths[scroller[rp & smask]];
		if (xpos <= -firstwidth)
		{
			rp++;
			xpos += firstwidth;
			width -= firstwidth;
		}
		update(0, 220, 320, 20);
	}
}

void CurrahMicroSpeechInsp::paintEvent(QPaintEvent* e)
{
	Inspector::paintEvent(e);

	QPainter p(this);
	p.setFont(scrollfont);

	int		   ypos = 235;
	int		   xpos = this->xpos;
	const uint mask = NELEM(scroller) - 1;

	for (uint i = rp; i < wp; i++)
	{
		uint a = scroller[i & mask]; // allophone number

		if (a < 128 && xpos < 320) // allophone, not pause
		{
			p.drawText(xpos, ypos, names[a]);
		}
		xpos += widths[a];
	}
}

} // namespace gui


/*
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
*/
