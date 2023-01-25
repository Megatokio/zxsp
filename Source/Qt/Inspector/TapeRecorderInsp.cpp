// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include <QPainter>
#include <QPaintEvent>
#include <QTextEdit>
#include "TapeRecorderInsp.h"
#include "TapeRecorder.h"
#include "Qt/qt_util.h"
#include <QMatrix>
#include <QMenu>
#include <QAction>
#include "Qt/Settings.h"
#include "RecentFilesMenu.h"
#include "unix/files.h"
#include "Templates/NVPtr.h"


static const int		frames_per_rot			= 36;
static const int		ROT_STEP				= 360 / frames_per_rot;
static const double		ACHSENABSTAND_MM		= 42.5;
static const Frequency	FULLSPEED_ROT_PER_SEC	= 3;
static const Time		FULLSPEED_SEC_PER_FRAME	= 1.0 / FULLSPEED_ROT_PER_SEC / frames_per_rot;


Cassette::Cassette(CassBody cass, HeadPos head_dn)
{
	switch(cass)
	{
	default:	// BlueBody:
		hdgr_fname	= head_dn ? "Images/tape/cassette_down_with_label.png" : "Images/tape/cassette_up_with_label.png";
		reel_fname	= "Images/tape/reel.png";
		head_pos	= head_dn;

		reel_symmetries = 6;

		width		= 232;					// Größe der Kassette (des Hintergrundbildes)
		height		= 148;

		axis_y		= head_dn ? 68 : 79;	// Position der Achsen
		axis_x1		= 67;
		axis_x2		= 164;

		window_x1	= 29;					// Größe und Position des durchsichtigen Fensters
		window_y1	= head_dn ? 45 : 44;
		window_x2	= 205;
		window_y2	= head_dn ?	103 : 102;

		label_x		= 5;					// Position des schwarzen Labels (l/o Ecke)
		label_y		= head_dn ? 16 : 101;

		ppmm		= 2.31;					// ~ height / 64.0mm ~ width / 100.5mm
	}
}

Plus2TapeRecorderInsp::Plus2TapeRecorderInsp( QWidget* p, MachineController* mc, volatile IsaObject* item )
: PlusTapeRecorderInsp(p,mc,item, "Images/tape/plus2_hdgr.jpg", "Images/tape/plus2_lid.png", ":/Icons/btn2_")
{}

Plus2aTapeRecorderInsp::Plus2aTapeRecorderInsp( QWidget* p, MachineController* mc, volatile IsaObject* item )
: PlusTapeRecorderInsp(p,mc,item, "Images/tape/plus2a_hdgr.jpg", "Images/tape/plus2a_lid.png", ":/Icons/btn2A_")
{}

// +2 or +2A tape recorder
PlusTapeRecorderInsp::PlusTapeRecorderInsp( QWidget *parent, MachineController* mc, volatile IsaObject *item,
											cstr hdgr, cstr tray, cstr btn_root )
:
   TapeRecorderInsp(parent,mc,item,
		QPoint(38,19),					// major info pos
		QPoint(38,31),					// minor info pos
		QPoint(120,141),				// tape counter pos
		hdgr,							// background image
		tray,							// cassette tray image
		QPoint(21,0),					// cassette tray window_pos
		head_down,						// head position
		"Images/tape/axis_plus2.png",	// axis image
		6,								// axis symmetries
		89,187,67)						// axis position x1, x2, y
{
	xlogIn("new PlusTapeRecorderInsp");

	const int btn_x = 30, btn_y = 201, btn_w = 40;

	btn_record = new MySimpleToggleButton(this, btn_x+0*btn_w, btn_y, btn_root, "record.png","record_dn.png",yes);
	btn_play   = new MySimpleToggleButton(this, btn_x+1*btn_w, btn_y, btn_root, "play.png",  "play_dn.png",	yes);
	btn_back   = new MySimpleToggleButton(this, btn_x+2*btn_w, btn_y, btn_root, "back.png",  "back_dn.png",	yes);
	btn_fore   = new MySimpleToggleButton(this, btn_x+3*btn_w, btn_y, btn_root, "fore.png",  "fore_dn.png",	yes);
	btn_eject  = new MySimpleToggleButton(this, btn_x+4*btn_w, btn_y, btn_root, "eject.png", "eject_dn.png", yes);
	btn_pause  = new MySimpleToggleButton(this, btn_x+5*btn_w, btn_y, btn_root, "pause.png", "pause_dn.png", no);

	/*	The buttons just set the tape recorder to the new state
		Later, updateWidgets() will update the animation and the button images
	*/
	connect(btn_record,&MySimpleToggleButton::toggled, this, [=]{nv_taperecorder()->record();});
	connect(btn_play,  &MySimpleToggleButton::toggled, this, [=]{nv_taperecorder()->play();});
	connect(btn_back,  &MySimpleToggleButton::toggled, this, [=]{nv_taperecorder()->rewind();});
	connect(btn_fore,  &MySimpleToggleButton::toggled, this, [=]{nv_taperecorder()->wind();});
	connect(btn_pause, &MySimpleToggleButton::toggled, this, [=](bool f){nv_taperecorder()->pause(f);});
	connect(btn_eject, &MySimpleToggleButton::toggled, this, [=]{handleEjectButton();});
}

