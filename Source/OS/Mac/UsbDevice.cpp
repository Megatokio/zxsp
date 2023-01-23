// Copyright (c) 2015 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <paths.h>
#include <termios.h>
#include <sysexits.h>
#include <sys/param.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <CoreFoundation/CoreFoundation.h>

// user space:
#include <IOKit/serial/ioss.h>			// /System/Library/Frameworks/IOKit.framework/Headers/serial/ioss.h
#include <IOKit/serial/IOSerialKeys.h>	// /System/Library/Frameworks/IOKit.framework/Headers/serial/IOSerialKeys.h
#include <IOKit/usb/USB.h>
#include <IOKit/usb/USBSpec.h>			// /System/Library/Frameworks/IOKit.framework/Headers/usb/USBSpec.h
#include <IOKit/usb/IOUSBLib.h>			// /System/Library/Frameworks/IOKit.framework/Headers/usb/IOUSBLib.h
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hid/IOHIDUsageTables.h>
#include <IOKit/IOBSD.h>				// /System/Library/Frameworks/IOKit.framework/Headers/IOBSD.h
#include <IOKit/IOKitLib.h>				// /System/Library/Frameworks/IOKit.framework/Headers/IOKitLib.h
#include <IOKit/IOMessage.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/IOReturn.h>

#include "UsbDevice.h"
#include "kio/kio.h"
#include "Mac/mac_util.h"
#include "Templates/HashMap.h"


#include <IOKit/storage/IOStorageDeviceCharacteristics.h> // for kIOPropertyProductNameKey
//#define kIOPropertyProductNameKey				"Product Name"



/*	service:
	service_USBDevice		-> USB-Geräte ohne HID-Devices!
	service_USBInterface	-> Interfaces in USB-Geräten
	service_SerialBSDClient	-> serielle Geräte: USB und Bluetooth
	service_HIDDevice		-> USB-HID-Devices (gibt's auch andere?)
*/
cstr service_USBDevice			= kIOUSBDeviceClassName;		// "IOUSBDevice"		IOUSBLib.h
cstr service_USBInterface		= kIOUSBInterfaceClassName;		// "IOUSBInterface"		IOUSBLib.h
cstr service_SerialBSDClient	= kIOSerialBSDServiceValue;		// "IOSerialBSDClient"	IOSerialKeys.h
cstr service_HIDDevice			= kIOHIDDeviceKey;				// "IOHIDDevice"		IOHIDKeys.h


/*	property_key:
*/
#define key_VendorID			kUSBVendorID			// "idVendor"			USBSpec.h
#define key_ProductID			kUSBProductID			// "idProduct"
#define	key_DeviceClass			kUSBDeviceClass			// "bDeviceClass"
#define	key_DeviceSubClass		kUSBDeviceSubClass		// "bDeviceSubClass"
#define	key_DeviceProtocol		kUSBDeviceProtocol		// "bDeviceProtocol"
#define	key_DeviceMaxPacketSize	kUSBDeviceMaxPacketSize	// "bMaxPacketSize0"
#define	key_DeviceReleaseNumber	kUSBDeviceReleaseNumber	// "bcdDevice"
#define	key_ManufacturerStrIdx	kUSBManufacturerStringIndex // "iManufacturer"
#define	key_ProductStrIdx		kUSBProductStringIndex		// "iProduct"
#define	key_SerialNumberStrIdx	kUSBSerialNumberStringIndex	// "iSerialNumber"
#define	key_NumConfigurations	kUSBDeviceNumConfigs	// "bNumConfigurations"
#define	key_InterfaceNumber		kUSBInterfaceNumber		// "bInterfaceNumber"
#define	key_AlternateSetting	kUSBAlternateSetting	// "bAlternateSetting"
#define	key_NumEndpoints		kUSBNumEndpoints		// "bNumEndpoints"
#define	key_InterfaceClass		kUSBInterfaceClass		// "bInterfaceClass"
#define	key_InterfaceSubclass	kUSBInterfaceSubClass	// "bInterfaceSubClass"
#define	key_InterfaceProtocoll	kUSBInterfaceProtocol	// "bInterfaceProtocol"
#define	key_InterfaceStrIdx		kUSBInterfaceStringIndex// "iInterface"
#define	key_ConfigurationValue	kUSBConfigurationValue	// "bConfigurationValue"
#define	key_USBProductName		kUSBProductString		// "USB Product Name"
#define	key_USBVendorName		kUSBVendorString		// "USB Vendor Name"
#define	key_USBSerialNumber		kUSBSerialNumberString	// "USB Serial Number"
#define	key_1284DeviceID		kUSB1284DeviceID		// "1284 Device ID"
#define key_CalloutDevice		kIOCalloutDeviceKey		// "IOCalloutDevice"	IOKit/serial/IOSerialKeys.h
#define key_TTYDeviceName		kIOTTYDeviceKey			// "IOTTYDevice"		IOKit/serial/IOSerialKeys.h
#define key_ProductName			kIOPropertyProductNameKey // "Product Name"		IOStorageDeviceCharacteristics.h


/*	matching keys and values:
*/
#define kIOSerialBSDTypeKey		"IOSerialBSDClientType"		// IOSerialKeys.h
// currently possible values for kIOSerialBSDTypeKey:
#define kIOSerialBSDAllTypes	"IOSerialStream"
#define kIOSerialBSDModemType	"IOModemSerialStream"
#define kIOSerialBSDRS232Type	"IORS232SerialStream"


/*	vendorID and productID:
*/
static HashMap<uint16,cstr> usbVendorDB;		// key = vendorID
static HashMap<uint32,cstr> usbProductDB;		// key = vendorID<<16 + productID

static uint _prolific = 0x067b;
static uint _prolific_2305 = 0x2305;		// IEEE-1284 parallel interface

ON_INIT([]
{
	usbVendorDB.add(0x05AC, "Apple Inc.");
		usbProductDB.add(0x05AC8006,"EHCI Root Hub Simulation");		// iMac
		usbProductDB.add(0x05AC8005,"OHCI Root Hub Simulation");		// iMac
		usbProductDB.add(0x05AC4500,"BRCM2046 Hub");					// iMac
		usbProductDB.add(0x05AC8502,"Built-in iSight");				// iMac
		usbProductDB.add(0x05AC8403,"Internal Memory Card Reader");	// iMac
		usbProductDB.add(0x05AC8242,"IR Receiver");					// iMac
		usbProductDB.add(0x05AC8215,"Bluetooth USB Host Controller");	// iMac
	usbVendorDB.add(0x067b, "Prolific Inc.");
		usbProductDB.add(0x067b2305, "2305 IEEE-1284 Controller");
		usbProductDB.add(0x067b2303, "2303 Serial Controller");
	usbVendorDB.add(0x046D, "Logitech");
		usbProductDB.add(0x046DC505, "USB Receiver");					// Kbd + Mouse
	usbVendorDB.add(0x1058, "Western Digital");
		usbProductDB.add(0x10581001, "External HDD");
	usbVendorDB.add(0x0D8C, "(unknown)");
		usbProductDB.add(0x0D8C000C, "C-Media USB Headphone Set");		// Mic + Earphones
	usbVendorDB.add(0x07D0, "Dazzle Kingsun");
		usbProductDB.add(0x07D04959, "USB to IRDA");					// IRDA dongle
	usbVendorDB.add(0x0A12, "Cambridge Silicon Radio Ltd.");
		usbProductDB.add(0x0A120001, "Bluetooth USB Host Controller");	// Bluetooth dongle
	usbVendorDB.add(0x05E3, "Hama");
		usbProductDB.add(0x05E30715, "SD Card Reader");

	testProlific2305();		// TEST
//	showUSBPrinters();		// TEST
//	showSerialDevices();	// TEST
//	showUSBDevices();		// TEST
});




