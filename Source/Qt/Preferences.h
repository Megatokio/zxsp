/*	Copyright  (c)	Günter Woigk 2012 - 2018
                    mailto:kio@little-bat.de

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    Permission to use, copy, modify, distribute, and sell this software and
    its documentation for any purpose is hereby granted without fee, provided
    that the above copyright notice appear in all copies and that both that
    copyright notice and this permission notice appear in supporting
    documentation, and that the name of the copyright holder not be used
    in advertising or publicity pertaining to distribution of the software
    without specific, written prior permission.  The copyright holder makes no
    representations about the suitability of this software for any purpose.
    It is provided "as is" without express or implied warranty.

    THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
    INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
    EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
    CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
    DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
    TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
    PERFORMANCE OF THIS SOFTWARE.
*/


#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QWidget>
#include <QRadioButton>
#include "kio/kio.h"
#include "ZxInfo.h"


class Preferences : public QWidget
{
    int modelList[num_models];

public:
    explicit Preferences(QWidget* parent = NULL);

private:
    void setSaveAndRestore(bool);
    void setDefaultScreenSize(int);
    void setStartOpenDiskDrive(bool);
    void setStartOpenKeyboard(bool);
    void setStartOpenTaperecorder(bool);
    void setStartOpenMachineImage(bool);
    void setStartEnableAudioIn(bool);
    void setStartStopTape(bool);
    void setFastLoadTape(bool);
    void setNewMachineModel(int);
    void setNewMachineKeyboardMode(int);
    void setNewMachineAy(bool);
    void setNewMachineJoystick(bool);
    void setNewMachineRampack(bool);
    void setNewMachineDivide(bool);
    void setSnapshotIndividualSettings(bool);
    void setSnapshotKeyboardMode(int);
    void setCheckForUpdate(bool);
};

#endif












