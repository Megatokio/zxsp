#pragma once
// Copyright (c) 2015 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Inspector.h"
class QRect;
class QMouseEvent;
class QLabel;


class MultifaceInsp : public Inspector
{
protected:
	QRect		buttonbox;
	QLabel*		label_nmi_pending;
	QLabel*		label_paged_in;

	static const int l_x=4,l_y=3,l_d=14;

public:
	MultifaceInsp(QWidget*, MachineController*, volatile IsaObject *, cstr image, const QRect& redbuttonbox);

	void	pressRedButton();

protected:
	void	mousePressEvent(QMouseEvent*) override;
	void	updateWidgets() override;
};