// --------------------------------------------------------------------
//			helpers: matching dictionary
//			• create a matching directory for a "service"
//			• add filter properties
//			• get an iterator for all matching services
// --------------------------------------------------------------------

inline
CFMutableDictionaryRef newMatchingDictForService(cstr service)
{
	return IOServiceMatching(service);	// NULL on error
}

void add(CFMutableDictionaryRef dict, cstr key, int32 value)
{
	QCFType<CFNumberRef> numberRef = CFNumberCreate(NULL/*allocator*/, kCFNumberSInt32Type, &value);
	CFDictionarySetValue(dict, QCFString(key), numberRef);
}

void add(CFMutableDictionaryRef dict, cstr key, cstr value)
{
	if(dict) CFDictionarySetValue(dict, QCFString(key), QCFString(value));
}

/*	get an iterator for all services matching a matching directory
	-->
		for(io_service_t device; (device = IOIteratorNext(iter)); IOObjectRelease(device))
		{
			// do something
		}
	   IOObjectRelease(iter);
*/
io_iterator_t newIteratorForMatchingServices(CFDictionaryRef matchingDict)
{
	if(matchingDict==NULL) return 0;
	io_iterator_t iter;
	kern_return_t err = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict, &iter);
	if(err) return 0;
	else return iter;
}



// --------------------------------------------------------------------
//			helpers: search property in io_object
// --------------------------------------------------------------------


/*	search property in registry_entry
	registry_entry may be an io_iterator_t
	recursive=0=no:								same as IORegistryEntryCreateCFProperty()
	recursive=1=kIORegistryIterateRecursively:	recursively childs in service plane
	recursive=2=kIORegistryIterateParents:		recursively parents in service plane
	returns NULL if not found
	There are six planes defined in the I/O Registry:
		Service
		Audio
		Power
		Device Tree
		FireWire
		USB
*/
CFTypeRef usbSearchProperty(io_registry_entry_t registry_entry, QCFString property_key, uint recursive)
{
	xlogline("searchProperty(%s)",(cstr)property_key);

	return IORegistryEntrySearchCFProperty(registry_entry, kIOUSBPlane/*kIOServicePlane*/, property_key,
										   NULL, recursive);
}

/*	search string property in registry entry recursively in service plane
	returns string property or NULL if not found
*/
cstr usbSearchStringProperty(io_registry_entry_t registry_entry, QCFString property_key, uint recursive)
{
	QCFType<CFTypeRef> value = usbSearchProperty(registry_entry, property_key, recursive);
	return toStr(value.as<CFStringRef>());
}

/*	search uint16 property from registry entry recursively in service plane
	returns uint16 property
	bool ok is set/cleared on success/error
*/
uint16 usbSearchShortIntProperty(io_registry_entry_t registry_entry, QCFString property_key, uint recursive, bool* ok)
{
	QCFType<CFTypeRef> prop = usbSearchProperty(registry_entry, property_key, recursive);
	uint32 value = 0;
	*ok = prop.isNotNull() && CFNumberGetValue(prop.as<CFNumberRef>(), kCFNumberSInt32Type, &value) && value<=0xffffu;
	return value;
}



// --------------------------------------------------------------------
//			helpers: create DeviceInterface
//			• wird für die Kommunikation mit dem Device benötigt
// --------------------------------------------------------------------

/*	Erzeuge ein USBDeviceInterface für einen Service
	Ein DeviceInterface wird benötigt um aus dem Userland mit dem Device zu kommunizieren.
	must be disposed by caller
*/
MyUSBDeviceInterface** newUSBDeviceInterfaceForDevice(io_service_t device)
{
	SInt32	score = 0;
	IOCFPlugInInterface** plugInInterface = NULL;

	kern_return_t kr = IOCreatePlugInInterfaceForService(device,
							kIOUSBDeviceUserClientTypeID,	// plugin type: for Device
							kIOCFPlugInInterfaceID,			// interface type
							&plugInInterface,				// the interface
							&score);						// the SCORE......

	if(kr!=KERN_SUCCESS) logline("IOCreatePlugInInterfaceForService returned 0x%08x", kr);
	if(plugInInterface==NULL) return NULL;

	MyUSBDeviceInterface** deviceInterface = NULL;
	HRESULT err = (*plugInInterface)->QueryInterface(plugInInterface,
										CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID),
										(void**) &deviceInterface);

	(*plugInInterface)->Release(plugInInterface);			// dispose plugin interface
	if(err!=S_OK) logline("QueryInterface returned %d", int(err));
	return deviceInterface;									// must be disposed by caller
}



/*	Erzeuge ein USBInterfaceInterface für ein Interface
	Ein InterfaceInterface wird benötigt um aus dem Userland mit dem Interface zu kommunizieren.
	must be disposed by caller
*/
MyUSBInterfaceInterface** newUSBInterfaceInterfaceForInterface(io_service_t interface)
{
	SInt32	score = 0;
	IOCFPlugInInterface** plugInInterface = NULL;

	kern_return_t kr = IOCreatePlugInInterfaceForService(interface,
							kIOUSBInterfaceUserClientTypeID,// plugin type: for Interface
							kIOCFPlugInInterfaceID,			// interface type
							&plugInInterface,				// the interface
							&score);						// the SCORE......

	if(kr!=KERN_SUCCESS) logline("IOCreatePlugInInterfaceForService returned 0x%08x", kr);
	if(plugInInterface==NULL) return NULL;

	MyUSBInterfaceInterface** deviceInterface = NULL;
	HRESULT err = (*plugInInterface)->QueryInterface(plugInInterface,
										CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID),
										(void**) &deviceInterface);

	(*plugInInterface)->Release(plugInInterface);			// dispose plugin interface
	if(err!=S_OK) logline("QueryInterface returned %d", int(err));
	return deviceInterface;									// must be disposed by caller
}



