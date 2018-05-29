/*	Copyright  (c)	Günter Woigk 2015 - 2018
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