// TS2020 tape recorder:
TS2020Inspector::TS2020Inspector( QWidget* parent, MachineController* mc, volatile IsaObject* item )
:
   TapeRecorderInsp(parent,mc,item,
		QPoint(68,44),					// major info pos
		QPoint(68,56),					// minor info pos
		QPoint(316,229),				// tape counter pos
		"Images/tape/ts2020.jpg",		// background image
		"Images/tape/ts2020_lid.png",	// cassette tray image
		QPoint(22,20),					// cassette tray position
		head_down,						// head position
		"Images/tape/axis_plus2.png",	// axis image
		6,								// axis symmetries
		104,205,94)						// axis position x1, x2, y
{
	xlogIn("new TS2020Inspector");

	const int  btn_y = 237;
	const cstr root  = ":/Icons/ts2020/";

	btn_record = new MySimpleToggleButton(this,  69, btn_y, root, "record.jpg","record_dn.jpg", yes);
	btn_play   = new MySimpleToggleButton(this, 111, btn_y, root, "play.jpg",  "taste_dn.jpg",  yes);
	btn_back   = new MySimpleToggleButton(this, 154, btn_y, root, "rewind.jpg","taste_dn.jpg",  yes);
	btn_fore   = new MySimpleToggleButton(this, 196, btn_y, root, "ff.jpg",    "taste_dn.jpg",  yes);
	btn_eject  = new MySimpleToggleButton(this,  26, btn_y, root, "eject.jpg", "taste_dn.jpg",  yes);
	btn_pause  = new MySimpleToggleButton(this, 239, btn_y, root, "pause.jpg", "taste_dn.jpg",  no );

	/*	The buttons just set the tape recorder to the new state
		Later, updateWidgets() will update the animation and the button images
	*/
	connect(btn_record,&MySimpleToggleButton::toggled, this, [=]{nv_taperecorder()->record();});
	connect(btn_play,  &MySimpleToggleButton::toggled, this, [=]{nv_taperecorder()->play();});
	connect(btn_back,  &MySimpleToggleButton::toggled, this, [=]{nv_taperecorder()->rewind();});
	connect(btn_fore,  &MySimpleToggleButton::toggled, this, [=]{nv_taperecorder()->wind();});
	connect(btn_pause, &MySimpleToggleButton::toggled, this, [=](bool f){nv_taperecorder()->pause(f);});
	connect(btn_eject, &MySimpleToggleButton::toggled, this, [=]{handleEjectButton();});
}

// tape recorder base class:
TapeRecorderInsp::TapeRecorderInsp( QWidget* w, MachineController* mc, volatile IsaObject* item,
		QPoint	majorinfopos,
		QPoint	minorinfopos,
		QPoint	tapecounterpos,
		cstr	hdgr_image_filename,
		cstr	tray_image_filename,
		QPoint	tray_position,
		HeadPos	head_position,
		cstr	axis_image_filename,
		int		axis_symmetries,
		int		axis_x1, int axis_x2, int axis_y )
