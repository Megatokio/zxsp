// Copyright (c) 2013 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#define LOGLEVEL 1
#include <QPainter>
#include <QMouseEvent>
#include <QRectF>
#include <QRect>
#include <QMenu>
#include <math.h>
#include "Lenslok.h"
#include "kio/kio.h"
#include "globals.h"
#include "cpp/cppthreads.h"
#include "Screen/Screen.h"
#include "Renderer/ZxspRenderer.h"
#include "Qt/Settings.h"
#include "MachineController.h"
#define y0 _y0	// <math.h>
#define y1 _y1	// <math.h>


/*	7 ZX Spectrum games which used the Lenslok protection system:
*/
enum GameId
{
	ACE,
	Elite,
	JewelsOfDarkness,
	ArtStudio,
	ThePriceOfMagik,
	Tomahawk,
	TTRacer,
	Unknown
};


/* game names:
*/
static const cstr game_names[] =
{
	"ACE (Air Combat Emulator)",
	"Elite",
	"Jewels of Darkness",
	"OCP Art Studio",
	"The Price of Magik",
	"Tomahawk",
	"TT Racer"
};


/*	2*6 strip values:
	The values indicate the position a strip is taken from, measured from the middle of the prism window.
*/
static const int strip_positions[][12] =
{
	{ -157, -60,   25, -120, -79,  43,   -43,  75, 112, -23,  56, 135 },	// ACE
	{ -60,  -90, -106,   18, -42, -30,    68, -20,  32,  48, 106,  86 },	// Elite
	{ -70,  -99, -124,   24, -47, -37,    73, -21,  38,  47, 113,  92 },	// Jewels of Darkness
	{ -82,  -60, -136, -104, -22, -40,    64, 120,  22,  44,  98, 142 },	// OCP Art Studio
	{ -54,  -78, -142,  -12, -34, -96,   102, 128,  14,  80,  34, 158 },	// Price of Magik
	{ -148, -56, -104,  -36, -76,  18,   -18,  58, 117,  36,  79, 144 },	// Tomahawk
	{ -37,  -76, -128,  -99,  11, -54,   -17, 119,  37,  86,  61, 151 } 	// TT Racer
};


/*	Bereiche, in denen das gesehene Bild berechnet werden muss:
	bezogen auf background_b = Lenslok-Grafik 'folded'
*/
static const int	y0 = 39,		// oberer Rand
			y1 = 108,		// unterer Rand
			x0 = 28,		// linker Rand
			x1 = 46,		// x0-x1 = mattiert
			x2 = 70,		// x1-x2 = 6 Streifen (3 pixel each)
			x3 = 90,		// x2-x3 = mattiert
			x4 = 94,		// x3-x4 = transparenter Mittelstreifen
			x5 = 116,		// x4-x5 = mattiert
			x6 = 140,		// x5-x6 = 6 Streifen (3 pixel each)
			x7 = 158;		// x6-x7 = mattiert

static const QRect prism_box(x0,y0,x7-x0,y1-y0);	// Prism window box inside Lenslok image background_b


/*	forward declaration:
*/
static uint get_game_id( cstr name );



/////////////////////////////////////////////////////////////////////////
////				Creator & Destructor
/////////////////////////////////////////////////////////////////////////


Lenslok::Lenslok(MachineController* mc, cstr name1, cstr name2)
:	QWidget(mc),
	controller(mc),
	background_a(catstr(appl_rsrc_path,"Images/Lenslok-1a-100.png"),NULL,Qt::NoOpaqueDetection),
	background_b(catstr(appl_rsrc_path,"Images/Lenslok-1b-100.png"),NULL,Qt::NoOpaqueDetection),
	background(&background_a),
	contextmenu(new QMenu(this)),
	timer(new QTimer(this)),
	ignore_focusout(no)
{
	xlogIn("new Lenslok");

						 game_id = get_game_id(name1);
	if(game_id==Unknown) game_id = get_game_id(name2);
	if(game_id==Unknown) game_id = get_game_id(settings.get_cstr(key_lenslok,"Price"));	// TODO: Elite

	// update transparent window: the timer is controlled in moveEvent()
	timer->setSingleShot(no);
	connect(timer,&QTimer::timeout,this,[=]{update(prism_box);});

	this->resize( background->size() );
	setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);	// <-- nicht ändern wg. auto-close on FocusOut. Alles andere geht schief!
	setAttribute(Qt::WA_TranslucentBackground,true);
	//setFocusPolicy(Qt::ClickFocus);
	setFocus();
}


