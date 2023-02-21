// Copyright (c) 1994 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "KeyboardInspector.h"
#include "Application.h"
#include "Keyboard.h"
#include "Machine.h"
#include "MachineController.h"
#include "Templates/NVPtr.h"
#include "ZxInfo.h"
#include <QMenu>
#include <QtGui>


namespace gui
{

enum ZxspKey // Names for specci keys
{
	//  These names represent keys on the ZXSP und ZXSP+ keyboard
	//  	0x00-70	= key's row
	//  	0x00-04	= key's column
	//  	0x05-07 = compound keys activating two matrix points

	x = 0x77, // no mapping
	c = 0x80, // add caps shift key
	s = 0x08, // add symbol shift key

	CSH = 0x00,
	Z,
	X,
	C,
	V,
	/*	left bottom row	*/ cZ1,
	cZ2,
	cZ3, /*	EDIT, CAPS LOCK, TRUE VIDEO		*/
	A = 0x10,
	S,
	D,
	F,
	G,
	/* 	...				*/ cZ4,
	cZ5,
	cZ6, /*	INV.VIDEO, CRSR LEFT, CRSR DOWN	*/
	Q = 0x20,
	W,
	E,
	R,
	T,
	/*	...				*/ cZ7,
	cZ8,
	cZ9, /*	CRSR UP, CRSR RIGHT, GRAPHICS	*/
	Z1 = 0x30,
	Z2,
	Z3,
	Z4,
	Z5,
	/* left top row		*/ cZ0,
	cSPC, /*	DELETE, BREAK,					*/
	Z0 = 0x40,
	Z9,
	Z8,
	Z7,
	Z6,
	/* right top row	*/ sO,
	sP, /*	; "								*/
	P = 0x50,
	O,
	I,
	U,
	Y,
	/* 	...				*/ sN,
	sM, /*	, .								*/
	ENT = 0x60,
	L,
	K,
	J,
	H,
	/*	...				*/ CSH2,
	SSH2,
	EXT, /*	2nd CAPS SHIFT, 2nd SYMBOL SHIFT, EXTENDED MODE */
	SPC = 0x70,
	SSH,
	M,
	N,
	B,
	/* right bottom row	*/ NOKEY = 0x77,

