// Copyright (c) 2015 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

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