Lenslok::~Lenslok()
{
	xlogline("~Lenslok");
	if(game_id<NELEM(game_names)) settings.setValue(key_lenslok,game_names[game_id]);
}


/////////////////////////////////////////////////////////////////////////
////				Helper
/////////////////////////////////////////////////////////////////////////


static uint get_game_id( cstr name )
{
	if(!name) return Unknown;

	name = lowerstr(name);
	if(contains(name,"elite"))	 return Elite;

	if(contains(name,"ace"))	 return ACE;
	if(contains(name,"air"))	 return ACE;
	if(contains(name,"combat"))	 return ACE;

	if(contains(name,"jewel"))	 return JewelsOfDarkness;
	if(contains(name,"dark"))	 return JewelsOfDarkness;

	if(contains(name,"ocp"))	 return ArtStudio;
	if(contains(name,"art"))	 return ArtStudio;

	if(contains(name,"pric"))	 return ThePriceOfMagik;
	if(contains(name,"magi"))	 return ThePriceOfMagik;

	if(contains(name,"tom"))	 return Tomahawk;
	if(contains(name,"hawk"))	 return Tomahawk;

	if(contains(name,"tt"))		 return TTRacer;
	if(contains(name,"race"))	 return TTRacer;

	return Unknown;
}


/*	helper:
	Berechne Farb-Mittelwert in der Box
	Wenn die Box über den Rand des Renderer-Screens übersteht, wird für die überstehenden Bereiche Schwarz verwendet.
*/
static uint32 blur_color(const Renderer& renderer, const QRect& box)
{
//	assert(box.intersects(QRect(0,0,renderer.width,renderer.height)));

	int n = box.width() * box.height();
	int l = max(box.left(),0);
	int r = min(box.right(), int(renderer.width));
	int t = max(box.top(),0);
	int b = min(box.bottom(), int(renderer.height));
	uint R = 0, G = 0, B = 0;

	for(int y=t;y<b;y++)
	{
		RgbaColor* row = renderer.bits + y*renderer.width;
		for(int x=l; x<r; x++)
		{
			RgbaColor pixel = row[x];
			R += (pixel>>24) & 0xff;
			G += (pixel>>16) & 0xff;
			B += (pixel>>8 ) & 0xff;
		}
	}

	R = (R+n/2)/n;
	G = (G+n/2)/n;
	B = (B+n/2)/n;

	return 0xff000000 + (R<<16) + (G<<8) + B;
}


/*	helper:
	copy & scale rect from renderer.screen.qbox -> this.zbox
	needs also to convert from RgbaColor to ARGB
*/
void Lenslok::draw_prism(QPainter& painter, QRectF qbox, const QRectF& zbox)
{
	Renderer& renderer = *ZxspRendererPtr(controller->getScreen()->getScreenRenderer());

	// Koordinaten der auf Integer ausgeweiteten Quell-Box:
	int l = floor(qbox.left());
	int t = floor(qbox.top());
	int r = ceil(qbox.right());
	int b = ceil(qbox.bottom());

	// Zwischen-Image in unzoomed Specci-Pixels
	QImage image(r-l, b-t, QImage::Format_RGB32);

	const int dx = l;		// coord. offset image -> screen
	const int dy = t;
	bool f = 0;				// Flag, ob Image-Hintergrund gelöscht werden muss

	// box in renderer.screen croppen:
	if(l<0) { f=1; l=0; }
	if(t<0) { f=1; t=0; }
	if(r > int(renderer.width))  { f=1; r = renderer.width;  }
	if(b > int(renderer.height)) { f=1; b = renderer.height; }

	// paint it black:
	if(f) for(int y=0; y<image.height(); y++)
	{
		uint32* row = (uint32*)image.scanLine(y);
		for(int x=0; x<image.width(); x++) { row[x] = 0xff000000; }
	}

	// copy & convert pixel:
	l -= dx;
	r -= dx;
	for(int y=t; y<b; y++)		// screen coord.
	{
		RgbaColor* qrow = renderer.bits + renderer.width * y + dx;
		uint32*    zrow = (uint32*)image.scanLine(y-dy);
		for(int x=l; x<r; x++) { zrow[x] = 0xff000000 + (qrow[x]>>8); }
	}

	// qbox nach image-coordinaten konvertieren:
	qbox.moveLeft(qbox.left()-dx);
	qbox.moveTop(qbox.top()-dy);

	// copy & scale pixels:
	//painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);	// Pixel _unter_ die Linse malen
	painter.drawImage(zbox,image,qbox);
}