/*	Erzeuge ein HIDDeviceInterface für einen Service
	Ein DeviceInterface wird benötigt um aus dem Userland mit dem Device zu kommunizieren.
	must be disposed by caller
*/
MyHIDDeviceInterface** newHIDDeviceInterfaceForService(io_service_t device)
{
	SInt32	score = 0;
	IOCFPlugInInterface** plugInInterface = NULL;

	kern_return_t kr = IOCreatePlugInInterfaceForService(device,
							kIOHIDDeviceUserClientTypeID,	// plugin type
							kIOCFPlugInInterfaceID,			// interface type
							&plugInInterface,				// the interface
							&score);						// the SCORE......

	if(kr!=KERN_SUCCESS) logline("IOCreatePlugInInterfaceForService returned 0x%08x", kr);
	if(plugInInterface==NULL) return NULL;

	MyHIDDeviceInterface** deviceInterface;
	HRESULT err = (*plugInInterface)->QueryInterface(plugInInterface,
										CFUUIDGetUUIDBytes(kIOHIDDeviceInterfaceID),
										(void**) &deviceInterface);

	(*plugInInterface)->Release(plugInInterface);			// dispose plugin interface
	if(err!=S_OK) logline("QueryInterface returned %d", int(err));
	return deviceInterface;									// must be disposed by caller
}



// --------------------------------------------------------------------
//			helpers: misc.
// --------------------------------------------------------------------

static
cstr getDeviceName(io_registry_entry_t registry_entry, cstr* key)
{
	cstr name = usbSearchStringProperty(registry_entry, *key=key_ProductName, no);
	if(!name) name = usbSearchStringProperty(registry_entry, *key=key_USBProductName, no);
	if(!name) name = usbSearchStringProperty(registry_entry, *key="BTName", no);
	return name;
}

static
io_registry_entry_t parentService(io_registry_entry_t service)
{
	io_registry_entry_t parent = 0;
	IORegistryEntryGetParentEntry(service, kIOServicePlane, &parent);
	IOObjectRelease(service);
	return parent;
}

static
cstr usb_pipe_direction_str(uint direction)
{
	switch (direction)
	{
		case kUSBOut:	return "out";
		case kUSBIn:	return "in";
		case kUSBNone:	return "none";
		case kUSBAnyDirn:return "any";
		default:		return "???";
	}
}

static
cstr usp_pipe_transfertype_str(uint transferType)
{
	switch (transferType)
	{
		case kUSBControl:	return "control";
		case kUSBIsoc:		return "isoc";
		case kUSBBulk:		return "bulk";
		case kUSBInterrupt:	return "interrupt";
		case kUSBAnyType:	return "any";
		default:			return "???";
	}
}


void logValue(CFTypeRef value);
void logDict(CFTypeRef value);

void logNumber(CFTypeRef value)
{
	CFNumberRef number = reinterpret_cast<CFNumberRef>(value);
	if(CFNumberIsFloatType(number))
	{
		float64 f;
		/*Boolean ok =*/ CFNumberGetValue(number,kCFNumberFloat64Type,&f);
		logline("%s",tostr(f));
	}
	else if(CFNumberGetByteSize(number)>4)
	{
		int64 n;
		/*Boolean ok =*/ CFNumberGetValue(number,kCFNumberSInt64Type,&n);
		logline("0x%s (%s)",hexstr(n,16),tostr(n));
	}
	else
	{
		int32 n;
		/*Boolean ok =*/ CFNumberGetValue(number,kCFNumberSInt32Type,&n);
		logline("0x%s (%s)",hexstr(n,n>0xffff?8:4),tostr(n));
	}
}

void logString(CFTypeRef value)
{
	logline("%s",quotedstr(toStr(reinterpret_cast<CFStringRef>(value))));
}

void logArray(CFTypeRef value)
{
	CFArrayRef array = reinterpret_cast<CFArrayRef>(value);

	uint size = CFArrayGetCount(array);
	logIn("Array[%u]", size);
	for(uint i=0;i<size;i++)
	{
		logValue(CFArrayGetValueAtIndex(array,i));
	}
}

void logDict(CFTypeRef value)
{
	CFDictionaryRef dict = reinterpret_cast<CFDictionaryRef>(value);

	uint size = CFDictionaryGetCount(dict);
	Array<CFTypeRef> keys(size);
	Array<CFTypeRef> values(size);
	CFDictionaryGetKeysAndValues(dict,keys.getData(),values.getData());
	logIn("Dictionary[%u]", size);

	uint maxlen = 0;
	for(uint i=0;i<size;i++) maxlen=max(maxlen,uint(CFStringGetLength(reinterpret_cast<CFStringRef>(keys[i]))));
	cstr spaces = spacestr(maxlen);

	for(uint i=0;i<size;i++)
	{
		CFStringRef key = reinterpret_cast<CFStringRef>(keys[i]);
		log("%s%s = ",toStr(key),spaces+CFStringGetLength(key));
		logValue(values[i]);
	}
}

void logValue(CFTypeRef value)
{
	CFTypeID tid = CFGetTypeID(value);

	if(tid==type_string)		logString(value);
	else if(tid==type_number)	logNumber(value);
	else if(tid==type_array)	logArray(value);
	else if(tid==type_dict)		logDict(value);
	else						logline("todo");
}


#if 0
void logDeviceInformationBits(uint32 n)
{
	logIn("Device information bits:");

	// note: USBDeviceInformationBits are enumerated in USB.h line 1260ff.
	static cstr msg[] =
	{
	"IsCaptive: The USB device is directly attached to its hub and cannot be removed.",
	"IsAttachedToRootHub: The USB device is directly attached to the root hub",
	"IsInternal: The USB device is internal to the enclosure (all the hubs it attaches to are captive)",
	"IsConnected: The USB device is connected to its hub",
	"IsEnabled: The hub port to which the USB device is attached is enabled",
	"IsSuspended: The hub port to which the USB device is attached is suspended",
	"IsInReset: The hub port to which the USB device is attached is being reset",
	"Overcurrent: The USB device generated an overcurrent",
	"PortIsInTestMode: The hub port to which the USB device is attached is in test mode",
	"IsRootHub: The device is the root hub simulation",
	"IsBuiltInRootHub: this is a root hub simulation and it's built into the enclosure. (it's not on an expansion card.)",
	"IsRemote: This device is attached to the controller through a remote connection",
	"IsAttachedToEnclosure: The hub port to which the USB device is connected has a USB connector on the enclosure",
	"IsOnThunderbolt: The USB device is downstream of a controller that is attached through Thunderbolt",
	};

	uint i=0;
	for(;i<NELEM(msg);i++) { if(n&(1<<i)) logline("%s", msg[i]); }
	for(;i<32;i++) { if(n&(1<<i)) logline( "bit %u is set", i ); }
}
#endif


