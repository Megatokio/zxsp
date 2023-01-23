#pragma once
// Copyright (c) 2015 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"


typedef const struct __CFString * CFStringRef;

extern str toStr(CFStringRef);

static_assert(_sizeof_pointer == _sizeof_long, "pointer size is assumed to be long");
typedef unsigned long CFTypeID;



extern CFTypeID type_dict;
extern CFTypeID type_array;
extern CFTypeID type_string;
extern CFTypeID type_number;
extern CFTypeID type_data;
extern CFTypeID type_date;
extern CFTypeID type_bool;



/*	Helper class that automates reference counting for CFTypes.
	After constructing the CFType object, it can be copied like a value-based type.

	based on Qt/.../QtSerialPort/private/qcore_mac_p.h

	Note that you must own the object you are wrapping.
	This is typically the case if you get the object from a Core Foundation function
	with the word "Create" or "Copy" in it. If you got the object from a "Get" function,
	either retain it or use constructFromGet(). One exception to this rule is the
	HIThemeGet*Shape functions, which in reality are "Copy" functions.
*/
template <typename T>
class QCFType
{
protected:
	T		type;
	void	retain		()							{ if(type) CFRetain(type); }
	void	release		()							{ if(type) CFRelease(type); }

public:
			QCFType		()							: type(NULL) {}
			QCFType		(const T& q)				: type(q) {}
			QCFType		(const QCFType& q)			: type(q.type) { retain(); }
			~QCFType	()							{ release(); }
	QCFType	operator=	(const QCFType& q)			{ q.retain(); release(); type = q.type; return *this; }

			operator T	()							{ return type; }
	T*		operator&	()							{ return &type; }

	bool	isNull		()	const					{ return type==NULL; }
	bool	isNotNull	()	const					{ return type!=NULL; }

	template <typename X> X as ()	const			{ return reinterpret_cast<X>(type); }

	static QCFType constructFromGet(const T& q)		{ CFRetain(q); return QCFType<T>(q); }
};



class QCFString : public QCFType<CFStringRef>
{
public:
	QCFString(cstr str);
	QCFString()										{}
	QCFString(const CFStringRef str)				: QCFType(str) {}
	QCFString(const QCFType<CFStringRef>& q)		: QCFType(q) {}

	operator str() const							{ return toStr(type); }
	operator CFStringRef() const					{ return type; }
};


























