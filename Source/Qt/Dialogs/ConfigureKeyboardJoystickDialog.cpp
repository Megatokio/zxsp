/*	Copyright  (c)	Günter Woigk 2016 - 2019
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
#include "ConfigureKeyboardJoystickDialog.h"
#include "MachineController.h"
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QLabel>
#include "qt_util.h"
#include <QPainter>
#include <QPainterPath>
#include <QPolygon>
#include "Templates/StrArray.h"
#include "Settings.h"
#include "unix/files.h"
#include <fnmatch.h>
#include <QKeyEvent>


// =================================================
//			map of key caps for os key codes
// =================================================

typedef char capstr[16];

static
capstr caps[128] =
{
	"A",	"S",	"D",	"F",	"H",	"G",	"Z",	"X",	// 0x00-0x07: a  s  d  f  h  g  z  x
	"C",	"V",	"^",	"B",	"Q",	"W",	"E",	"R",	// 0x08-0x0F: c  v  ^  b  q  w  e  r
	"Y",	"T",	"1",	"2",	"3",	"4",	"6",	"5",	// 0x10-0x17: y  t  1  2  3  4  6  5
	"•",	"9",	"7",	"•", 	"8",	"0",	"•", 	"O",	// 0x18-0x1F: •  9  7  •  8  0  •  O
	"U",	"•", 	"I",	"P",	"enter","L",	"J",	"x",	// 0x20-0x27: u  •  i  p ENT l  j  •
	"K",	";",	"x",	",",	"•", 	"N",	"M",	".",	// 0x28-0x2F: k  ;  •  ,  •  n  m  .
	"•", 	"space","<",	"del",	"•", 	"esc",	"•", 	"•", 	// 0x30-0x37: • SPC < DEL • ESC •  •
	"shift","•",    "alt",	"ctrl",	"shift","alt",	"ctrl",	"•", 	// 0x38-0x3F: shift • alt ctrl shift2 alt2 ctrl2 •
	"•", 	"[.]",	"•",	"[*]",	"•",	"[+]",	"•",	"[num]", // 0x40-0x47: • [.] • [*] • [+] • [num]
	"•",	"•",	"•",	"[/]",	"[enter]","•",	"[-]",	"•",	// 0x48-0x4F: • • • [/] [enter] • [-] •
	"•",	"[=]",	"[0]",	"[1]",	"[2]",	"[3]",	"[4]",	"[5]",	// 0x50-0x57: • [=] [0] [1] [2] [3] [4] [5]
	"[6]",	"[7]",	"•",	"[8]",	"[9]",	"•",	"•",	"•",	// 0x58-0x5F: [6] [7] • [8] [9] • • •
	"•",	"•",	"F7",	"F3",	"F8",	"F9",	"•",	"F11",	// 0x60-0x67:
	"•",	"F13",	"•",	"F14",	"•",	"F10",	"•",	"F12",	// 0x68-0x6F:
	"•",	"•",	"•",	"•",	"•",	"•",	"F4",	"•",	// 0x70-0x77:
	"F2",	"F15",	"F1",	"left",	"right","down",	"up",	"",		// 0x78-0x7F: •  •  • left right down up •
};

#if SECAPS
static bool caps_loaded = false;
static bool caps_modified = false;

static void read_caps();
static void write_caps();

struct KJ1 { ~KJ1() { if(caps_modified) write_caps(); } };
static KJ1 kj1;		// trigger write on exit


/*	write key caps data base to settings
	each key cap is encoded like this:
		defb	'a' + strlen	typ. 1, at most 15
		defm	"text"
*/
void write_caps()
{
	char s[sizeof(caps)+1];		// buffer
	ptr  z = s;					// dest ptr

	for(uint key=0; key<NELEM(caps); key++)
	{
		cptr q = caps[key];
		uint l = strnlen(q,sizeof(capstr)-1);
		*z++ = 'a' + l;			// store strlen
		memcpy(z,q,l);			// store text
		z += l;
	}

	*z = 0;

	settings.setValue(key_os_key_code_to_key_caps, s);

	caps_modified = false;
}

