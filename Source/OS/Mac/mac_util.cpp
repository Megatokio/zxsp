/*	Copyright  (c)	Günter Woigk 2015 - 2019
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

#include "kio/kio.h"
#include "mac_util.h"
#include <CoreFoundation/CoreFoundation.h>


CFTypeID type_dict;
CFTypeID type_array;
CFTypeID type_string;
CFTypeID type_number;
CFTypeID type_data;
CFTypeID type_date;
CFTypeID type_bool;

void init_CFTypeIDs()	// statische Initialisierer funktionieren nicht!
{
	type_dict   = CFDictionaryGetTypeID();
	type_array	= CFArrayGetTypeID();
	type_string = CFStringGetTypeID();
	type_number = CFNumberGetTypeID();
	type_data	= CFDataGetTypeID();
	type_date	= CFDateGetTypeID();
	type_bool	= CFBooleanGetTypeID();
}

ON_INIT(init_CFTypeIDs);



/*	helper: convert CFStringRef to str
	CFStringRef may be NULL
	returns str in tempmem or NULL
*/
str toStr(CFStringRef cfstr)
{
	if(cfstr==NULL) return NULL;
	uint32 cfstrlen = CFStringGetLength(cfstr);

	cstr s = CFStringGetCStringPtr(cfstr,kCFStringEncodingUTF8);
	if(s) return dupstr(s);

	for(uint n=1; n<=5; n++)
	{
		char* z = new char[cfstrlen*n+10];
		bool ok = CFStringGetCString(cfstr, z, cfstrlen*n+10, kCFStringEncodingUTF8);
		if(ok) { str s = dupstr(z); delete[] z; return s; }	else delete[] z;
	}

	logline("ERROR: convert CFStringRef to c-string failed");
	return NULL;
}



QCFString::QCFString(cstr str)
:
	QCFType<CFStringRef>(CFStringCreateWithCString(NULL/*allocator*/, str, kCFStringEncodingUTF8))
{}