/*	configure device to use configuration_idx
	note: this invalidates all interfaces!
*/
IOReturn ConfigureDevice(MyUSBDeviceInterface **dev, uint configuration_idx)
{
	uint8		num_configurations;
	IOReturn	err;
	IOUSBConfigurationDescriptorPtr configuration_descriptor;

	// Get the number of configurations.
	err = (*dev)->GetNumberOfConfigurations(dev, &num_configurations);		assert(!err);
	if(configuration_idx >= num_configurations) return error;

	// Get the configuration descriptor
	err = (*dev)->GetConfigurationDescriptorPtr(dev, configuration_idx, &configuration_descriptor);
	if(err) { logline("GetConfigurationDescriptorPtr: error = 0x%08X", err); return err; }

	// Set the device’s configuration. The configuration value is found in
	// the bConfigurationValue field of the configuration descriptor
	err = (*dev)->SetConfiguration(dev, configuration_descriptor->bConfigurationValue);
	if(err) { logline("SetConfiguration: error = 0x%08X", err); return err; }

	return ok;
}


IOReturn writeToDevice(IOUSBDeviceInterface **dev, UInt16 deviceAddress, UInt16 length, UInt8 writeBuffer[])
{
	IOUSBDevRequest request;

	request.bmRequestType = USBmakebmRequestType(kUSBOut, kUSBVendor, kUSBDevice);
	request.bRequest = 0xa0;
	request.wValue = deviceAddress;
	request.wIndex = 0;
	request.wLength = length;
	request.pData = writeBuffer;

	return (*dev)->DeviceRequest(dev, &request);
}


#if 0
/*	struct IOUSBDevRequest
	{
		UInt8       bmRequestType;	Request type: kUSBStandard, kUSBClass or kUSBVendor
		UInt8       bRequest;		Request code
		UInt16      wValue;			16 bit parameter for request, host endianess
		UInt16      wIndex;			16 bit parameter for request, host endianess
		UInt16      wLength;		Length of data part of request, 16 bits, host endianess
		void *      pData;			Pointer to data for request - data returned in bus endianess
		UInt32      wLenDone;		Set by standard completion routine to number of data bytes actually transferred
	};
*/
IOReturn printerGetIEEEDeviceDescription(IOUSBDeviceInterface** dev_if,
										uint configuration_idx, uint interface_and_alt_idx, cstr* result)
{
	uint8 bu[256];

	IOUSBDevRequest request;
	request.bmRequestType = 0b10100001;			// bit fields: %DTTRRRRR D=Dir[0=out,1=in], T=Type, R=Recipient
	request.bRequest = 0;						// 0 = GET_DEVICE_ID
	request.wValue = configuration_idx;			// mostly 0
	request.wIndex = interface_and_alt_idx;		// mostly 0
	request.wLength = 256;
	request.pData = bu;

	*result = NULL;
	IOReturn err = (*dev_if)->DeviceRequest(dev_if, &request);
	if(err) return err;
	uint numBytesReceived = request.wLenDone;
	if(numBytesReceived<=2) { logline("IEEE Device ID returned 0 bytes"); return err; }

	uint len = peek2X(bu)-2;
	assert(len<255);
	*result = substr(bu+2,bu+2+len);
	return err;
}
#endif


IOReturn printerGetPortStatus(MyUSBDeviceInterface **dev, uint interface_idx, uint8* result)
{
	IOUSBDevRequest request;
	request.bmRequestType = 0b10100001;			// bit fields: %DTTRRRRR D=Dir[0=out,1=in], T=Type, R=Recipient
	request.bRequest = 1;						// 1 = GET_PORT_STATUS
	request.wValue = 0;
	request.wIndex = interface_idx;
	request.wLength = 1;
	request.pData = result;

	IOReturn err = (*dev)->DeviceRequest(dev, &request);
	assert(err || request.wLenDone==1);
	return err;
}

int testProlific2305pipe(MyUSBDeviceInterface** dev_if, MyUSBInterfaceInterface** if_if, int pipe_idx)
{
	logIn("testProlific2305pipe()");

	assert(pipe_idx==1);

	uint16	maxPacketSize;
	uint8	direction, number, transferType, interval, status, interface_idx;
	IOReturn err;

	err = (*if_if)->GetPipeProperties(if_if,
								pipe_idx, &direction,
								&number, &transferType,
								&maxPacketSize, &interval);
	if(err) { logline("GetPipeProperties(%d) error: 0x%08X", pipe_idx, err); return -1; }

	logline("Pipe %d: direction %s, transferType %s, maxPacketSize %d",
								pipe_idx,
								usb_pipe_direction_str(direction),
								usp_pipe_transfertype_str(transferType),
								maxPacketSize);

	err = (*if_if)->GetPipeStatus(if_if, pipe_idx);
	if(err) { logline("GetPipeStatus error: 0x%08X", uint(err)); return -1; }

	err = (*if_if)->GetInterfaceNumber(if_if,&interface_idx);
	if(err) { logline("GetInterfaceNumber error: 0x%08X",uint(err)); return -1; }
	assert(interface_idx==0);

//	err = (*if_if)->ClearPipeStallBothEnds(if_if,pipe_idx);
//	if(err) { logline("ClearPipeStall error: 0x%08X",uint(err)); return -1; }

//	err = printerGetPortStatus(dev_if, interface_idx, &status);
//	if(err) { logline("printerGetPortStatus error: 0x%08X",uint(err)); return -1; }
//	if(status!=0x18) logline("Printer status = 0x%02X", status);		// 0x18 = no error, selected, paper ok

//	err = (*if_if)->ClearPipeStallBothEnds(if_if,pipe_idx);
//	if(err) { logline("ClearPipeStall #2 error: 0x%08X",uint(err)); return -1; }

	cstr msg = "Hallo schwarzer Peter\n\r";
	err = (*if_if)->WritePipe(if_if, pipe_idx, (void*)msg, strlen(msg)); // 0xE00002ed device not responding
	if(err)
	{
		logline("WritePipe: error 0x%08X", uint(err));

		err = printerGetPortStatus(dev_if, 0/*interface*/, &status);
		if(err) logline("printerGetPortStatus error: 0x%08X",uint(err));

		err = (*if_if)->GetPipeStatus(if_if, pipe_idx);
		if(err==kIOUSBPipeStalled) logline("pipe status = Stalled");
		else if(err) logline("GetPipeStatus error: 0x%08X", uint(err));

		logline("bulk write to pipe failed");
		return -1;
	}
	return 0; // 0 = ok
}


