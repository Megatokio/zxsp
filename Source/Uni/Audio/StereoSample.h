/*	Copyright  (c)	Günter Woigk 2002 - 2019
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice, this permission notice and the
	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY,
	NOT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
	A PARTICULAR PURPOSE, AND IN NO EVENT SHALL THE COPYRIGHT HOLDER
	BE LIABLE FOR ANY DAMAGES ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
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