:
	Inspector(w,mc,item,hdgr_image_filename),
	btn_record(nullptr),
	btn_play(nullptr),
	btn_back(nullptr),
	btn_fore(nullptr),
	btn_next(nullptr),
	btn_prev(nullptr),
	btn_eject(nullptr),
	btn_pause(nullptr),
	major_block_info(""),
	minor_block_info(""),
	tape_position(0),
	major_block_info_label(new QLabel(this)),
	minor_block_info_label(new QLabel(this)),
	tape_position_label(new QLineEdit(this)),
	tape_filepath(nullptr),
	cass(BlueBody,head_position),
	axis_x1(axis_x1),
	axis_x2(axis_x2),
	axis_y(axis_y),								// position in tr_image [pixels]
	ppmm((axis_x2-axis_x1)/ACHSENABSTAND_MM),	// scaling: pixels per mm  ((double))
	anim_tr_loaded(no),
	anim_tr_pause(no),
	anim_tr_state(TapeRecorder::stopped)
{
	xlogIn("new TapeRecorderInsp");
	assert( item->isA(isa_TapeRecorder) );

	// tape recorder background image:
	// this image defines the image size.
	{
		tr_image = QImage(catstr(appl_rsrc_path,hdgr_image_filename));
		if(tr_image.format()!=QImage::Format_RGB32)
		{
			xlogline("TaperecorderInspector: tr_image musste nach RGB konvertiert werden.");
			tr_image = tr_image.convertToFormat(QImage::Format_RGB32);
		}
	}

	// tape recorder window image:
	// scaled to fit tr_window_rect:
	{
		tr_window_image = QImage(catstr(appl_rsrc_path,tray_image_filename));
		if(tr_window_image.format()!=QImage::Format_ARGB32_Premultiplied)
		{
			xlogline("TaperecorderInspector: tr_window musste nach ARGB_Premultiplied konvertiert werden!");	// dem ist so
			tr_window_image = tr_window_image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
		}
		tr_window_rect = QRect(tray_position,tr_window_image.size());
	}

	// axis images:
	{
		QImage axis_source_image(catstr(appl_rsrc_path,axis_image_filename));
		axis_dia = floor(11*ppmm); axis_dia += axis_dia&1;		// axis image diameter [pixels]

		for(int i=0;i<frames_per_rot/axis_symmetries;i++)
		{
			QMatrix trans;
			trans = trans.translate(axis_source_image.width()/2.0,axis_source_image.height()/2.0);
			trans = trans.rotate(i*ROT_STEP);
			trans = trans.scale((qreal)axis_dia/axis_source_image.width(),(qreal)axis_dia/axis_source_image.height());

			tr_axis_images.append(QImage(axis_source_image.transformed(trans,Qt::SmoothTransformation)));
		}
	}

	// cassette image
	// scaled to tr_image
	// cass_rect defines position and size of cassette in tr_image
	// cass_window_rect defines position and size of cassette window in tr_image:
	{
		double scale  = ppmm / cass.ppmm;				// cass img pixels -> tr img pixels

		int cass_x1 = int(round( axis_x1 - scale *  cass.axis_x1 ));
		int cass_x2 = int(round( axis_x2 + scale * (cass.width-cass.axis_x2) ));
		int cass_y1 = int(round( axis_y  - scale *  cass.axis_y ));
		int cass_y2 = int(round( axis_y  + scale * (cass.height-cass.axis_y) ));

		cass_rect = QRect(cass_x1,cass_y1,cass_x2-cass_x1,cass_y2-cass_y1);

		cass_image = QImage(cass_rect.size(),QImage::Format_ARGB32_Premultiplied);
		cass_image.fill(0x00000000);
		QPainter(&cass_image).drawImage(cass_image.rect(),QImage(catstr(appl_rsrc_path,cass.hdgr_fname)));

		int x1 = int(floor(cass.windowX1() * scale));
		int x2 = int(ceil (cass.windowX2() * scale));
		int y1 = int(floor(cass.windowY1() * scale));
		int y2 = int(ceil (cass.windowY2() * scale));

		cass_window_rect = QRect(cass_x1+x1,cass_y1+y1,x2-x1,y2-y1);
	}

	// reel images:
	{
		QImage reel_source_image(catstr(appl_rsrc_path,cass.reel_fname));
		reel_dia = floor(42.1*ppmm); reel_dia -= reel_dia&1;

		for(int i=0;i<frames_per_rot/cass.reel_symmetries;i++)
		{
			QMatrix trans;
			trans = trans.translate(reel_source_image.width()/2.0,reel_source_image.height()/2.0);
			trans = trans.rotate(i*ROT_STEP);
			trans = trans.scale((qreal)reel_dia/reel_source_image.width(),(qreal)reel_dia/reel_source_image.height());

			cass_reel_images.append(QImage(reel_source_image.transformed(trans,Qt::SmoothTransformation)));
		}
	}

	// resulting window_rect for animation:
	// this is the intersection of tr_window and cass_window:
	window_rect     = tr_window_rect.intersected(cass_window_rect);
	left_axis_rect  = QRect(axis_x1-axis_dia/2,axis_y-axis_dia/2,axis_dia,axis_dia);
	right_axis_rect = QRect(axis_x2-axis_dia/2,axis_y-axis_dia/2,axis_dia,axis_dia);
	left_reel_rect  = QRect(axis_x1-reel_dia/2,axis_y-reel_dia/2,reel_dia,reel_dia);
	right_reel_rect = QRect(axis_x2-reel_dia/2,axis_y-reel_dia/2,reel_dia,reel_dia);

	// tape on reel:
	// note: in a real cassette the max. diameter of the left and right tape winding can overlap
	// we limit the max. diameter to be not intersecting.
	min_d		= round(23/*~20*/*ppmm);// min. diameter of tape on reel for tape winding diameter calculation
	max_d		= round(42.5*ppmm);		// max. diameter of tape on reel for tape winding diameter calculation (30 minutes)
	min_vis_d	= round(23*ppmm);		// min. visible diameter of tape on reel	TODO: depends on reel image
	max_vis_d	= round(39*ppmm);		// max. visible diameter of tape on reel	TODO: depends on reel image

	// text info fields:
	{
	// major block info text field:
		major_block_info_label->move(majorinfopos);
		major_block_info_label->setFont(QFont("Lucida Grande",11));
		setColors(major_block_info_label,0xffffff);
		major_block_info_label->setText(major_block_info);

	// minor block info text field:
		minor_block_info_label->move(minorinfopos);
		minor_block_info_label->setFont(QFont("Lucida Grande",11));
		setColors(minor_block_info_label,0xffffff);
		minor_block_info_label->setText(minor_block_info);

	// tape position counter:
		tape_position_label->move(tapecounterpos);
		QFont font = QFont("Menlo",12);
		//font.setWeight(QFont::Bold);
		font.setStretch(125);
		font.setLetterSpacing(QFont::AbsoluteSpacing,5);
		tape_position_label->setFont(font);
		setColors(tape_position_label,0xffcccccc);
		tape_position_label->setFixedSize(50,20);
		tape_position_label->setAlignment(Qt::AlignCenter);
		tape_position_label->setFrame(0);
		tape_position_label->setContentsMargins(0/*l*/,3/*o*/,-12/*r*/,2/*u*/);
		tape_position_label->setReadOnly(1);
		tape_position_label->setText("OOO");
	}

	// animation:
	{
		next_time_l			= 1e8;		// calculated time for next frame
		next_time_r			= 1e8;
		current_frame_l		= 0;		// index of currently displayed frame (0 ... 11)
		current_frame_r		= 0;
		current_dia_l		= 0;
		current_dia_r		= 0;

		timer->start(1000/60);
	}
}