/////////////////////////////////////////////////////////////////////////
////				Events et. al.
/////////////////////////////////////////////////////////////////////////


/*	Qt callback
	(portions of) the window need to be repainted
*/
void Lenslok::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	p.setCompositionMode(QPainter::CompositionMode_Source);			// only needed in Qt 5.1
	p.drawPixmap ( 0,0, *background );
	if(background==&background_a) return;							// Lenslok not flipped => not in decoding mode

// Lenslok flipped => decoding mode:

	Screen*			screen			= controller->getScreen();
	Renderer*		screen_renderer = ZxspRendererPtr(screen->getScreenRenderer());
	qreal			vzoom			= screen->getZoom();
	qreal			hzoom			= vzoom/screen->getHF();

//	if(screen->getHF()!=1) return;									// TC2048: hor. Skalierung bei zoom==1|3 nicht integer! => Probleme…

	QRect prism_box(::prism_box.translated(geometry().topLeft()));	// Lenslok prism box in glob. coord.
	QRect window_box(controller->geometry());						// Specci screen box in glob. coord.
	if(!window_box.intersects(prism_box)) return;					// Lenslok komplett außerhalb des Specci-Fensters

// Lenslok over Specci window:

	// wir manipulieren nur den Ausschnitt, der über dem Specci-Fensters liegt:

	p.setClipRect(window_box.intersected(prism_box).translated(-this->pos()));

	// window_box auf gesamten Renderer-Screen erweitern:

	window_box.moveLeft(window_box.left()+screen->getLeftBorder()-screen_renderer->h_border*hzoom);
	window_box.moveTop (window_box.top() +screen->getTopBorder() -screen_renderer->v_border*vzoom);
//	window_box.setWidth(screen_renderer->width*hzoom);		not used
//	window_box.setHeight(screen_renderer->height*vzoom);	not used

	// zeichne die 4 mattierten Bereiche:

	p.setCompositionMode(QPainter::CompositionMode_DestinationOver);	// Pixel _unter_ die Linse malen

	for(int i=0;i<4;i++)
	{
		static int L[] = {::x0,x2,x4,x6}; int x0 = L[i];
		static int R[] = {::x1,x3,x5,x7}; int x1 = R[i];

		qreal l = ceil ((geometry().x()+x0-window_box.x())/hzoom);
		qreal t = ceil ((geometry().y()+y0-window_box.y())/vzoom);
		qreal r = floor((geometry().x()+x1-window_box.x())/hzoom);
		qreal b = floor((geometry().y()+y1-window_box.y())/vzoom);

		uint32 color = blur_color(*screen_renderer, QRect(l,t,r-l,b-t));

		p.fillRect(x0,y0,x1-x0,y1-y0,color);
	}

	// zeichne die 12 Prismen:

	int center = geometry().x() + (x3+x4)/2;

	for(int i=0;i<12;i++)
	{
		qreal x = (center + strip_positions[game_id][i] - window_box.x()) / hzoom;		// Specci Screen position
		qreal y = (prism_box.top()-window_box.top()) / vzoom;

		QRectF qbox(x, y, 6.0/hzoom, prism_box.height()/vzoom);
		QRectF zbox(i<6 ? x1+4*i : x5-24+4*i, y0, 4.0, y1-y0);

		draw_prism(p, qbox, zbox);
	}
}


/*	Qt event: left or right mouse button pressed down
	this may start a window dragging or a single click to flip the window between the two states.
*/
void Lenslok::mousePressEvent(QMouseEvent* e)
{
	if(e->button()!=Qt::LeftButton) { QWidget::mousePressEvent(e); return; }

	xlogline("Lenslok mousePressEvent");

	click_p0 = e->globalPos();
	click_dx = e->globalX() - x();
	click_dy = e->globalY() - y();
	click_t0 = now();
}


