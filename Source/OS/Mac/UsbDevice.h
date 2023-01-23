#pragma once
// Copyright (c) 2015 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

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