void TapeRecorderInsp::paintEvent( QPaintEvent* )  // Qt
{
	xxlogIn("TapeRecorderInsp:paintEvent");

	QPainter p(this);

	p.drawImage(0,0,tr_image);

	if(anim_tr_loaded)
	{
	// Bandwickel:
		p.setRenderHint(QPainter::Antialiasing, true);
		p.setPen(Qt::NoPen);
		p.setBrush(QColor(68,54,60));		// dunkelbraun
		p.drawEllipse(QPointF(axis_x1,axis_y),current_dia_l*0.5,current_dia_l*0.5);
		p.drawEllipse(QPointF(axis_x2,axis_y),current_dia_r*0.5,current_dia_r*0.5);

	// Spulen:
		QImage& left_reel = cass_reel_images[current_frame_l%cass_reel_images.count()];
		p.drawImage(axis_x1-left_reel.width()/2, axis_y-left_reel.height()/2, left_reel);

		QImage& right_reel = cass_reel_images[current_frame_r%cass_reel_images.count()];
		p.drawImage(axis_x2-right_reel.width()/2, axis_y-right_reel.height()/2, right_reel);

	// Kassettengehäuse:
		QRect z = tr_window_rect.intersected(cass_rect);
		QRect q = z.translated(-cass_rect.topLeft());
		p.drawImage(z.topLeft(),cass_image,q);
	}

	// Achsen:
	QImage& left_axis = tr_axis_images[current_frame_l%tr_axis_images.count()];
	p.drawImage(axis_x1-left_axis.width()/2, axis_y-left_axis.height()/2, left_axis);

	QImage& right_axis = tr_axis_images[current_frame_r%tr_axis_images.count()];
	p.drawImage(axis_x2-right_axis.width()/2, axis_y-right_axis.height()/2, right_axis);

	// Deckel:
	p.drawImage(tr_window_rect,tr_window_image);
}