/*	Qt event: mouse moved
	the Lenslok has no titlebar, so we have to handle window dragging here.
	if this occurs while the mouse button is down, we move the window accordingly.
	if the prism window needs immediate update, this will be handled in the moveEvent().
*/
void Lenslok::mouseMoveEvent(QMouseEvent* e)
{
	xlogline("Lenslok mouseMoveEvent");

	if(e->buttons()&Qt::LeftButton)
	{
		this->move(e->globalX()-click_dx,e->globalY()-click_dy);
	}
}


/*	Qt event: mouse button up
	if the delay between mouse down and mouse up is very short and the mouse has not moved since then,
	then this is a 'click' and will flip the window between the 'flat' state and the 'rotated folded' state.
*/
void Lenslok::mouseReleaseEvent(QMouseEvent* e)
{
	xlogline("Lenslok mouseReleaseEvent");

	if(e->globalPos() == click_p0 && now() <= click_t0+0.5)
	{
		QPoint old_center = background->rect().center();
		background = background==&background_a ? &background_b : &background_a;
		QPoint new_center = background->rect().center();

		this->resize(background->size());
		this->move(this->pos()+old_center-new_center);
	}
}


/*	Qt callback: the Lenslok window is moved
	we test here whether the prism box enters the Specci window
	and start the timer for regular updates of the calculated opaque prism box background
	or stop the timer if the prism box moved away from the Specci window.
*/
void Lenslok::moveEvent(QMoveEvent*)
{
	xlogline("Lenslok moveEvent");

	bool f = background == &background_b &&
			 prism_box.translated(geometry().topLeft()).intersects(controller->geometry());

	if(f || timer->isActive())  { this->repaint(prism_box); }
	if(f && !timer->isActive()) { timer->start(1000/50); }
	if(!f && timer->isActive()) { timer->stop(); }
}




/*	Qt callback: keyboard focus lost
	the user has clicked outside the Lenslok window
	this is the signal for the Lenslok to disappear
*/
void Lenslok::focusOutEvent(QFocusEvent*)
{
	if(ignore_focusout) ignore_focusout = false;
	else deleteLater();
}

bool Lenslok::event(QEvent* e)
{
	xlogline("Lenslok event: %s",QEventTypeStr(e->type()));

	return QWidget::event(e);
}


/*	Qt callback: key pressed
	since we grab keyboard focus we have to promote the received key events to the machine
*/
void Lenslok::keyPressEvent(QKeyEvent*e)
{
	xlogIn("Lenslok:keyPressEvent");
	controller->keyPressEvent(e);
}


/*	Qt callback: key released
	since we grab keyboard focus we have to promote the received key events to the machine
*/
void Lenslok::keyReleaseEvent(QKeyEvent*e)
{
	xlogIn("Lenslok:keyReleaseEvent");
	controller->keyReleaseEvent(e);
}


/*	Qt callback:
	populate and show context menu (if any)
*/
void Lenslok::contextMenuEvent(QContextMenuEvent* e)
{
	xlogIn("Lenslok:contextMenuEvent");

	assert(NELEM(actions)>=NELEM(game_names));

//	if(mouse&&mouse->isGrabbed()) return;		// no context menu if mouse is grabbed

	if(contextmenu->isEmpty())
	{
		for(uint i=0;i<NELEM(game_names);i++)
		{
			actions[i] = contextmenu->addAction(game_names[i], this, &Lenslok::select_game);
			actions[i]->setData(i);
			actions[i]->setCheckable(true);
		}
	}

	for(uint i=0;i<NELEM(game_names);i++)
	{
		actions[i]->setChecked(i==game_id);
	}

	ignore_focusout = yes;
	contextmenu->popup(e->globalPos());
	e->accept();
}


/* slot called from context menu actions:
*/
void Lenslok::select_game()
{
	QAction* actionAdd = qobject_cast<QAction*>(QObject::sender());
	assert(actionAdd);

	game_id = actionAdd->data().toInt();
	update();
}




