void read_caps()
{
	return;

	cstr q = settings.get_cstr(key_os_key_code_to_key_caps, "");
	cptr e = strchr(q,0);

	for(uint key=0; key<NELEM(caps); key++)
	{
		ptr  z = caps[key];
		uint l = *q++ - 'a';

		if(l >= sizeof(capstr)) break;	// data corrupted or ""
		if(q+l > e) break;				// data truncated

		memcpy(z,q,l);
		z[l] = 0;
		q += l;
	}

	caps_loaded = true;
}

void addKeyCap(uint8 key, cstr cap)
{
	if(!cap || *cap<=' ' || *cap==0x7F) return;
	assert(strlen(cap) < sizeof(capstr));

	key &= 0x7F;
	cap = upperstr(cap);

	if(!caps_loaded) read_caps();

	strncpy(caps[key], cap, sizeof(capstr));
	caps_modified = true;
}
#endif


cstr getKeyCap(uint8 key)
{
#if SECAPS
	if(!caps_loaded) read_caps();
#endif
	return caps[key&0x7F];
}




// ======================================================
//	fnmatch patterns -> kbd joystick keys (os key codes)
// ======================================================


static StrArray patterns;
static bool patterns_modified;

static void	read_patterns()
{
	settings.get_StrArray(key_kbd_joystick_fnmatch_patterns,patterns);
	for(uint i=patterns.count(); i--;) { if(strlen(patterns[i])<=10)
		patterns.remove(i); }
	patterns_modified = no;
	#if XLOG
		logIn("Stored KeyJoy Fnmatch patterns:");
		for(uint i=0;i<patterns.count();i++) logline("%s",patterns[i]);
	#endif
}

static void	write_patterns()
{
	settings.set_StrArray(key_kbd_joystick_fnmatch_patterns,patterns); patterns_modified = no;
}

struct KJ2 { ~KJ2() { if(patterns_modified) write_patterns(); } };
static KJ2 kj2;		// trigger write on exit

static//helper
void store_byte(ptr s, uint byte)
{
	s[0] = hexchar(byte>>4);
	s[1] = hexchar(byte);
}


/*	add a filename pattern with kbd joystick keys to the data base
	each data base entry is a concatenation as this:
		"UUDDLLRRFF" + fnpattern
		 ^ the key codes for the 5 buttons encoded as 2 hex chars each
	returns the persistent, actually stored pattern.
*/
cstr addKeyJoyFnmatchPattern(uint8 keys[5], cstr fnpattern)
{
	assert(fnpattern && *fnpattern);	// not empty
	fnpattern = lowerstr(fnpattern);

	if(patterns.count()==0) read_patterns();
	patterns_modified = yes;

	for(uint i=patterns.count();i--;)
	{
		str s = patterns[i];
		if(eq(fnpattern, s+10))
		{
			for(i=0;i<5;i++) store_byte(s+2*i,keys[i]);
			return s+10;
		}
	}

	str s = catstr("1234567890",fnpattern);
	for(uint i=0;i<5;i++) store_byte(s+2*i,keys[i]);
	patterns.append(s);
	return patterns.last()+10;
}


/*	retrieve kbd joystick keys for filename from data base
	returns fnmatch pattern used
*/
cstr getKeyJoyFnmatchPattern(uint8 keys[5], cstr filename)
{
	filename = filename ? lowerstr(basename_from_path(filename)) : "";

	if(patterns.count()==0) read_patterns();

	cstr s0 = "FFFFFFFFFF*";		// if nothing else matches
	uint l0 = 0;					// strlen(s0+10)

	for(int i=patterns.count(); i--;)
	{
		cstr s = patterns[i];
		uint l = strlen(s+10);

		if(l > l0 && fnmatch(s+10, filename, FNM_NOESCAPE)==0)
		{
			s0 = s; l0 = l;
			if(eq(filename,s+10)) break;	// exact match
		}
	}

	for(uint i=0;i<5;i++)
	{
		keys[i] = (hex_digit_value(s0[2*i])<<4) + hex_digit_value(s0[2*i+1]);
	}

	return s0+10;
}





// ======================================================
//		the Configure Keyboard Joystick Dialog
// ======================================================