void TapeRecorderInsp::updateWidgets()
{
	// called by timer
	// This handles the animation and 6 buttons of the +2/+2A/TS2020
	// The WalkmanInspector reimplements this method for its 7 button control

	xxlogIn("TapeRecorderInsp::updateWidgets");

	if(!is_visible) return;
	if(!object) return;

	Inspector::updateWidgets();	// nop
	updateAnimation();

	volatile TapeRecorder* tr = tape_recorder();
	btn_record->setDown(tr->isRecordDown());
	btn_back->setDown(tr->isRewinding());
	btn_play->setDown(tr->isPlayDown());
	btn_fore->setDown(tr->isWinding());
	btn_pause->setDown(tr->isPauseDown());

	// update window title:
	// da die Machine ein Tape direkt einlegen kann und ich da (und sonstwo noch) nicht immer nach einem
	// Taperecorder Inspector suchen will, wird das hier gepollt => single place.
	// updateCustomTitle() ist mit dem ToolWindow verbunden, das danach getCustomTitle() aufruft.
	// updateWidgets() ist nochmal im WalkmanInspector überladen und dieser Code hier doppelt.
	cstr new_filepath = tape_recorder()->getFilepath();
	if(tape_filepath != new_filepath)
	{
		tape_filepath = new_filepath;
		emit updateCustomTitle();
	}
}

inline int TapeRecorderInsp::reel_diameter_for_seconds( Time seconds )
{
	// calculate diameter (in pixels) of tape on reel
	// this depends on the playing time of the tape on this reel

	//	assumes 0h45m fit on a reel
	//	uses: min_d and max_d
	//
	//	piv	= pi/4										pi/4
	//	v	= r^2*pi = (d/2)^2*pi = d^2 * piv			volume
	//
	//
	//	45 min == 1500*60*45 bits = 4,050,000 bits = 506,250 bytes  <->  full reel <->  d=max_d
	//	 0 min														<-> empty reel <->  d=min_d
	//
	//	b45 = 506,250									bytes per full reel  (45 minutes)
	//
	//	ppb = (max_d^2-min_d^2) *piv / b45				pixels per bytes	((volume/bytes))
	//
	//	iv	= min_d^2*piv								inner reel volume @ bytes=0
	//	tv	= iv + bytes*ppb							tape volume + inner reel volume
	//	tv  = d^2*piv
	//	<=> d^2*piv = iv + bytes*ppb
	//	<=> d^2 = ( iv + bytes*ppb ) /piv
	//			= iv/piv + bytes*ppb/piv
	//			= min_d^2*piv/piv + bytes*((max_d^2-min_d^2) *piv / b45)/piv
	//			= min_d^2         + bytes*((max_d^2-min_d^2)      / b45)
	//			= min_d^2 + bytes *(max_d^2-min_d^2) /b45

	return int ( 0.5+ sqrt( seconds * (max_d*max_d-min_d*min_d) / (30*60) + min_d*min_d ) );
}

inline Time TapeRecorderInsp::delay_to_next_frame_for_animation( int dia )
{
	// calcualte duration (in seconds) until next frame of animation
	// this depends on the reel diameter

	//	calculation of animation speed:
	//
	//	vl	= 47.5 mm/s					linear tape speed
	//	dd	= 42.5 mm					distance between axes
	//	u	= d * pi					circumference
	//	vr	= vl/u						rotational speed
	//	ptm	= dd / (axis_x2-axis_x1)	scaling factor pixels -> mm
	//
	//	fpr	= 36						frames per rotation
	//	tpr	= 1/vr						time/rotation
	//	tpf	= tpr/fpr					time/frame
	//		= 1/vr/36
	//		= 1/36/vl*vu
	//		= 1/36/47.5*d*pi					*s/mm
	//		= 1/36/47.5*pi*diam*ptm				*s/mm
	//		= 1/36/47.5*42.5*pi*diam/reel_d		*s
	//		= 1000/36/47.5*42.5*pi/reel_d*diam	*ms
	//
	return dia / 36.0 / 47.5 * 3.141592653589793 * 42.5 / (axis_x2-axis_x1);
}