int testProlific2305interface(MyUSBDeviceInterface** dev_if, MyUSBInterfaceInterface** if_if)
{
	logIn("testProlific2305interface()");

	IOReturn err;
	int r = 0;
	uint8 if_class, if_subclass, if_protocol, if_alt_setting, if_num_endpoints;

	//Get interface class and subclass:	should be 7, 1
	err = (*if_if)->GetInterfaceClass(if_if, &if_class);
	if(err) { logline("GetInterfaceClass error: 0x%08X",uint(err)); return -1; }
	err = (*if_if)->GetInterfaceSubClass(if_if, &if_subclass);
	if(err) { logline("GetInterfaceSubClass error: 0x%08X",uint(err)); return -1; }
	logline("Interface class %d, subclass %d", if_class, if_subclass);

	err = (*if_if)->GetInterfaceProtocol(if_if, &if_protocol);
	if(err) { logline("GetInterfaceProtocol error: 0x%08X",uint(err)); return -1; }
	logline("Interface protocol: %d", if_protocol);

	err = (*if_if)->GetAlternateSetting(if_if, &if_alt_setting);
	if(err) { logline("GetAlternateSetting error: 0x%08X",uint(err)); return -1; }
	logline("Interface alternate setting in use: %d", if_alt_setting);
	//IOReturn (*SetAlternateInterface)(void *self, UInt8 alternateSetting);

//	err = (*if_if)->SetAlternateInterface(if_if, 1);
//	if(err) { logline("SetAlternateSetting error: 0x%08X",uint(err)); return -1; }	// 0xE00002CD


	// open interface:
	err = (*if_if)->USBInterfaceOpen(if_if);
	if(err) { logline("USBInterfaceOpen error: 0x%08X", uint(err)); return -1; }

	err = (*if_if)->SetAlternateInterface(if_if, 0);
	if(err) { logline("SetAlternateSetting error: 0x%08X",uint(err));
	return -1; }	// 0xE00002CD


	// Get the number of endpoints associated with this interface in this alt setting:
	err = (*if_if)->GetNumEndpoints(if_if, &if_num_endpoints);
	if(err) { logline("GetNumEndpoints returned error 0x%08X", uint(err)); goto x; }
	logline("Interface num endpoints = %u",if_num_endpoints);

	// Access each pipe in turn, starting with the pipe at index 1
	// The pipe at index 0 is the default control pipe and should be
	// accessed using (*usbDevice)->DeviceRequest() instead

	for(int pipeRef = 1; pipeRef <= if_num_endpoints; pipeRef++)
	{
		logline("found pipe #%i",pipeRef);
		r |= testProlific2305pipe(dev_if,if_if,pipeRef);
	}

x:	err = (*if_if)->USBInterfaceClose(if_if); if_if=0;
	if(err) { logline("USBInterfaceClose error: 0x%08X", uint(err)); r = -1; }

	return r; // ok
}


int testProlific2305device(io_service_t device)
{
	logIn("testProlific2305device()");

	// general error codes in IOReturn.h
	// usb error codes in USB.h line 360ff
	IOReturn err;
	int r = 0;
	uint8 num_configs = 0;

	// loop over all available configurations:
	// note: the 2305 has exactly one.

	MyUSBDeviceInterface** dev_if = newUSBDeviceInterfaceForDevice(device);
	if(!dev_if) { logline("dev_if = NULL"); return -1; }

	err = (*dev_if)->USBDeviceOpen(dev_if);
	if(err) { logline("USBDeviceOpen error: 0x%08X",uint(err)); goto xx; }

	err = (*dev_if)->GetNumberOfConfigurations(dev_if,&num_configs);
	if(err) { logline("GetNumberOfConfigurations error: 0x%08X",uint(err)); goto x; }
	logline("number of configurations: %u",num_configs);

	for(uint config=0; config<num_configs; config++)
	{
		logIn("Testing Configuration #%u",config);

		err = ConfigureDevice(dev_if, config);
		if(err) { logline("ConfigureDevice error: 0x%08X",uint(err)); r = -1; continue; }

		// get interface(s):

		IOUSBFindInterfaceRequest request;
		request.bInterfaceClass    = kIOUSBFindInterfaceDontCare;
		request.bInterfaceSubClass = kIOUSBFindInterfaceDontCare;
		request.bInterfaceProtocol = kIOUSBFindInterfaceDontCare;
		request.bAlternateSetting  = kIOUSBFindInterfaceDontCare;

		io_iterator_t iterator;
		err = (*dev_if)->CreateInterfaceIterator(dev_if, &request, &iterator);
		assert(!err);//if(err) { logline("CreateInterfaceIterator: error 0x%08X",uint(err)); r = -1; continue; }

		// loop over all interfaces:

		for(io_service_t interface; (interface = IOIteratorNext(iterator)); IOObjectRelease(interface))
		{
			MyUSBInterfaceInterface** if_if = newUSBInterfaceInterfaceForInterface(interface);
			if(!if_if) { logline("if_if = NULL"); r = -1; continue; }

			uint8 if_idx;
			err = (*if_if)->GetInterfaceNumber(if_if,&if_idx);
			if(err) { logline("GetInterfaceNumber error: 0x%08X",uint(err)); r = -1; continue; }
			logline("Found Interface %u",if_idx);

			r |= testProlific2305interface(dev_if,if_if);
		}
	}

x:	if(err) r = -1;
	err = (*dev_if)->USBDeviceClose(dev_if);
	if(err) { logline("USBDeviceClose: error 0x%08X",uint(err)); r = -1; }
xx:	//CFRelease(dev_if); dev_if=NULL;	// TODO OR NOT TODO...

	return r;	// 0 = ok
}





void testProlific2305()
{
	logIn("testProlific2305()");

	// get device(s)

	CFMutableDictionaryRef dict = newMatchingDictForService(service_USBDevice);
	add(dict,key_VendorID,_prolific);
	add(dict,key_ProductID,_prolific_2305);

	io_iterator_t iter = newIteratorForMatchingServices(dict);
	uint num = 0;

	// loop over all devices:

	for(io_service_t device; (device = IOIteratorNext(iter)); IOObjectRelease(device))
	{
		logline("Found a Device:");
		num++;
		int r = testProlific2305device(device);
		(void)r; // 0 = ok
	}

	if(num==0) logline("no Prolific 2305 IEEE-1284 adapter found");

	IOObjectRelease(iter);
}








/*	IOKitLib implements non-kernel task access to common IOKit object types - IORegistryEntry, IOService,
	IOIterator etc. These functions are generic - families may provide API that is more specific.

	IOKitLib represents IOKit objects outside the kernel with the types io_object_t, io_registry_entry_t,
	io_service_t, and io_connect_t. Function names usually begin with the type of object they are
	compatible with - eg. IOObjectRelease can be used with any io_object_t.
	Inside the kernel, the c++ class hierarchy allows the subclasses of each object type to receive
	the same requests from user level clients, for example in the kernel,
	IOService is a subclass of IORegistryEntry, which means any of the IORegistryEntryXXX functions
	in IOKitLib may be used with io_service_t's as well as io_registry_t's.
	There are functions available to introspect the class of the kernel object which any io_object_t et al.
	represents. IOKit objects returned by all functions should be released with IOObjectRelease.

https://developer.apple.com/library/mac/documentation/DeviceDrivers/Conceptual/IOKitFundamentals/Families_Ref/Families_Ref.html#//apple_ref/doc/uid/TP0000021-BABCCBIJ
	OSObject
	+-IORegistryEntry
	| +-IOService
	|
	+-IOHiKeyboardMapper
	+-IOService
	| +-IOHIDevice
	| | +-IOHIKeyboard
	| | +-IOTabletPointer
	| | +-IOHIPointing
	| |   +-IOHITablet
	| +-IOHIDSystem
	|
	+-IOService
	| +-IOSerialDriverSync
	| +-IOSerialStreamSync
	|   +-IORS232SerialStreamSync
	|     +-IOModemSerialStreamSync
	|
	+-IOUSBPipe
	+-IOService
	| +-IOUSBBus
	| | +-IOUSBController
	| +-IOUSBNub
	| | +-IOUSBDevice
	| | +-IOUSBInterface
	| +-IOUSBHub
	| +-IOUSBLog
	|
	+---IOSCSIProtocolServices
	|   +-IOUSBMassStorageClass
	|     +-IOUSBMassStorageUFISubclass
	|
*/




