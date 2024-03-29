#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Inspector.h"
#include "MySimpleToggleButton.h"
#include "TapeRecorder.h"
#include "Templates/Array.h"
#include "Templates/NVPtr.h"
#include <QLabel>
#include <QLineEdit>
#include <QRect>
#include <QTimer>


namespace gui
{

enum CassBody { BlueBody };			 // available cassette bodies
enum HeadPos { head_up, head_down }; // magnetic head position


// helper class:
struct Cassette
{
	cstr hdgr_fname;
	cstr reel_fname;
	bool head_pos;

	int width; // Größe der Kassette (des Hintergrundbildes)
	int height;

	int axis_y; // Position der Achsen
	int axis_x1;
	int axis_x2;

	int window_x1; // Größe und Position des durchsichtigen Fensters
	int window_y1;
	int window_x2;
	int window_y2;

	int label_x; // Position des schwarzen Labels
	int label_y;

	int reel_symmetries;

	double ppmm;

	Cassette(CassBody, HeadPos);

	int windowX1() { return window_x1; }
	int windowY1() { return window_y1; }
	int windowX2() { return window_x2; }
	int windowY2() { return window_y2; }
	int windowWidth() { return window_x2 - window_x1; }
	int windowHeight() { return window_y2 - window_y1; }
};


// base class:
class TapeRecorderInsp : public Inspector
{
protected:
	volatile TapeRecorder* const tr;

	// buttons:
	MySimpleToggleButton* btn_record;
	MySimpleToggleButton* btn_play;
	MySimpleToggleButton* btn_back;
	MySimpleToggleButton* btn_fore;
	MySimpleToggleButton* btn_next;
	MySimpleToggleButton* btn_prev;
	MySimpleToggleButton* btn_eject;
	MySimpleToggleButton* btn_pause;

	// labels:
	cstr	   major_block_info; // displayed value
	cstr	   minor_block_info; // displayed value
	int		   tape_position;	 // displayed value
	QLabel*	   major_block_info_label;
	QLabel*	   minor_block_info_label;
	QLineEdit* tape_position_label;
	cstr	   tape_filepath;

	// animation:
	QImage			tr_image;		 // tape recorder image (skin)
	QImage			tr_window_image; // tape recorder window image
	QRect			tr_window_rect;	 // tape recorder window rect
	QVector<QImage> tr_axis_images;	 // !!! ObjArray
	Cassette		cass;
	QImage			cass_image;
	QRect			cass_rect;
	QRect			cass_window_rect;
	QVector<QImage> cass_reel_images;		  // !!! ObjArray
	int				axis_x1, axis_x2, axis_y; // axis positions (centers) in tape recorder image [pixels]
	double			ppmm;					  // scaling of tape recorder image [pixels per mm]

	QRect window_rect; // intersection of tr_window and cass_window for animation
	QRect left_reel_rect;
	QRect right_reel_rect;
	QRect left_axis_rect;
	QRect right_axis_rect;
	int	  axis_dia; // pixels; N*2
	int	  reel_dia; // pixels, N*2
	int	  min_d, max_d;
	int	  min_vis_d;
	int	  max_vis_d;

	Time next_time_l; // calculated time for next frame
	Time next_time_r;
	int	 current_frame_l; // index of currently displayed frame (0 ... 11)
	int	 current_frame_r;
	int	 current_dia_l;
	int	 current_dia_r;

	bool anim_tr_loaded; // cassette ist dargestellt
	bool anim_tr_pause;	 // modifiziert ggf. anim_tr_state
	int	 anim_tr_state;	 // '>>', '<<', '>' oder keine Animation

protected:
	TapeRecorderInsp(
		QWidget*, MachineController*, volatile TapeRecorder*, QPoint majorinfopos, QPoint minorinfopos,
		QPoint tapecounterpos, cstr hdgr_image_filename, cstr tray_image_filename, QPoint tray_position,
		HeadPos head_position, // head_up, head_down
		cstr axis_image_filename, int axis_symmetries, int tr_axis_x1, int tr_axis_x2, int tr_axis_y);

	void updateAnimation();
	void updateWidgets() override;			// timer
	void paintEvent(QPaintEvent*) override; // Qt
	void fillContextMenu(QMenu*) override;
	cstr getCustomTitle() override;

	void handleEjectButton();

	//NVPtr<TapeRecorder> nv_taperecorder() { return NVPtr<TapeRecorder>(dynamic_cast<volatile TapeRecorder&>(*object)); }

private:
	void insert_tape(cstr filepath);
	void insert_tape_w_anim();
	void insert_empty_tape_w_anim();
	void eject_tape();
	void save_as();
	void set_wprot(bool);
	int	 reel_diameter_for_seconds(Time);
	Time delay_to_next_frame_for_animation(int);
	cstr get_save_filename(cstr msg = "Save tape as…");
	cstr get_load_filename(cstr msg = "Insert tape file…");
};


class TS2020Inspector : public TapeRecorderInsp
{
public:
	TS2020Inspector(QWidget* parent, MachineController*, volatile TS2020* item);
};

class PlusTapeRecorderInsp : public TapeRecorderInsp
{
	// Base class for +2 and +2A tape recorder
	// same geometry and behaviour, just different shades of grey
protected:
	PlusTapeRecorderInsp(QWidget*, MachineController*, volatile TapeRecorder*, cstr hdgr, cstr lid, cstr btns);
};

class Plus2TapeRecorderInsp : public PlusTapeRecorderInsp
{
public:
	Plus2TapeRecorderInsp(QWidget*, MachineController*, volatile Plus2TapeRecorder*);
};

class Plus2aTapeRecorderInsp : public PlusTapeRecorderInsp
{
public:
	Plus2aTapeRecorderInsp(QWidget*, MachineController*, volatile Plus2aTapeRecorder*);
};

} // namespace gui
