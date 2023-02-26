#pragma once
// Copyright (c) 2015 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Inspector.h"
class QRadioButton;


namespace gui
{

class CurrahMicroSpeechInsp : public Inspector
{
	volatile CurrahMicroSpeech* uspeech;

	QRadioButton* button_8bit;
	QRadioButton* button_hifi;

	// scroll text:
	uint8 scroller[64]; // circular buffer
	uint  rp;			// index of first allophone in scroller[] to print
	uint  wp;			// number of allophones in scroller[]
	int	  xpos;			// xpos of first allophone in scroller
	int	  width;		// total width [pixels] of scroller

public:
	CurrahMicroSpeechInsp(QWidget*, MachineController*, volatile CurrahMicroSpeech*);

protected:
	void paintEvent(QPaintEvent*) override;
	void updateWidgets() override;
};

} // namespace gui