#if 0
/*	get an iterator for a certain service
	and for a certain vendor and product
*/
io_iterator_t newIteratorForMatchingServices(cstr service, int32 vendorID, int32 productID, int32 /*bcdDevice*/,
							int32 /*bInterfaceNumber*/, int32 /*bConfigurationValue*/)
{
	/*	Set up the matching criteria for the devices we're interested in. The matching criteria needs to follow
		the same rules as kernel drivers: mainly it needs to follow the USB Common Class Specification, pp. 6-7.
		See also Technical Q&A QA1076 "Tips on USB driver matching on Mac OS X"
		<http://developer.apple.com/qa/qa2001/qa1076.html>.
		One exception is that you can use the matching dictionary "as is", i.e. without adding any matching
		criteria to it and it will match every IOUSBDevice in the system.
	*/
	CFMutableDictionaryRef dict = newMatchingDictForService(service);	// Interested in instances of class
	if(dict==NULL) return 0;											// IOUSBDevice and its subclasses

// fill in matching criteria (if any):

	/*	We are interested in all USB devices (as opposed to USB interfaces).  The Common Class Specification
		tells us that we need to specify the idVendor, idProduct, and bcdDevice fields, or, if we're not interested
		in particular bcdDevices, just the idVendor and idProduct.  Note that if we were trying to match an
		IOUSBInterface, we would need to set more values in the matching dictionary (e.g. idVendor, idProduct,
		bInterfaceNumber and bConfigurationValue.
	*/
	if(vendorID!=0)  add(dict,kUSBVendorID,vendorID);
	if(productID!=0) add(dict,kUSBProductID,productID);

// Now we have a dictionary, get an iterator:
	return newIteratorForMatchingServices(dict);
}
#endif




void printSomething(io_registry_entry_t interface)
{
	// general error codes in IOReturn.h
	// usb error codes in USB.h line 360ff
	IOReturn err;

	MyUSBInterfaceInterface** if_if = newUSBInterfaceInterfaceForInterface(interface);
	if(!if_if) return logline("if_if = NULL");

	io_service_t device;
	err = (*if_if)->GetDevice(if_if, &device);
	if(err) return logline("GetDevice returned error 0x%08X",uint(err));

	MyUSBDeviceInterface** dev_if = newUSBDeviceInterfaceForDevice(device);
	if(!dev_if) return logline("dev_if = NULL");

	err = (*dev_if)->USBDeviceOpen(dev_if);
	if(err) return logline("USBDeviceOpen returned error 0x%08X",uint(err));

	err = ConfigureDevice(dev_if,0);
	if(err) return logline("ConfigureDevice returned error 0x%08X",uint(err));

	err = (*dev_if)->USBDeviceClose(dev_if);
	if(err) return logline("USBDeviceClose returned error 0x%08X",uint(err));




//	uint32 info = 0;
//	err = (*dev_if)->GetUSBDeviceInformation(dev_if, &info);
//	if(err) logline("GetUSBDeviceInformation returned error 0x%08X",uint(err));	// -> 0xe000405f = kIOUSBNoAsyncPortErr
//	else    logDeviceInformationBits(info);

//	uint32 locationID;		// "locationID"
//	err = (*dev_if)->GetLocationID(dev_if, &locationID);
//	if(err) logline("GetLocationID returned error 0x%08X", uint(err));
//	else logline("Location ID: 0x%08X", uint(locationID));

//	uint8 configNum;		// "bConfigurationValue"
//	err = (*dev_if)->GetConfiguration(dev_if, &configNum);
//	if(err) logline("GetConfiguration returned error 0x%08X", uint(err));	// -> 0xE00002CD = device not open
//	else logline("Location ID: 0x%02X", uint(configNum));

//	err = (*dev_if)->USBInterfaceOpen(dev_if);
//	if(err) return logline("USBDeviceOpen returned error 0x%08X", uint(err));	// OK
//	else logline("Device OPEN");

//	err = (*dev_if)->GetConfiguration(dev_if, &configNum);
//	if(err) logline("GetConfiguration returned error 0x%08X", uint(err));	// -> 0xE000404E = Interface ref not recognized
//	else logline("Location ID: 0x%02X", uint(configNum));

//	err = (*dev_if)->GetUSBDeviceInformation(dev_if, &info);
//	if(err) logline("GetUSBDeviceInformation returned error 0x%08X",uint(err));	// -> 0xe000405f = kIOUSBNoAsyncPortErr
//	else    logDeviceInformationBits(info);

//	err = (*dev_if)->USBInterfaceClose(dev_if);
//	if(err) logline("USBDeviceClose returned error 0x%08X", uint(err));		// OK
//	else logline("Device CLOSED");

	{
		// open interface:
		err = (*if_if)->USBInterfaceOpen(if_if);
		if(err) return logline("USBDeviceOpen returned error 0x%08X", uint(err));	// OK
		else logline("Device OPEN");

		// Get the number of endpoints associated with this interface
		uint8 interfaceNumEndpoints;
		err = (*if_if)->GetNumEndpoints(if_if, &interfaceNumEndpoints);
		if(err!=kIOReturnSuccess) { logline("GetNumEndpoints returned error 0x%08X", uint(err)); goto x; }
		else logline("interfaceNumEndpoints = %u",interfaceNumEndpoints);

		// Access each pipe in turn, starting with the pipe at index 1
		// The pipe at index 0 is the default control pipe and should be
		// accessed using (*usbDevice)->DeviceRequest() instead

		for(int pipeRef = 1; pipeRef <= interfaceNumEndpoints; pipeRef++)
		{
			uint8	direction;
			uint8	number;
			uint8	transferType;
			uint16	maxPacketSize;
			uint8	interval;
			cstr	message;

			err = (*if_if)->GetPipeProperties(if_if,
										pipeRef, &direction,
										&number, &transferType,
										&maxPacketSize, &interval);
			if (err != kIOReturnSuccess)
			{
				logline("InterfaceGetPipeProperties(%d) returned error 0x%08X", pipeRef, err);
				continue;
			}

			log("PipeRef %d: ", pipeRef);
			switch (direction)
			{
				case kUSBOut:	message = "out";	break;
				case kUSBIn:	message = "in";		break;
				case kUSBNone:	message = "none";	break;
				case kUSBAnyDirn:message = "any";	break;
				default:		message = "???";	break;
			}
			log("direction %s, ", message);

			switch (transferType)
			{
				case kUSBControl:	message = "control";	break;
				case kUSBIsoc:		message = "isoc";		break;
				case kUSBBulk:		message = "bulk";		break;
				case kUSBInterrupt:	message = "interrupt";	break;
				case kUSBAnyType:	message = "any";		break;
				default:			message = "???";		break;
			}
			logline("transfer type %s, maxPacketSize %d", message, maxPacketSize);

			err = (*if_if)->GetPipeStatus(if_if, pipeRef);
			if(err==kIOReturnNoDevice) logline("pipe status = No Device");
			else if(err==kIOReturnNotOpen)  logline("pipe status = Not open");
			else if(err==kIOUSBPipeStalled) logline("pipe status = Stalled");
			else if(err!=kIOReturnSuccess) { logline("GetPipeStatus returned error 0x%08X", uint(err)); continue; }
			logline("pipe status = 0x%02X", uint(uint8(err)));

			if(err)
			{
				err = (*if_if)->AbortPipe(if_if,pipeRef);
				if(err!=kIOReturnSuccess) { logline("AbortPipe returned error 0x%08X", uint(err)); continue; }

				err = (*if_if)->GetPipeStatus(if_if, pipeRef);
				if(err==kIOReturnNoDevice) logline("pipe status = No Device");
				else if(err==kIOReturnNotOpen)  logline("pipe status = Not open");
				else if(err==kIOUSBPipeStalled)  logline("pipe status = Stalled");
				else if(err!=kIOReturnSuccess) { logline("GetPipeStatus returned error 0x%08X", uint(err)); continue; }
				logline("pipe status = 0x%02X", uint(uint8(err)));
			}


			message = "Hallo Kio !\n\r";
			err = (*if_if)->WritePipe(if_if, pipeRef, (void*)message, strlen(message));	// 0xE00002ed device not responding
			if(err != kIOReturnSuccess)
			{
				logline("Unable to perform bulk write: 0x%08X", uint(err));

				err = (*if_if)->GetPipeStatus(if_if, pipeRef);
				if(err==kIOUSBPipeStalled) logline("pipe status = Stalled");
				else if(err!=kIOReturnSuccess) { logline("GetPipeStatus returned error 0x%08X", uint(err)); continue; }
				logline("pipe status = 0x%02X", uint(uint8(err)));

				continue;
			}
		}
	}

x:	err = (*if_if)->USBInterfaceClose(if_if);
	if(err) logline("USBDeviceClose returned error 0x%08X", uint(err));		// OK
	else logline("Device CLOSED");
	(*if_if)->Release(if_if);
}