#define WIDTH	(16*16)
#define HEIGHT	(16*22)
const int lw = 40;



class KbdLed : public QLineEdit
{
	ConfigureKeyboardJoystickDialog* dialog;
	uint	idx;

public:
			KbdLed(ConfigureKeyboardJoystickDialog* parent, uint idx);
	void	keyPressEvent(QKeyEvent*);
	void	focusInEvent(QFocusEvent*);
	bool	updateState();
};

#define LED_UNSET		0xFFccffff
#define LED_COLLISSION	0xFFff6666
#define LED_OK			0xFF66ff66


KbdLed::KbdLed(ConfigureKeyboardJoystickDialog* p, uint idx)
:	QLineEdit(p),
	dialog(p),
	idx(idx)
{
	setFocusPolicy(Qt::StrongFocus);	// TAB und CLICK
	setAlignment(Qt::AlignHCenter);
	setFixedWidth(lw);
	setColors(this,0xff000000,LED_UNSET);
}

//	update text and colors
//	returns: ok?
//
bool KbdLed::updateState()
{
	uint mykey = dialog->new_keys[idx];

	setText(getKeyCap(mykey));

	if(mykey == 0xFF)
	{ setColors(this,0xff000000,LED_UNSET); return no; }

	for(uint i=0;i<5;i++)
	{
		if(i!=idx && dialog->new_keys[i]==mykey)
		{ setColors(this,0xff000000,LED_COLLISSION); return no; }
	}

	setColors(this,0xff000000,LED_OK); return yes;
}


void KbdLed::keyPressEvent(QKeyEvent* e)
{
/*
	e.key()                 Großbuchstabe der Taste, wenn kein Modifierkey gedrückt wäre
	e.modifiers()           Maske aller gedrückter Modifier
								Qt::SHIFT   = Shift-Taste
								Qt::META    = Control-Taste
								Qt::CTRL    = Cmd-Taste
								Qt::ALT     = Alt-Taste
	e.nativeModifiers()     OSX-Modifiermaske, außer wenn Modifiertaste alleine gedrückt ist, dann 0
	e.nativeScanCode()      nutzlos: 0 oder 1
	e.nativeVirtualKey()    OSX-Keycode, außer wenn Modifiertaste alleine gedrückt ist, dann 0
							ACHTUNG: Der Tastencode für 'A' ist auch 0!
	e.text()                Resultierendes druckbares Zeichen, außer bei Controlcodes, CTRL+Taste und CMD+Taste: leer
*/
	if(e->isAutoRepeat()) return;

	int		key			= e->key();                 // Taste ohne Modifier, Uppercase
	uint32	modifiers	= e->modifiers();           // Modifier-Maske
	uint8	keycode		= e->nativeVirtualKey();	// OSX Tastencode
	QString	text		= e->text();                // Resultierendes Zeichen
	uint16  unicode		= text.count()==0 ? 0 : text.at(0).unicode();

	xlogline("key:        0x%08x (%i)", uint(key), int(key));
	xlogline("modifiers:  0x%08x", uint(modifiers));
	xlogline("native key: 0x%08x", uint(keycode));
	xlogline("text:       %s",     text.toUtf8().data());

	if((modifiers&Qt::CTRL) || key==Qt::Key_Control) return;							// Cmd key involved?
	if(modifiers & ~Qt::KeypadModifier) { xlogline("Modifiers!"); return; }				// Modifiers involved?
	if(keycode==0 && unicode!='A' && unicode!='a') { xlogline("Keycode=0"); return; }	// ?
	if(keycode>=NELEM(caps)) { xlogline("Keycode≥128"); return; }						// ?

#if SECAPS
	cstr s = text.toUtf8().data();
	if(s && *s > ' ' && *s!=0x7F)
	{
		if(modifiers & Qt::KeypadModifier) s = catstr("[",s,"]");
		addKeyCap(keycode,s);
	}
#endif

	dialog->new_keys[idx] = keycode;
	this->focusNextChild();
}

void KbdLed::focusInEvent(QFocusEvent* e)
{
	dialog->led_up->updateState();
	dialog->led_down->updateState();
	dialog->led_left->updateState();
	dialog->led_right->updateState();
	dialog->led_fire->updateState();

	this->selectAll();
	QLineEdit::focusInEvent(e);
}