void TapeRecorderInsp::updateAnimation()
{
	xxlogIn("TapeRecorderInsp::updateAnimation");

	if(!is_visible) return;

	NVPtr<TapeRecorder> tr(tape_recorder());

	// update text fields:
	{
		cstr s; int p;

		if(major_block_info != (s=tr->getMajorBlockInfo()))
		{
			major_block_info = s;
			major_block_info_label->setText(s?s:" ");
			major_block_info_label->adjustSize();
		}

		if(minor_block_info != (s=tr->getMinorBlockInfo()))
		{
			minor_block_info = s;
			minor_block_info_label->setText(s?s:" ");
			minor_block_info_label->adjustSize();
		}

		if(tape_position != (p=int(tr->getCurrentPosition())))
		{
			tape_position = p;
			char s[4] = "OOO";
			p=p%1000; if(p/100) s[0] = char('0'+p/100);
			p=p%100;  if(p/10)  s[1] = char('0'+p/10);
			p=p%10;   if(p)     s[2] = char('0'+p);
			tape_position_label->setText(s);
		}
	}

	// update animation

	Time now = system_time;		// seconds-based time

	// pause state change results in animation change only if tape loaded and playing:
	if(anim_tr_state != TapeRecorder::playing || !anim_tr_loaded)
	{
		anim_tr_pause = tape_recorder()->pause_is_down;
	}

	// animation change?
	if( anim_tr_loaded != tape_recorder()->isLoaded() ||
		anim_tr_pause  != tape_recorder()->pause_is_down ||
		anim_tr_state  != tape_recorder()->state )
	{
		anim_tr_loaded = tape_recorder()->isLoaded();
		anim_tr_pause  = tape_recorder()->pause_is_down;
		anim_tr_state  = tape_recorder()->state;
		next_time_l = next_time_r = now+0.05;
		update();
		return;
	}

	if(anim_tr_loaded)
	{
		int old_dia_l = current_dia_l;
		int old_dia_r = current_dia_r;

		if(cass.head_pos==head_down)
		{
			current_dia_r = reel_diameter_for_seconds(tr->getCurrentPosition());
			current_dia_l = reel_diameter_for_seconds(tr->getTotalPlaytime() - tr->getCurrentPosition());
		}
		else
		{
			current_dia_l = reel_diameter_for_seconds(tr->getCurrentPosition());
			current_dia_r = reel_diameter_for_seconds(tr->getTotalPlaytime() - tr->getCurrentPosition());
		}

		int d = 1;
		int f = 1;

		if(anim_tr_state == TapeRecorder::winding)
		{
			f = 2; d = 2;
		}
		else if(anim_tr_state == TapeRecorder::rewinding)
		{
			f = 2; d = 360-2;
		}
		else if(anim_tr_state != TapeRecorder::playing || anim_tr_pause)	// stopped
		{
			if(old_dia_l!=current_dia_l)
			{
				update(left_reel_rect.intersected(tr_window_rect).intersected(cass_window_rect));
			}
			if(old_dia_r!=current_dia_r)
			{
				update(right_reel_rect.intersected(tr_window_rect).intersected(cass_window_rect));
			}
			next_time_l = next_time_r = now+1e8;
			return;
		}

		if(now >= next_time_r)
		{
			next_time_r += delay_to_next_frame_for_animation(current_dia_r) / f;
			if( next_time_r<=now ) next_time_r = now + 2*FULLSPEED_SEC_PER_FRAME;
			current_frame_r = (current_frame_r+360-d) % 360;
			update(right_reel_rect.intersected(tr_window_rect).intersected(cass_window_rect));
		}
		if(now >= next_time_l)
		{
			next_time_l += delay_to_next_frame_for_animation(current_dia_l) / f;
			if( next_time_l<=now ) next_time_l = now + 2*FULLSPEED_SEC_PER_FRAME;
			current_frame_l = (current_frame_l+360-d) % 360;
			update(left_reel_rect.intersected(tr_window_rect).intersected(cass_window_rect));
		}
	}
	else	// no cassette loaded
	{
		if(anim_tr_state == TapeRecorder::winding || anim_tr_state == TapeRecorder::playing)
		{
			if(cass.head_pos==head_down) next_time_l = now+1e8; else next_time_r = now+1e8;
		}
		else if(anim_tr_state == TapeRecorder::rewinding)
		{
			if(cass.head_pos==head_down) next_time_r = now+1e8; else next_time_l = now+1e8;
		}
		else // stopped
		{
			next_time_l = next_time_r = now+1e8;
			return;
		}

		if(now >= next_time_r)
		{
			next_time_r += 2*FULLSPEED_SEC_PER_FRAME;
			if( next_time_r<=now ) next_time_r = now + 2*FULLSPEED_SEC_PER_FRAME;
			current_frame_r = cass.head_pos==head_down ? (current_frame_r+360-2) % 360 : (current_frame_r+2) % 360;
			update(right_axis_rect.intersected(tr_window_rect).intersected(cass_window_rect));
		}
		if(now >= next_time_l)
		{
			next_time_l += 2*FULLSPEED_SEC_PER_FRAME;
			if( next_time_l<=now ) next_time_l = now + 2*FULLSPEED_SEC_PER_FRAME;
			current_frame_l = cass.head_pos==head_down ? (current_frame_l+2) % 360 : (current_frame_l+360-2) % 360;
			update(left_axis_rect.intersected(tr_window_rect).intersected(cass_window_rect));
		}
	}
}