void showUSBPrinters()
{
	CFMutableDictionaryRef dict = newMatchingDictForService(service_USBInterface);
	if(!dict) return;

	if(1)	// either pass empty dict (=> show all) or full set for special known device, else nothing is matched
	{
		add(dict,key_VendorID,0x067b);
		add(dict,key_ProductID,0x2305);
		add(dict,key_InterfaceNumber,0x00);
		add(dict,key_ConfigurationValue,0x01);
	}

	io_iterator_t iter = newIteratorForMatchingServices(dict);

	for(io_registry_entry_t interface; (interface=IOIteratorNext(iter)); IOObjectRelease(interface))
	{
		logIn("Printer interface:");
		CFMutableDictionaryRef dict = NULL;
		kern_return_t kr = IORegistryEntryCreateCFProperties(interface,&dict,NULL/*allocator*/,0/*options*/);
		if(kr!=KERN_SUCCESS) break;
		logDict(dict);
		bool ok;
		if(usbSearchShortIntProperty(interface,key_InterfaceClass,no,&ok)==7 && ok)
			printSomething(interface);
	}
	IOObjectRelease(iter);
}




void showSerialDevices()
{
	logIn("showSerialDevices");

	CFMutableDictionaryRef dict = newMatchingDictForService(service_SerialBSDClient);
	if(!dict) return;
//	add(dict, kIOSerialBSDTypeKey, kIOSerialBSDAllTypes);

	io_iterator_t iter = newIteratorForMatchingServices(dict);
	if(!iter) return;

	for(io_registry_entry_t service; (service=IOIteratorNext(iter)); IOObjectRelease(service))
	{
		bool v_ok=no; uint vendorID;
		bool p_ok=no; uint productID;
		cstr portName = NULL;
		cstr devicePath = NULL;
		cstr deviceName = NULL; cstr key_deviceName;
		cstr vendorName = NULL;
		cstr serialNumber = NULL;

		// note: Bei BT-Devices muss man ca. 5 mal runter, bevor man was findet, bei USB ca. 10 mal.

		for(; service; service = parentService(service))
		{
			if(!v_ok) vendorID = usbSearchShortIntProperty(service, key_VendorID, no, &v_ok);
			if(!p_ok) productID = usbSearchShortIntProperty(service, key_ProductID, no, &p_ok);
			if(!portName) portName = usbSearchStringProperty(service, key_TTYDeviceName, no);
			if(!devicePath) devicePath = usbSearchStringProperty(service, key_CalloutDevice, no);
			if(!deviceName) deviceName = getDeviceName(service,&key_deviceName);
			if(!vendorName) vendorName = usbSearchStringProperty(service, key_USBVendorName, no);
			if(!serialNumber) serialNumber = usbSearchStringProperty(service, key_USBSerialNumber, no);

			if(v_ok && p_ok && portName && devicePath && deviceName && vendorName && serialNumber)
				break;
		}

		if(vendorName)	logline("%s = %s",	key_USBVendorName,vendorName);
		if(v_ok)		logline("%s = %04x",key_VendorID,vendorID);
		if(deviceName)	logline("%s = %s",	key_deviceName,deviceName);
		if(p_ok)		logline("%s = %04x",key_ProductID,productID);
		if(serialNumber)logline("%s = %s",	key_USBSerialNumber,serialNumber);
		if(portName)	logline("%s = %s",	key_TTYDeviceName,portName);
		if(devicePath)	logline("%s = %s",	key_CalloutDevice,devicePath);
	}

	IOObjectRelease(iter);
}


cstr num_keys[] =
{
	key_VendorID,
	key_ProductID,
	key_DeviceClass,
	key_DeviceSubClass,
	key_DeviceProtocol,
	key_DeviceMaxPacketSize,
	key_DeviceReleaseNumber,
	key_ManufacturerStrIdx,
	key_ProductStrIdx,
	key_SerialNumberStrIdx,
	key_NumConfigurations,
	key_InterfaceNumber,
	key_AlternateSetting,
	key_NumEndpoints,
	key_InterfaceClass,
	key_InterfaceSubclass,
	key_InterfaceProtocoll,
	key_InterfaceStrIdx,
	key_ConfigurationValue,
	key_1284DeviceID
};

