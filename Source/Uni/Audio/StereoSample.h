/*	Copyright  (c)	Günter Woigk 2002 - 2018
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

#ifndef STEREOSAMPLE_H
#define STEREOSAMPLE_H


typedef float Sample;
typedef Sample const cSample;
class StereoSample;
typedef StereoSample const	cStereoSample;


class StereoSample
{
public:
	Sample	left,right;

	StereoSample() noexcept						: left(0), right(0) {}
	StereoSample(Sample m) noexcept				: left(m), right(m) {}
	StereoSample(Sample l, Sample r) noexcept	: left(l), right(r) {}
	StereoSample(cStereoSample& q) noexcept		: left(q.left), right(q.right) {}

StereoSample&	operator=	( Sample q )			noexcept	{ left=right=q; return *this; }
StereoSample&	operator=	( cStereoSample& q )	noexcept	{ left=q.left; right=q.right; return *this; }
StereoSample&	operator+=	( Sample q )			noexcept	{ left+=q; right+=q; return *this; }
StereoSample&	operator+=	( cStereoSample& q )	noexcept	{ left+=q.left; right+=q.right;	return *this; }
StereoSample&	operator-=	( Sample q )			noexcept	{ left-=q; right-=q; return *this; }
StereoSample&	operator-=	( cStereoSample& q )	noexcept	{ left-=q.left; right-=q.right;	return *this; }
StereoSample&	operator*=	( float f )				noexcept	{ left*=f; right*=f; return *this; }
StereoSample	operator*	( float f )		const	noexcept	{ return StereoSample(left*f,right*f); }
bool			operator>=	( Sample q )	const	noexcept	{ return mono()>=q; }
bool			operator<=	( Sample q )	const	noexcept	{ return mono()<=q; }
bool			operator<	( Sample q )	const	noexcept	{ return mono()<q;  }

Sample			mono		()				const	noexcept	{ return (left+right)/2.0f; }
};


#endif