void TapeRecorderInsp::fillContextMenu(QMenu* menu)
{
	// Called by Inspector for right-click in inspector window

	Inspector::fillContextMenu(menu);

	if(tape_recorder()->isLoaded())
	{
		menu->addAction("Eject tape", this, &TapeRecorderInsp::eject_tape);
		menu->addAction("Save as …", this, &TapeRecorderInsp::save_as);

		QAction* action_wprot = new QAction("Write protected",menu);
		action_wprot->setCheckable(true);
		action_wprot->setChecked(tape_recorder()->isWriteProtected());
		connect(action_wprot,&QAction::toggled,this,&TapeRecorderInsp::set_wprot);
		menu->addAction(action_wprot);
	}
	else
	{
		menu->addAction("Insert empty tape", this, &TapeRecorderInsp::insert_empty_tape_w_anim);
		menu->addAction("Insert tape …", this, &TapeRecorderInsp::insert_tape_w_anim);
		menu->addAction("Recent tapes …")->setMenu(
				new RecentFilesMenu(tape_recorder()->list_id, this, [=](cstr path){insert_tape(path);}));
	}

	QAction* instantLoadAction = new QAction("Instant load/save tape",this);
	instantLoadAction->setCheckable(true);
	instantLoadAction->setChecked(tape_recorder()->instant_load_tape);
	connect(instantLoadAction, &QAction::toggled, this, [=](bool f){tape_recorder()->setInstantLoadTape(f);});
	menu->addAction(instantLoadAction);

	QAction* autoStartStopTape = new QAction("Auto start/stop tape",this);
	autoStartStopTape->setCheckable(true);
	autoStartStopTape->setChecked(tape_recorder()->auto_start_stop_tape);
	connect(autoStartStopTape, &QAction::toggled, this, [=](bool f){tape_recorder()->setAutoStartStopTape(f);});
	menu->addAction(autoStartStopTape);

	if(/*tape_recorder()->isLoaded() &&*/ tape_recorder()->isStopped() && !tape_recorder()->isWriteProtected())
	{
		menu->addSeparator();
		menu->addAction("Insert empty block before", this, [=]{nv_taperecorder()->newBlockBeforeCurrent();});
		QAction* a1 = menu->addAction("Insert empty block after",this,[=]{nv_taperecorder()->newBlockAfterCurrent();});
		QAction* a2 = menu->addAction("Delete current block", this, [=]{nv_taperecorder()->deleteCurrentBlock();});

		PLocker z(machine->_lock);

		TapeFile* tf = tape_recorder()->tapefile;
		if(tf->isLastBlock() && tf->getPlaytimeOfBlock()<0.5)
		{
			a1->setEnabled(no);
			a2->setEnabled(no);
		}
	}
}

cstr TapeRecorderInsp::getCustomTitle()
{
	return tape_filepath ? filename_from_path(tape_filepath) : nullptr;
}

cstr TapeRecorderInsp::get_save_filename(cstr msg) throws
{
	cstr filter = machine->isA(isa_MachineZxsp) ?
					"ZX Spectrum tapes (*.tap *.tape *.tzx);;"
	//				"Audio (*.aiff *.aif *.wav *.mp3);;"
					"All Files (*)"
				: machine->isA(isa_MachineZx81) ?
					"ZX81 tapes (*.tzx *.p *.81 *.p81);;"
	//				"Audio (*.aiff *.aif *.wav *.mp3);;"
					"All Files (*)"
				: machine->isA(isa_MachineZx80) ?
					"ZX80 tapes (*.tzx *.o *.80);;"
	//				"Audio (*.aiff *.aif *.wav *.mp3);;"
					"All Files (*)"
				: machine->isA(isa_MachineJupiter) ?
					"Jupiter Ace tapes (*.tap *.tape *.tzx);;"
	//				"Audio (*.aiff *.aif *.wav *.mp3);;"
					"All Files (*)"
				:
	//				"Audio (*.aiff *.aif *.wav *.mp3);;"
					"All Files (*)";

	return selectSaveFile(this,msg,filter);
}

