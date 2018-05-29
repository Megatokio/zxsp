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


#ifndef USBDEVICE_H
#define USBDEVICE_H

#include "mac_util.h"
//#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/usb/IOUSBLib.h>


typedef struct __CFDictionary * CFMutableDictionaryRef;
typedef const struct __CFDictionary * CFDictionaryRef;


// IOUSBInterfaceInterface		// 10.0
// IOUSBInterfaceInterface300	// 10.5
// IOUSBInterfaceInterface500	// 10.7.3
// IOUSBInterfaceInterface550	// 10.8.2
// IOUSBInterfaceInterface650	// 10.9

// IOUSBDeviceInterface
// IOUSBDeviceInterface320		// 10.5.4
// IOUSBDeviceInterface500		// 10.7.3

// IOHIDDeviceInterface
// IOHIDDeviceInterface122		// 10.3



// for compatibility with OS X 10.6:
typedef IOUSBInterfaceInterface300	MyUSBInterfaceInterface;
typedef IOUSBDeviceInterface320		MyUSBDeviceInterface;
typedef IOHIDDeviceInterface122		MyHIDDeviceInterface;



extern CFTypeRef	usbSearchProperty		(io_registry_entry_t, QCFString property_key, uint recursive);
extern cstr			usbSearchStringProperty(io_registry_entry_t, QCFString property_key, uint recursive);
extern uint16		usbSearchShortIntProperty(io_registry_entry_t, QCFString property_key, uint recursive, bool *ok);

extern CFMutableDictionaryRef	newMatchingDictForService(cstr service);
extern void						add(CFMutableDictionaryRef, cstr key, int32 value);
extern void						add(CFMutableDictionaryRef, cstr key, cstr value);
extern io_iterator_t			newIteratorForMatchingServices(CFDictionaryRef matchingDict);

extern MyUSBDeviceInterface**	newUSBDeviceInterfaceForDevice(io_service_t device);
extern MyHIDDeviceInterface**	newHIDDeviceInterfaceForService(io_service_t device);
extern MyUSBInterfaceInterface** newUSBInterfaceInterfaceForInterface(io_service_t interface);


extern void showSerialDevices();
extern void showUSBDevices();
extern void showUSBPrinters();
extern void testProlific2305();

class UsbDevice
{
public:
	UsbDevice();
	~UsbDevice();
};

#endif