	DOT = SSH // "." on ZX80/ZX81 keyboard
};


KeyboardInspector::KeyboardInspector(QWidget* w, MachineController* m, volatile IsaObject* i) :
	Inspector(w, m, i, catstr("Keyboards/", m->getModelInfo()->kbd_filename)),
	model(m->getModel()),
	mousekey(NOKEY),
	keymap() // all key bits = 1 == up
{
	assert(object->isA(isa_Keyboard));

#if QT_VERSION != 0x050300
	setAttribute(Qt::WA_OpaquePaintEvent, 1); // broken in Qt 5.3.0beta
#endif

	connect(m, &MachineController::signal_keymapModified, this, &KeyboardInspector::updateWidgets);

	timer->start(1000 / 20); // also calls updateWidgets()

	setCursor(Qt::PointingHandCursor);
}


QRect KeyboardInspector::keyRect(uint8 spec)
{
	// Berechne Tasten-Boxen für Taste gemäß Scancode
	// Komposittasten müssen mit ihrem Komposittasten-Pseudoscancode übergeben werden.
	// Beachte: Die ENTER-Taste hat manchmal L-Form. Dann wird nur die untere Hälfte zurückgegeben.
	// Das Layout von plus/128/pentagon und +2/+3 unterscheidet sich nur durch die Breite von EXTENDED MODE und EDIT.

	xlogIn("KbdInsp:KeyRect(spec=$%2hhX)", spec);

	int	 x, y, w, h; // {x,y,w,h} = result box
	bool f = false;

	spec &= 0x77;  // remove modifier bits  ((should not be set anyway!))
	y = spec >> 4; // 0..7			0..3 = left side, 4..7 = right side
	x = spec & 7;  // 0..4			0=outmost key, 4=inmost key

	switch (model)
	{
		//	case ts1000:
	case zx80:
	case zx81:
	{
		if (x >= 5) goto xx; // error
		if (y >= 4)
		{
			y = 7 - y;
			x = 9 - x;
		} // right side?  =>  y=0..3  x=5..9

		const int x0[] = {17 + 12, 17 + 36, 17 + 24, 17}; // left border of leftmost key

		w = 40; // key width
		h = 32; // key height
		x = x * 48 + x0[y];
		y = y * 46 + 11;

		if (model == zx81)
		{
			h = 28;
			y += 2;
		}
		break;
	}

		//	case pentagon128:
		//		{
		//			if (x>=5) goto xx;				// error
		//			if (y>=4) { y=7-y; x=9-x; }		// right side?  =>  y=0..3  x=5..9
		//			int const x0[]={71,81,71,61};	// x-Wert für 1. Taste einer Reihe
		//			x = x0[y]+40*x;
		//			y = 21+40*y;
		//			w = 41;
		//			h = 41;
		//			break;
		//		}

		//	case ts1500:
		//	case tk85:
		//	case tk95:		// preliminary, as long as it uses the zxsp keyboard image due to lack of own keyboard image
	case tk90x:
	case zxsp_i1:
	case zxsp_i2:
	case zxsp_i3:
	{
		if (x >= 5) goto xx; // error
		if (y >= 4)
		{
			y = 7 - y;
			x = 9 - x;
		} // right side?  =>  y=0..3  x=5..9

		const int x0[] = {
			20, 45, 33, 8}; // x-Wert für 1. Taste einer Reihe (bei CSH: normalisiert auf Std-Tastenbreite)

		x = x0[y] + 50 * x;
		y = 9 + 49 * y;
		w = 37;
		h = 27;

		if (spec == CSH)
		{
			x = 8;
			w = 49;
		}							 // CAPS SHIFT ist breiter
		if (spec == SPC) { w = 62; } // SPACE ist breiter
		break;
	}

	case jupiter:
	{
		if (x >= 5) goto xx; // error
		if (y >= 4)
		{
			y = 7 - y;
			x = 9 - x;
		} // right side?  =>  y=0..3  x=5..9

		const int i	   = 1;								   // inset. else the inverting key box has unesthetic corners
		const int x0[] = {32 + i, 54 + i, 41 + i, 19 + i}; // x-Wert für 1. Taste einer Reihe

		x = x0[y] + 48 * x;
		y = 15 + i + 48 * y;
		w = 33 - 2 * i;
		h = 30 - 2 * i;

		if (spec == SPC) { w = 55 - 2 * i; } // SPACE ist breiter
		break;
	}

	case u2086:
	case tc2068:
	case ts2068:
	case tc2048:
	{
		h = 39; // height for all keys
		w = 46; // default width

		// Komposit- und Sondertasten:
		switch (spec)
		{
		case CSH: /* 1. CSH */
			x = 13;
			y = 37;
			w = 58;
			goto x; // 1. CAPS SHIFT Taste: breiter ((die linke))
		case CSH2:	/* 2. CSH */
			x = 70 + 45 * 9;
			y = 37;
			w = 58;
			goto x; // 2. CAPS SHIFT Taste: zus. Taste ((die rechte))
		case ENT:	/* ENTER  */
			x = 47 + 45 * 9;
			y = 37 + 39;
			w = 81;
			goto x; // ENTER: breiter
		case SPC:	/* SPACE  */
			x = 115;
			y = 0;
			w = 316;
			goto x; // SPACE: unter allen anderen Tasten und richtig breit
		case cSPC:	/* BREAK  */
			x = 70 + 45 * 8;
			y = 37;
			w = 46;
			goto x; // BREAK: neue Komposittaste
		}

		// normale Taste:
		if (x >= 5) goto xx; // error
		if (y >= 4)
		{
			y = 7 - y;
			x = 9 - x;
		}								   // right side?  =>  y=0..3  x=5..9
		const int x0[] = {25, 47, 36, 13}; // x-wert für 1. taste einer reihe (bei CSH-Taste: extrapolierte Position)
		x			   = 45 * x + x0[y];
		y			   = 39 * y + 37;
		break;
	}

	case zxplus:
	case inves:
	case zx128:
	case zx128_span:
	case pentagon128: f = true; FALLTHROUGH
	case zxplus2:
	case zxplus2_span:
	case zxplus2_frz:
	case zxplus3:
	case zxplus3_span:
	case zxplus2a:
	case zxplus2a_span:
	{
		const int i = 1;		  // inset, req. for light Pentagon keyboard
		w			= 41 - 2 * i; // default width
		h			= 41 - 2 * i; // height for all keys


		// Komposit- und Sondertasten:
		switch (spec)
		{
		case EXT: /*EXTEND MODE*/
			x = 0 + i;
			y = 80 + i;
			w = f ? 61 - 2 * i : 71 - 2 * i;
			goto x;
		case sO: /* ; */
			x = 40 + i;
			y = 0 + i;
			goto x;
		case sP: /* " */
			x = 80 + i;
			y = 0 + i;
			goto x;
		case sN: /* , */
			x = 460 + i;
			y = 0 + i;
			goto x;
		case sM: /* . */
			x = 410 + i;
			y = 40 + i;
			goto x;
		case cZ1: /*EDIT*/
			x = f ? 60 + i : 70 + i;
			y = 80 + i;
			w = f ? 51 - 2 * i : 41 - 2 * i;
			goto x;
		case cZ2: /*CAPS LOCK*/
			x = 90 + i;
			y = 40 + i;
			goto x;
		case cZ3: /*TRUE VIDEO*/
			x = 0 + i;
			y = 160 + i;
			goto x;
		case cZ4: /*INV VIDEO*/
			x = 40 + i;
			y = 160 + i;
			goto x;
		case cZ5: /*CURSOR LEFT*/
			x = 120 + i;
			y = 0 + i;
			goto x;
		case cZ6: /*CURSOR DOWN*/
			x = 420 + i;
			y = 0 + i;
			goto x;
		case cZ7: /*CURSOR UP*/
			x = 380 + i;
			y = 0 + i;
			goto x;
		case cZ8: /*CURSOR RIGHT*/
			x = 160 + i;
			y = 0 + i;
			goto x;
		case cZ9: /*GRAPHICS*/
			x = 60 + i;
			y = 120 + i;
			goto x;
		case cZ0: /*DELETE*/
			x = 0 + i;
			y = 120 + i;
			w = 61 - 2 * i;
			goto x;
		case cSPC: /*BREAK*/
			x = 480 + i;
			y = 160 + i;
			w = 61 - 2 * i;
			goto x;
		case SSH2: /*2. SSH Taste*/
			x = 0 + i;
			y = 0 + i;
			goto x;
		case CSH2: /*2. CSH Taste*/
			x = 0 + i;
			y = 40 + i;
			w = 91 - 2 * i;
			goto x;
		case CSH:
			x = 450 + i;
			y = 40 + i;
			w = 91 - 2 * i;
			goto x;
		case SSH:
			x = 500 + i;
			y = 0 + i;
			goto x;
		case SPC:
			x = 200 + i;
			y = 0 + i;
			w = 181 - 2 * i;
			goto x;
			//			case ENT:					x=470+i; y=80+i;  w=81-2*i;  goto x;	// L-förmig!
		case ENT:
			x = 500 + i;
			y = 80 + i;
			h = 81 - 2 * i;
			goto x; // L-förmig!
		case L:
			if (model == pentagon128) w = 51 - 2 * i;
			break;
		}

		// normale Taste:
		if (x >= 5) goto xx; // error
		if (y >= 4)
		{
			y = 7 - y;
			x = 9 - x;
		} // right side?  =>  y=0..3  x=5..9
		const int x0[] = {
			90 + i, 110 + i, 100 + i,
			80 + i}; // x-wert für 1. taste einer reihe (bei CSH-Taste: extrapolierte Position)
		x = 40 * x + x0[y];
		y = 40 * y + 40 + i;
		break;
	}

	default: // error exit:
	{
		static bool f = 1;
		if (f)
		{
			logline("KbdPanel:KeyRect: model %s not yet supported", zx_info[model].name);
			f = 0;
		}
	xx:
		x = y = w = h = 0;
		break;
	}
	}

x: // printf("box($%02x) = (%i,%i),(%i,%i)\n",spec,x,y,w,h);
	return QRect(x, 201 - y - h, w, h);
}

QRegion KeyboardInspector::keyRegion(uint8 spec)
{
	QRegion regio = keyRect(spec);

	if (spec == ENT) switch (machine->model)
		{
		default: break;
		case zxplus:
		case inves:
		case zx128:
		case zx128_span:
		case zxplus2:
		case zxplus2_span:
		case zxplus2_frz:
		case zxplus3:
		case zxplus3_span:
		case zxplus2a:
		case zxplus2a_span: return regio.united(QRect(500 - 40 + 11, 201 - 80 - 41, 30, 41));

		case pentagon128: return regio.united(QRect(500 + 1 - 19, 201 - (80 + 1) - 41 + 2, 19, 41 - 2));
		}
	return regio;
}

QRect Tk90xKbdInsp::keyRect(uint8 spec)
{
	xlogIn("Tk90xKbdInsp:KeyRect(spec=$%2hhX)", spec);

	spec &= 0x77;	   // remove modifier bits  ((should not be set anyway!))
	int x = spec & 7;  // 0..4			0=outmost key, 4=inmost key
	int y = spec >> 4; // 0..7			0..3 = left side, 4..7 = right side
	int w = 33;
	int h = 24;
	if (x >= 5) return QRect();

	// if (x>=5) goto xx;		  	// error
	if (y >= 4)
	{
		y = 7 - y;
		x = 9 - x;
	} // right side?  =>  y=0..3  x=5..9

	const int x0[] = {34, 58, 46, 22}; // x-Wert für 1. Taste einer Reihe (bei CSH: normalisiert auf Std-Tastenbreite)
	const int y0[] = {166, 118, 70, 22}; // von unten nach oben

	x = x0[y] + int((453 - 24) * x / 9.0);
	y = y0[y];

	if (spec == CSH)
	{
		x = 22;
		w = 44;
	}							 // CAPS SHIFT ist breiter
	if (spec == SPC) { w = 56; } // SPACE ist breiter

	return QRect(x, y, w, h);
}

QRect Ts1000KbdInsp::keyRect(uint8 spec)
{
	xlogIn("Tc1000KbdInsp:KeyRect(spec=$%2hhX)", spec);

	spec &= 0x77;	   // remove modifier bits  ((should not be set anyway!))
	int x = spec & 7;  // 0..4			0=outmost key, 4=inmost key
	int y = spec >> 4; // 0..7			0..3 = left side, 4..7 = right side
	int w = 42;
	int h = 30;
	if (x >= 5) return QRect();

	// if (x>=5) goto xx;		  	// error
	if (y >= 4)
	{
		y = 7 - y;
		x = 9 - x;
	} // right side?  =>  y=0..3  x=5..9

	const int x0[] = {25, 49, 38, 11}; // x-Wert für 1. Taste einer Reihe (bei CSH: normalisiert auf Std-Tastenbreite)
	const int y0[] = {160, 112, 65, 18}; // von unten nach oben

	int x1 = x0[y] + int(439 * x / 9);
	int y1 = y0[y] - int(3 * x / 11);

	return QRect(x1, y1, w, h);
}

QRect Tk85KbdInsp::keyRect(uint8 spec)
{
	xlogIn("Tk85KbdInsp:KeyRect(spec=$%2hhX)", spec);

	spec &= 0x77;	   // remove modifier bits  ((should not be set anyway!))
	int x = spec & 7;  // 0..4			0=outmost key, 4=inmost key
	int y = spec >> 4; // 0..7			0..3 = left side, 4..7 = right side
	int w = 37;
	int h = 27;
	if (x >= 5) return QRect(); // error
	if (y >= 4)
	{
		y = 7 - y;
		x = 9 - x;
	} // right side?  =>  y=0..3  x=5..9

	static const int x0[] = {18, 44, 31, 6}; // x-Wert für 1. Taste einer Reihe (CSH: normalisiert auf Std-Tastenbreite)
	static const int y0[] = {161, 111, 61, 11}; // von unten nach oben

	int x1 = x0[y] + 454 * x / 9;
	int y1 = y0[y] + x / 6;

	if (spec == CSH)
	{
		x1 = 9;
		w  = 47;
	}							 // CAPS SHIFT ist breiter
	if (spec == SPC) { w = 61; } // SPACE ist breiter

	return QRect(x1, y1, w, h);
}

QRect Ts1500KbdInsp::keyRect(uint8 spec)
{
	xlogIn("Ts1500KbdInsp:KeyRect(spec=$%2hhX)", spec);

	spec &= 0x77;	   // remove modifier bits  ((should not be set anyway!))
	int x = spec & 7;  // 0..4			0=outmost key, 4=inmost key
	int y = spec >> 4; // 0..7			0..3 = left side, 4..7 = right side
	int w = 37;
	int h = 27;
	if (x >= 5) return QRect();

	// if (x>=5) goto xx;		  	// error
	if (y >= 4)
	{
		y = 7 - y;
		x = 9 - x;
	} // right side?  =>  y=0..3  x=5..9

	const int x0[] = {19, 45, 32, 7}; // x-Wert für 1. Taste einer Reihe (bei CSH: normalisiert auf Std-Tastenbreite)
	const int y0[] = {161, 111, 61, 11}; // von unten nach oben

	int x1 = x0[y] + 454 * x / 9;
	int y1 = y0[y] - x / 5;

	if (spec == CSH)
	{
		x1 = 9;
		w  = 47;
	}							 // CAPS SHIFT ist breiter
	if (spec == SPC) { w = 61; } // SPACE ist breiter

	return QRect(x1, y1, w, h);
}

QRect Tk95KbdInsp::keyRect(uint8 spec)
{
	xlogIn("Tk95KbdInsp:KeyRect(spec=$%2hhX)", spec);

	spec &= 0x77;		 // remove modifier bits  ((should not be set anyway!))
	int col = spec & 7;	 // 0..4			0=outmost key, 4=inmost key
	int row = spec >> 4; // 0..7			0..3 = left side, 4..7 = right side

	const int x1 = 43, x2 = 61, x3 = 70, x4 = 52; // x-Wert für 1. Taste einer Reihe
	const int xx[] = {x4, x3, x2, x1};
	const int y1 = 24, y2 = 60, y3 = 95, y4 = 131, y5 = 166; // y-Wert für alle Tasten einer Reihe
	const int yy[] = {y4, y3, y2, y1};						 // von unten nach oben

	int x = 0, y = y1, w = 34, h = 34;

#define X(X0, COL) (X0 + (363 - x1) * COL / 9)

	switch (spec)
	{
		// Zeile 1:
	case cZ1: w = 42; break;		// EDIT
	case sP: x = X(x1, 10); break;	// "
	case cZ3: x = X(x1, 11); break; // True video
	case cZ4: x = X(x1, 12); break; // Inv. video
	case cSPC:
		x = X(x1, 13);
		break; // BREAK

		// Zeile 2:
	case cZ9:
		y = y2 - 1;
		w = x2;
		break; // GRAPHICS
	case cZ7:
		y = y2 - 1;
		x = X(x2, 10);
		break; // UP
	case cZ6:
		y = y2 - 2;
		x = X(x2, 11);
		break; // DOWN
	case ENT:
		y = y2 - 2;
		x = X(x2, 12);
		w += 18;
		break; // ENTER

		// Zeile 3:
	case EXT:
		y = y3;
		w = x3;
		break; // Extended mode
	case sO:
		y = y3;
		x = X(x3, 9);
		break; // ;
	case cZ5:
		y = y3 - 1;
		x = X(x3, 10);
		break; // LEFT
	case cZ8:
		y = y3 - 1;
		x = X(x3, 11);
		break; // RIGHT
	case cZ0:
		y = y3 - 1;
		x = X(x3, 12);
		w += 9;
		break; // DELETE

		// Zeile 4:
	case CSH:
		y = y4 - 1;
		w = x4;
		break; // CAPS SHIFT
	case cZ2:
		y = y4;
		x = x4;
		break; // CAPS LOCK
	case sM:
		y = y4;
		x = X(x4, 8);
		break; // .
	case sN:
		y = y4;
		x = X(x4, 9);
		break; // ,
	case CSH2:
		y = y4;
		x = X(x4, 10);
		break; // CAPS SHIFT 2
	case SSH:
		y = y4 - 1;
		x = X(x4, 11);
		w *= 3;
		break; // Symbol shift

		// Zeile 5:
	case SPC:
		y = y5;
		x = 105;
		w = 321;
		break; // SPACE

		// Keys with default width at original position:
	default:
		if (col >= 5) return QRect();
		if (row >= 4)
		{
			row = 7 - row;
			col = 9 - col;
		} // right side?  =>  y=0..3  x=5..9
		x = X(xx[row], col);
		y = yy[row] - col / 5;
		break;
	}

#undef X

	return QRect(x, y, w, h);
}

uint8 KeyboardInspector::findKeyForPoint(QPoint where)
{
	// find key for mouse pointer position

	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
		{
			uint8 spec = uint8(i * 16 + j);
			if (spec == ENT ? keyRegion(spec).contains(where) : keyRect(spec).contains(where)) return spec;
		}
	xlogline("point outside any key");
	return NOKEY;
}

void KeyboardInspector::updateWidgets()
{
	// update keyboard display
	// called by timer
	// and directly after known key state change

	if (!machine || !object) return;

	Keymap newkeys = const_cast<Keymap&>(kbd()->keymap);
	if (newkeys.allrows == keymap.allrows) return;

	for (int i = 0; i < 8; i++)
	{
		uint8 c = newkeys[i] ^ keymap[i];
		if (c)
		{
			for (int j = 0; j < 8; j++)
			{
				if (c & (1 << j))
				{
					uint8 spec = uint8((i << 4) + j);
					if (spec == ENT) update(keyRegion(spec));
					else update(keyRect(spec));
				}
			}
		}
	}

	keymap = newkeys;
}

void KeyboardInspector::fillContextMenu(QMenu* m)
{
	//	add actions to popup menu
	//	standard Inspector stuff will be added later by caller

	m->addActions(controller->getKeyboardActions());
}


// ----------------------------------------------
//			Qt Interface
// ----------------------------------------------

void KeyboardInspector::paintEvent(QPaintEvent* e)
{
	// paint updated region

	// if the Qt::WA_OpaquePaintEvent widget attribute is set,
	// the widget is responsible for painting all its pixels with an opaque color.

	xlogIn("KbdInsp:paintEvent");
	QPainter p(this);
	p.drawPixmap(e->rect(), background, e->rect());
	p.setPen(Qt::NoPen);
	p.setBrush(QColor(0, 0, 0, 127)); // rgba: black, 50%
	p.setBackgroundMode(Qt::OpaqueMode);

	for (int i = 0; i < 8; i++) // 8 scanbytes
	{
		uint8 byte = keymap[i];
		if (byte == 0xff) continue; // no key down
		for (int j = 0; j < 8; j++) // 5 std bits + 3 bits used for composed keys
		{
			if ((byte >> j) & 1) continue; // this key is not down
			uint8 spec = uint8((i << 4) + j);
			if (spec == ENT)
			{
				p.setClipRegion(keyRegion(spec));
				p.drawRect(p.clipRegion().boundingRect());
			}
			else p.drawRect(keyRect(spec));
		}
	}
}

void KeyboardInspector::mousePressEvent(QMouseEvent* e)
{
	xlogIn("KbdInsp:mousePressEvent");

	if (e->button() == Qt::LeftButton || (QGuiApplication::keyboardModifiers() & Qt::META))
	{ // note: CTRL-Key + left mouse button werden von Qt (OSX?) in right mouse button umgewandelt
		// außerdem erzeugen sie ein contextMenu event, das in event() abgefangen wird

		xlogline("mouse down at %i,%i", e->x(), e->y());
		mousekey = findKeyForPoint(e->pos()); // e->pos() = widget (window) coordinates
		if (mousekey != NOKEY)
		{
			xlogline("mousekey=$%02x", uint(mousekey));
			NVPtr<Keyboard>(kbd())->specciKeyDown(mousekey);
			updateWidgets();
		}
	}
	else Inspector::mousePressEvent(e);
}

void KeyboardInspector::mouseReleaseEvent(QMouseEvent* e)
{
	xlogIn("KbdInsp:mouseReleaseEvent");

	if (mousekey != NOKEY)
	{
		NVPtr<Keyboard>(kbd())->specciKeyUp(mousekey);
		mousekey = NOKEY;
		updateWidgets();
	}
	Inspector::mouseReleaseEvent(e);
}

void KeyboardInspector::mouseMoveEvent(QMouseEvent* e)
{
	// If mouse tracking is switched off, mouse move events only occur while a mouse button is pressed.
	// If mouse tracking is switched on, mouse move events occur even if no mouse button is pressed.

	xlogIn("KbdInsp:mouseMoveEvent");

	if (e->buttons() & Qt::LeftButton)
	{
		if (mousekey != NOKEY && keyRegion(mousekey).contains(e->pos())) return; // still over same key

		// mouse left previous key

		uint8 old = mousekey;
		{
			NVPtr<Keyboard> keyboard(kbd());
			if (mousekey != NOKEY) keyboard->specciKeyUp(mousekey);
			mousekey = findKeyForPoint(e->pos());
			if (mousekey != NOKEY) keyboard->specciKeyDown(mousekey);
		}
		if (mousekey != old) updateWidgets();
	}
	else Inspector::mouseMoveEvent(e);
}

bool KeyboardInspector::event(QEvent* e)
{
	xlogline("KbdInsp event: %s", QEventTypeStr(e->type()));

	// CTRL-Key + left mouse button werden von MacOS in right mouse button umgewandelt
	// damit geht kein CTRL-Klick im Kbd Inspector => ContextMenu event abfangen
	// siehe auch mousePressEvent()

	if (e->type() == QEvent::ContextMenu && (QGuiApplication::keyboardModifiers() & Qt::META)) return true /*handled*/;

	return Inspector::event(e);
}

} // namespace gui
