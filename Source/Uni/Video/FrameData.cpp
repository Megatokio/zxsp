// Copyright (c) 2025 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "FrameData.h"

namespace zxsp
{

/*	Lock for putting data into any FrameDataQueue:
	lock if multiple suppliers for one queue are possible (normally true)
	lock before even accessing a FrameDataQueue pointer if it can be changed
		by other hread (usually the gui thread)
*/
QMutex FrameDataQueue::mutex;

} // namespace zxsp