class KbdFnLed : public QLineEdit
{
	ConfigureKeyboardJoystickDialog* dialog;

public:
		KbdFnLed(ConfigureKeyboardJoystickDialog* p) :QLineEdit(p),dialog(p){}
		void focusInEvent(QFocusEvent* e);
};

void KbdFnLed::focusInEvent(QFocusEvent* e)
{
	dialog->btn_for_match_pattern->setChecked(true);
	QLineEdit::focusInEvent(e);
}



ConfigureKeyboardJoystickDialog::ConfigureKeyboardJoystickDialog(MachineController* mc)
:
	ConfigDialog(mc, WIDTH, HEIGHT, ConfigDialog::DefaultStyle),
	led_filenamepattern(new KbdFnLed(this)),
	btn_default_for_all_files(new QRadioButton("Default for all files",this)),
	btn_for_match_pattern(new QRadioButton("For file pattern",this)),
	btn_use_just_now(new QRadioButton("Use just now",this)),
	led_up(new KbdLed(this,3)),
	led_down(new KbdLed(this,2)),
	led_left(new KbdLed(this,1)),
	led_right(new KbdLed(this,0)),
	led_fire(new KbdLed(this,4)),
	btn_cancel(new QPushButton("Cancel",this)),
	btn_ok(new QPushButton("OK",this)),
	old_matchpattern(NULL)
{
// Layout:
	int x=x0;
	int y=y0;

	QFont bigfont = QFont("Lucida Grande",16);
	QLabel* hl = new QLabel("Keyboard Joystick Configuration",this);
	setColors(hl,0xffffff);
	hl->setFont(bigfont);
	hl->move(x,y);

	y+=20;
	QFont smallfont = QFont("Lucida Grande",12);
	QLabel* info = new QLabel("Note: keys used for the keyboard joystick\n"
							  "may appear dead on the machine",this);
	setColors(info,0xffffff);
	info->setFont(smallfont);
	info->move(x,y);
	y+=16;

	// checkboxes:
	y+=20;
	btn_default_for_all_files->move(x,y);
	setColors(btn_default_for_all_files,0xffffff);

	y+=20;
	btn_for_match_pattern->move(x,y);
	setColors(btn_for_match_pattern,0xffffff);
	led_filenamepattern->move(x+WIDTH/2,y);
	led_filenamepattern->setFixedWidth(x+WIDTH-led_filenamepattern->x());

	y+=20;
	btn_use_just_now->move(x,y);
	setColors(btn_use_just_now,0xffffff);

	// joystick cross:
	y+=24;
	xm = x + WIDTH/2;
	ym = y + 6*16;
	led_up->move(xm-lw/2,ym-6*16);
	led_left->move(xm-7*16,ym-10);
	led_right->move(xm+7*16-lw,ym-10);
	led_down->move(xm-lw/2,ym+5*16-4);
	led_fire->move(xm-6*16,ym+3*16-4);

	// buttons at bottom:
	y += 13*16;
	const int bw = 80;
	btn_ok->setFixedWidth(bw);
	btn_cancel->setFixedWidth(bw);
	btn_ok->move(x+WIDTH-bw,y);
	btn_cancel->move(x+WIDTH-2*bw,y); y+=16;

// setup values:

	led_filenamepattern->setFocusPolicy(Qt::ClickFocus);	// only CLICK not TAB
	connect(led_filenamepattern,&QLineEdit::textEdited,[](){});

	memcpy(old_keys,mc->keyjoy_keys,sizeof(old_keys));
	memcpy(new_keys,mc->keyjoy_keys,sizeof(new_keys));
	old_matchpattern = newcopy(mc->keyjoy_fnmatch_pattern);

	if(!eq(old_matchpattern,"") && !eq(old_matchpattern,"*"))	// have fnpattern => use it
		led_filenamepattern->setText(old_matchpattern);
	else if(!eq(mc->filepath,""))								// have filename => use it
		led_filenamepattern->setText(lowerstr(basename_from_path(mc->filepath)));
//	else														// have nothing
//		led_filenamepattern->setText("");

	if(old_matchpattern==NULL)	// => der User hat bei diesem Snapshot schon mal explizit "just now" angewählt
		btn_use_just_now->setChecked(true);
	if(eq(old_matchpattern,"*"))
		btn_default_for_all_files->setChecked(true);
	else
		btn_for_match_pattern->setChecked(true);

	led_up->updateState();
	led_down->updateState();
	led_left->updateState();
	led_right->updateState();
	led_fire->updateState();

	connect(btn_cancel, &QPushButton::clicked, this, &QWidget::deleteLater);

	connect(btn_ok, &QPushButton::clicked, [this,mc]
	{
		if(btn_use_just_now->isChecked())
		{
			// use keys for current machine
a:			memcpy(mc->keyjoy_keys, new_keys, sizeof(mc->keyjoy_keys));
			mc->keyjoy_fnmatch_pattern = NULL;
		}
		else if(btn_default_for_all_files->isChecked())
		{
			// store fnpattern and keys into settings:
			addKeyJoyFnmatchPattern(new_keys,"*");

			// use keys for current machine
			memcpy(mc->keyjoy_keys, new_keys, sizeof(mc->keyjoy_keys));
			mc->keyjoy_fnmatch_pattern = "*";
		}
		else // use for matching filenames
		{
			// store fnpattern and keys into settings:
			cstr pattern = led_filenamepattern->text().toUtf8().data();
			if(eq(pattern,"")) goto a;		// silently ignore.		DENK...
			pattern = addKeyJoyFnmatchPattern(new_keys,pattern);

			// use keys for current machine
			if(eq(mc->filepath,""))
				showWarning( "The configured keys are not used right now because you selected a "
							 "filename pattern and there is currently no snapshot loaded." );
			else if( fnmatch(pattern,lowerstr(basename_from_path(mc->filepath)),FNM_NOESCAPE) )
				showWarning("The configured keys are not used right now because the filename pattern "
							 "does not match the currently loaded snapshot.");
			else
			{
				memcpy(mc->keyjoy_keys, new_keys, sizeof(mc->keyjoy_keys));
				mc->keyjoy_fnmatch_pattern = pattern;
			}
		}

		// Dialog entfernen:
		deleteLater();
	});

//	connect(btn_use_just_now, &QRadioButton::toggled, [&](bool newstate)
//	{
//	});

//	connect(btn_default_for_all_files, &QRadioButton::toggled, [&](bool newstate)
//	{
//	});

//	connect(btn_for_match_pattern, &QRadioButton::toggled, [&](bool newstate)
//	{
//	});
}


