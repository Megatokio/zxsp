#pragma once
// Copyright (c) 2015 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Inspector.h"
class QRect;
class QMouseEvent;
class QLabel;


namespace gui
{

class MultifaceInsp : public Inspector
{
protected:
	union
	{
		volatile Multiface*	   mf;
		volatile Multiface1*   mf1;
		volatile Multiface128* mf128;
		volatile Multiface3*   mf3;
	};

	QRect	buttonbox;
	QLabel* label_nmi_pending;
	QLabel* label_paged_in;

	static constexpr int l_x = 4, l_y = 3, l_d = 14;

public:
	MultifaceInsp(QWidget*, MachineController*, volatile Multiface*, cstr image, const QRect& redbuttonbox);

	void pressRedButton();

protected:
	void mousePressEvent(QMouseEvent*) override;
	void updateWidgets() override;
};

} // namespace gui