cstr TapeRecorderInsp::get_load_filename(cstr msg) throws
{
	cstr filter = machine->isA(isa_MachineZxsp) ?
					"ZX Spectrum tapes (*.tap *.tape *.tzx);;"
					"Audio (*.aiff *.aif *.wav *.mp3);;"
					"All Files (*)"
				: machine->isA(isa_MachineZx81) ?
					"ZX81 tapes (*.tzx *.p *.81 *.p81);;"
					"Audio (*.aiff *.aif *.wav *.mp3);;"
					"All Files (*)"
				: machine->isA(isa_MachineZx80) ?
					"ZX80 tapes (*.tzx *.o *.80);;"
					"Audio (*.aiff *.aif *.wav *.mp3);;"
					"All Files (*)"
				: machine->isA(isa_MachineJupiter) ?
					"Jupiter Ace tapes (*.tap *.tape *.tzx);;"
					"Audio (*.aiff *.aif *.wav *.mp3);;"
					"All Files (*)"
				:	"Audio (*.aiff *.aif *.wav *.mp3);;"
					"All Files (*)";

	return selectLoadFile(this,msg,filter);
}

void TapeRecorderInsp::handleEjectButton()
{
	// with button animation & sound

	if(!tape_recorder()->isStopped())	// wenn Motor läuft, dann stoppen
	{
		nv_taperecorder()->stop();
	}
	else if(tape_recorder()->isLoaded())  // wenn Band eingelegt, dann auswerfen
	{
		eject_tape();
	}
	else								// sonst Band einlegen
	{
		eject_tape();					// wg. sound
		insert_tape(get_load_filename());
	}
	btn_eject->setDown(no);
}

void TapeRecorderInsp::insert_tape_w_anim()
{
	// ask for filename and load tape
	// with button animation & sound

	btn_eject->setDown(yes);
	eject_tape();					// wg. sound
	insert_tape(get_load_filename());
	btn_eject->setDown(no);
}

void TapeRecorderInsp::insert_empty_tape_w_anim()
{
	// ask for filename and create tape
	// with button animation & sound

	btn_eject->setDown(yes);
	eject_tape();				// wg. sound

	cstr filepath = get_save_filename("Insert new empty tape");
	if(filepath) try{ create_file(filepath); } catch(FileError&){}

	insert_tape(filepath);
	btn_eject->setDown(no);
}

void TapeRecorderInsp::save_as()
{
	// ask user for name and save tape
	// note: tapes must always have an associated file: in case the tape is modified
	//		 and destroyed there must be a file for saving without asking the user.

	assert(tape_recorder()->isLoaded());

	cstr filepath = get_save_filename();
	if(filepath)
	{
		tape_recorder()->setFilename(filepath);
		addRecentFile(tape_recorder()->list_id,filepath);
		addRecentFile(RecentFiles,filepath);
	}
}

void TapeRecorderInsp::set_wprot(bool f)
{
	// Toggle write protection state of tape
	// the wprot state is the wprot state of the tape file
	// TODO: set_file_writable() currently only checks the unix file mode

	assert(isMainThread());

	if(!tape_recorder()->tapefile) return;

	// Schreibschützen geht nicht, wenn schon was auf das Band geschrieben wurde:
	if(f && tape_recorder()->isModified())
	{
		showWarning("The tape is already modified.\n"
					"Use \"Save as…\" if you don't want to overwrite the original file.");
		return;
	}
	if(f && tape_recorder()->isRecordDown())
	{
		showWarning("The record button is down.\n" "Stop recording and try again.");
		return;
	}

	int err = tape_recorder()->setWriteProtected(f);
	if(err) showAlert(errorstr(err));
}

void TapeRecorderInsp::insert_tape(cstr filepath)
{
	assert(!tape_recorder()->isLoaded());

	TapeFile* tapefile = nullptr;

	if(filepath!=nullptr)
	{
		tapefile = new TapeFile(tape_recorder()->machine_ccps, filepath);
		xlogline("%s total length = %i sec", filepath, int(tapefile->getTotalPlaytime()));
		addRecentFile(tape_recorder()->list_id,filepath);
		addRecentFile(RecentFiles,filepath);
	}

	nv_taperecorder()->insert(tapefile);
}

void TapeRecorderInsp::eject_tape()
{
	TapeFile* tf = nv_taperecorder()->eject();
	delete tf;
}