cstr str_keys[] =
{
	key_USBProductName,
	key_USBVendorName,
	key_USBSerialNumber,
	key_CalloutDevice,
	key_TTYDeviceName,
	key_ProductName
};






void showUSBDevices()
{
	logIn("showUSBDevices");

	CFMutableDictionaryRef dict = newMatchingDictForService(service_USBDevice);
	io_iterator_t iter = newIteratorForMatchingServices(dict);

	assert(eq("fooBar",toStr(QCFString("fooBar"))));

	for(io_service_t device; (device = IOIteratorNext(iter)); IOObjectRelease(device))
	{
			logIn("USB device:");
			io_name_t       deviceName;
			kern_return_t kr = IORegistryEntryGetName(device, deviceName);
			logline("device name = %s",kr==KERN_SUCCESS?deviceName:"not found");

			for(;;)
			{
				bool ok;
				for(uint i=0; i<NELEM(num_keys); i++)
				{
					cstr key = num_keys[i];
					uint16 value = usbSearchShortIntProperty(device, key, 0, &ok);
					if(ok)
						logline("%s = 0x%04X",key,value);
					//else logline("%s NOT FOUND",key);
				}
				for(uint i=0; i<NELEM(str_keys); i++)
				{
					cstr key = str_keys[i];
					cstr value = usbSearchStringProperty(device, key, 0);
					if(value)
						logline("%s = %s",key,value);
					//else logline("%s NOT FOUND",key);
				}
				device = parentService(device);
				if(!device) break;
				logline(".");
			}


/*
	IOReturn (*CreateDeviceAsyncEventSource)(void *self, CFRunLoopSourceRef *source);
	CFRunLoopSourceRef (*GetDeviceAsyncEventSource)(void *self);
	IOReturn (*CreateDeviceAsyncPort)(void *self, mach_port_t *port);
	mach_port_t (*GetDeviceAsyncPort)(void *self);
	IOReturn (*USBDeviceOpen)(void *self);
	IOReturn (*USBDeviceClose)(void *self);
	IOReturn (*GetDeviceClass)(void *self, UInt8 *devClass);
	IOReturn (*GetDeviceSubClass)(void *self, UInt8 *devSubClass);
	IOReturn (*GetDeviceProtocol)(void *self, UInt8 *devProtocol);
	IOReturn (*GetDeviceVendor)(void *self, UInt16 *devVendor);
	IOReturn (*GetDeviceProduct)(void *self, UInt16 *devProduct);
	IOReturn (*GetDeviceReleaseNumber)(void *self, UInt16 *devRelNum);
	IOReturn (*GetDeviceAddress)(void *self, USBDeviceAddress *addr);
	IOReturn (*GetDeviceBusPowerAvailable)(void *self, UInt32 *powerAvailable);
	IOReturn (*GetDeviceSpeed)(void *self, UInt8 *devSpeed);
	IOReturn (*GetNumberOfConfigurations)(void *self, UInt8 *numConfig);
	IOReturn (*GetLocationID)(void *self, UInt32 *locationID);
	IOReturn (*GetConfigurationDescriptorPtr)(void *self, UInt8 configIndex, IOUSBConfigurationDescriptorPtr *desc);
	IOReturn (*GetConfiguration)(void *self, UInt8 *configNum);
	IOReturn (*SetConfiguration)(void *self, UInt8 configNum);
	IOReturn (*GetBusFrameNumber)(void *self, UInt64 *frame, AbsoluteTime *atTime);
	IOReturn (*ResetDevice)(void *self);
	IOReturn (*DeviceRequest)(void *self, IOUSBDevRequest *req);
	IOReturn (*DeviceRequestAsync)(void *self, IOUSBDevRequest *req, IOAsyncCallback1 callback, void *refCon);
	IOReturn (*CreateInterfaceIterator)(void *self, IOUSBFindInterfaceRequest *req, io_iterator_t *iter);
	IOReturn (*USBDeviceOpenSeize)(void *self);
	IOReturn (*DeviceRequestTO)(void *self, IOUSBDevRequestTO *req);
	IOReturn (*DeviceRequestAsyncTO)(void *self, IOUSBDevRequestTO *req, IOAsyncCallback1 callback, void *refCon);
	IOReturn (*USBDeviceSuspend)(void *self, Boolean suspend);
	IOReturn (*USBDeviceAbortPipeZero)(void *self);
	IOReturn (*USBGetManufacturerStringIndex)(void *self, UInt8 *msi);
	IOReturn (*USBGetProductStringIndex)(void *self, UInt8 *psi);
	IOReturn (*USBGetSerialNumberStringIndex)(void *self, UInt8 *snsi);
	IOReturn (*USBDeviceReEnumerate)(void *self, UInt32 options);
	IOReturn (*GetBusMicroFrameNumber)(void *self, UInt64 *microFrame, AbsoluteTime *atTime);
	IOReturn (*GetIOUSBLibVersion)(void *self, NumVersion *ioUSBLibVersion, NumVersion *usbFamilyVersion);
	IOReturn (*GetBusFrameNumberWithTime)(void *self, UInt64 *frame, AbsoluteTime *atTime);
*/
//			IOUSBDeviceInterface** usb_if = newUSBDeviceInterfaceForService(device);
//			uint8  byte;
//			uint16 word;
//			uint32 n;

//			(*usb_if)->GetDeviceClass(usb_if,&byte);
//			logline("deviceClass = 0x%02X",uint(byte));

//			(*usb_if)->GetDeviceSubClass(usb_if,&byte);
//			logline("deviceSubClass = 0x%02X",uint(byte));

//			(*usb_if)->GetDeviceProtocol(usb_if,&byte);
//			logline("deviceProtocol = 0x%02X",uint(byte));

//			(*usb_if)->GetDeviceVendor(usb_if,&word);
//			logline("deviceVendor = 0x%04X (%s)",uint(word), vendorDB.get(word,"(unknown)"));

//			(*usb_if)->GetDeviceProduct(usb_if,&word);
//			logline("deviceProduct = 0x%04X",uint(word));

//			(*usb_if)->GetDeviceReleaseNumber(usb_if,&word);
//			logline("deviceReleaseNumber = 0x%04X",uint(word));		// bcdDevice?

//			USBDeviceAddress usb_address;
//			(*usb_if)->GetDeviceAddress(usb_if,&usb_address);
//			logline("deviceAddress = 0x%04X",uint(usb_address));

//			(*usb_if)->GetDeviceBusPowerAvailable(usb_if,&n);
//			logline("deviceBusPowerAvailable = 0x%08X",uint(n));


	}

	IOObjectRelease(iter);
}






UsbDevice::UsbDevice()
{

}

UsbDevice::~UsbDevice()
{

}