ConfigureKeyboardJoystickDialog::~ConfigureKeyboardJoystickDialog()
{
	delete[] old_matchpattern;
#if SECAPS
	if(caps_modified) write_caps();
#endif
	if(patterns_modified) write_patterns();
}



/*	repaint what's not a widget:
*/
void ConfigureKeyboardJoystickDialog::paintEvent(QPaintEvent* e)
{
	ConfigDialog::paintEvent((e));

	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing);

	QPen pen(Qt::red);
	p.setPen(pen);
	QBrush brush(Qt::yellow);
	p.setBrush(brush);

	QPolygon pup(3);

	pup.putPoints(0,3, xm-4*16,ym, xm-2*16,ym-16, xm-2*16,ym+16);
	p.drawPolygon(pup);

	pup.putPoints(0,3, xm+4*16,ym, xm+2*16,ym-16, xm+2*16,ym+16);
	p.drawPolygon(pup);

	pup.putPoints(0,3, xm,ym-4*16, xm-16,ym-2*16, xm+16,ym-2*16);
	p.drawPolygon(pup);

	pup.putPoints(0,3, xm,ym+4*16, xm-16,ym+2*16, xm+16,ym+2*16);
	p.drawPolygon(pup);

	pen.setWidth(2);
	p.drawLine(xm,ym,xm-3*16,ym+3*16);
	p.drawLine(xm-3*16,ym+3*16, xm-4*16,ym+3*16);
	p.drawEllipse(xm-16,ym-16,2*16,2*16);
}




































