/*	Copyright  (c)	Günter Woigk 2006 - 2018
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

#include <IOKit/IOCFPlugIn.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hid/IOHIDUsageTables.h>
#include "UsbJoystick.h"
#include "UsbDevice.h"



/* ----	Print Device Properties -------------
		((from the XCode Docs))
		print the property dictionary
		for the supplied device.
		warning: prints really a lot!
*/
static
void ShowDeviceProperties ( io_registry_entry_t /* io_object_t */ myDevice )
{
	xlogIn("\nShowDeviceProperties");

	kern_return_t			result;
	CFMutableDictionaryRef	properties = NULL;
	char					path[512];

	result = IORegistryEntryGetPath ( myDevice, kIOServicePlane, path );
	if( result == KERN_SUCCESS ) { logline( "path = \"%s\"\n", path ); }
	else						 { logline( "IORegistryEntryGetPath() error = $%X\n", uint(result) ); }

// Create a CF dictionary representation of the I/O Registry entry's properties
	result = IORegistryEntryCreateCFProperties( myDevice, &properties, kCFAllocatorDefault, kNilOptions );
	if ( (result != KERN_SUCCESS) || properties==NULL )
	{ logline( "IORegistryEntryCreateCFProperties() error = $%X\n", uint(result) ); return; }

// Some common properties of interest:
//	kIOHIDTransportKey, kIOHIDVendorIDKey, kIOHIDProductIDKey, kIOHIDVersionNumberKey,
//	kIOHIDManufacturerKey, kIOHIDProductKey, kIOHIDSerialNumberKey, kIOHIDLocationIDKey,
//	kIOHIDPrimaryUsageKey, kIOHIDPrimaryUsagePageKey, kIOHIDElementKey.
	CFShow( properties );

	CFRelease( properties );
	logNl();
}


///* ----	Get a device interface for a io_object -------------
//		((from the XCode Docs))
//		struct IOHIDDeviceInterface122 is the extended
//		IOHIDDeviceInterface since OSX 10.3
//*/
//static
//IOHIDDeviceInterface122** NewDeviceInterface ( io_object_t device )
//{
//	xlogIn("NewDeviceInterface");

////	io_name_t                 className;
////	ioerror = IOObjectGetClass( myDevice, className );
////	if( ioerror != kIOReturnSuccess )
////	{ logline( "NewDeviceInterface:IOObjectGetClass: error = $%X", uint(ioerror) ); return NULL; }
////	xlogline( "Device class = %s", className );	// e.g. "IOUSBHIDDriver"

//	IOCFPlugInInterface**	  plugin_if = NULL;
//	IOHIDDeviceInterface122** dev_if    = NULL;
//	SInt32	score = 0;
//	uint32	err   = ok;

//// Create a plugin interface
//	err = IOCreatePlugInInterfaceForService ( device, kIOHIDDeviceUserClientTypeID, kIOCFPlugInInterfaceID, &plugin_if, &score );
//	if( err != kIOReturnSuccess ) { logline( "NewDeviceInterface:IOCreatePlugInInterface: error = $%lX\n", err ); return NULL; }
//	assert( plugin_if && *plugin_if );

//// Create the device interface
//	err = (*plugin_if)->QueryInterface ( plugin_if, CFUUIDGetUUIDBytes( kIOHIDDeviceInterfaceID ), (void**)&dev_if );

//// Purge the plugin interface
//	(*plugin_if)->Release(plugin_if);

//	if( err != S_OK ) { logline( "NewDeviceInterface:QueryInterface: error = $%lX\n", err ); return NULL; }
//	return dev_if;
//}



// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%



UsbJoystick::UsbJoystick()
:	Joystick(isa_UsbJoystick)
{
	xlogIn("new UsbJoystick");
	dev_if = 0;
}

UsbJoystick::~UsbJoystick()
{
	xlogIn("~UsbJoystick");
	disconnect();
}


/* ----	connect to USB device ----
		create device interface
		get cookies for the buttons
		verify that's it a joystick
		if no joystick, close device interface
*/
void UsbJoystick::connect ( io_object_t dev )
{
	xlogIn("UsbJoystick:Connect");
	if(XXLOG) ShowDeviceProperties(dev); 		// <-- very longish!

	PLocker z(lock);
	dev_if = newHIDDeviceInterfaceForService( dev );

	if( !dev_if )
	{
		xlogline( "NewDeviceInterface = NULL" );	// ((not a joystick or error))
		return;
	};

	if( getCookies() )
	{
		bool err = no;
		if( !x_axis )			 { xlogline( "the device has no x axis."); err=yes; }
		if( !y_axis )			 { xlogline( "the device has no y axis."); err=yes; }
		if( buttons.count()==0 ) { xlogline( "the device has no button."); err=yes; }
		if( !err )
		{
			IOReturn ioerror = (*dev_if)->open(dev_if,0);
			if( ioerror==kIOReturnSuccess ) { logline("joystick!\n"); return; }	// ok
			logline( "UsbJoystick:Connect:open(dev_if) failed: error = $%X\n", uint(ioerror) );
		}
	}
	else 	xlogline( "this is no joystick!\n");

// error:
	(*dev_if)->Release(dev_if);
	dev_if = 0;
}


/* ----	close device interface to USB device ----
*/
void UsbJoystick::disconnect()
{
	xlogIn("UsbJoystick:Disconnect");

	PLocker z(lock);
	if( dev_if )
	{
		(*dev_if)->close(dev_if);
		(*dev_if)->Release(dev_if);
		dev_if = 0;
	}
}


/* ----	Get Joystick buttons state ------------
		returns result = %000FUDLR
*/
uint8 UsbJoystick::getState(bool mark_active) volatile
{
	xxlogIn( "UsbJoystick:getState" );

	// already polled in this dsp callback?
	if(last_time == system_time) return state;

	// mark joystick active?
	if(mark_active) last_time = system_time;

	// get new state:
	PLocker z(lock);
	state = const_cast<UsbJoystick*>(this)->get_state();
	return state;
}

uint8 UsbJoystick::get_state() const
{
	if( !dev_if ) return 0;

	HRESULT/*SInt32*/	herror;
	IOHIDEventStruct	hidEvent;
	int x,y,b1=0;

	cstr what = "x-axis";
	herror = (*dev_if)->getElementValue( dev_if, x_axis, &hidEvent );
	if (herror) goto xx;
	x = hidEvent.value;		// seen values: 	min=0  ..  dflt=127..130  ..  max=255

	what = "y-axis";
	herror = (*dev_if)->getElementValue( dev_if, y_axis, &hidEvent );
	if (herror) goto xx;
	y = hidEvent.value;

	what = "buttons";
	for(uint i=0; i<buttons.count() && !b1; i++)
	{
		if(buttons[i]==0) continue;
		herror = (*dev_if)->getElementValue( dev_if, buttons[i], &hidEvent );
		if (herror) goto xx;
		b1 = hidEvent.value;
	}

	xxlogline( "x-axis = %i; y-axis = %i; button1 = %i", int(x), int(y), int(b1) );

	static uint8 xval[] = { button_left_mask, 0, 0, button_right_mask };
	static uint8 yval[] = { button_up_mask,   0, 0, button_down_mask  };

	return	( b1 ? button1_mask : 0 ) | xval[x>>6] | yval[y>>6];

// error:
xx:	logline( "UsbJoystick:getState:getElementValue:%s: error = $%X\n", what, uint(herror) );
	(*dev_if)->close(dev_if);
	(*dev_if)->Release(dev_if);
	dev_if = 0;
	return 0;
}


/* ----	Get Cookies for x/y axis and button ----
		((from the XCode docs))
		retrieves cookies for Joystick::cookies	((struct JoystickCookies))
		via Joystick::dev_if			((IOHIDDeviceInterface122**))
		and determines whether it is a joystick
*/
bool UsbJoystick::getCookies()
{
	CFTypeRef			object;
	int32				number;
	IOHIDElementCookie	cookie;
	int32				usage;
	int32				usagePage;
	CFArrayRef			elements;
	CFDictionaryRef		element;
	IOReturn			ioerror;

	x_axis = y_axis = 0;
	buttons.purge();
	if( !dev_if ) return no;
	bool is_joystick = no;

	logIn( "Joystick:getCookies" );

// O-Ton Apple: Copy all elements, since we're grabbing most of the elements for this device anyway,
// and thus, it's faster to iterate them ourselves. When grabbing only one or two elements,
// a matching dictionary should be passed in here instead of NULL.
	ioerror = (*dev_if)->copyMatchingElements(dev_if, NULL, &elements);
	if( ioerror != kIOReturnSuccess )
	{ logline( "Joystick:getCookies:copyMatchingElements: error = $%X", uint(ioerror) ); return no; }
	logline( "%i elements", int(CFArrayGetCount(elements)) );

// Loop over elements:
	for ( CFIndex i=0; i<CFArrayGetCount(elements); i++ )
	{
		element = (CFDictionaryRef)CFArrayGetValueAtIndex(elements, i);
		//logline("\nElement #%i:",int(i));CFShow(element);logNl();

	// Get usage page
		object = CFDictionaryGetValue( element, CFSTR(kIOHIDElementUsagePageKey) );
		if( object==0 || CFGetTypeID(object) != CFNumberGetTypeID() ) continue;
		if( !CFNumberGetValue((CFNumberRef) object, kCFNumberLongType, &number) ) continue;
		usagePage = number;
		if( usagePage!=kHIDPage_GenericDesktop && usagePage!=kHIDPage_Button ) continue;

	// Get usage
		object = CFDictionaryGetValue( element, CFSTR(kIOHIDElementUsageKey) );
		if( object==0 || CFGetTypeID(object) != CFNumberGetTypeID() ) continue;
		if( !CFNumberGetValue((CFNumberRef) object, kCFNumberLongType, &number) ) continue;
		usage = number;

	// Get cookie
		object = CFDictionaryGetValue( element, CFSTR(kIOHIDElementCookieKey) );
		if( object==0 || CFGetTypeID(object) != CFNumberGetTypeID() ) continue;
		if( !CFNumberGetValue((CFNumberRef) object, kCFNumberLongType, &number) ) continue;
		cookie = (IOHIDElementCookie) number;

	// Check for x and y axis and application
		log( "usagePage = $%04X, usage = $%02X, cookie=$%08X  ", uint(usagePage), uint(usage), uint(cookie) );
		if( usagePage == kHIDPage_GenericDesktop /* 1 */ )
		{
			switch( usage )
			{
			case kHIDUsage_GD_Pointer:	/* 1 */ 		logline( "pointer device" );		break;
			case kHIDUsage_GD_Mouse:	/* 2 */ 		logline( "mouse" );					return no;
			case kHIDUsage_GD_Joystick:	/* 4 */ 		logline( "joystick" ); is_joystick=yes; break;
			case kHIDUsage_GD_GamePad:	/* 5 */ 		logline( "game pad" ); is_joystick=yes; break;
			case kHIDUsage_GD_Keyboard:	/* 6 */ 		logline( "keyboard" );				return no;
			case kHIDUsage_GD_Keypad:	/* 7 */ 		logline( "keypad" );				return no;
			case kHIDUsage_GD_MultiAxisController:/*8*/	logline( "multi-axis controller" ); is_joystick = yes; break;
			case kHIDUsage_GD_X:		/* $30 */ 		logline( "x-axis" ); x_axis = cookie; break;
			case kHIDUsage_GD_Y:		/* $31 */		logline( "y-axis" ); y_axis = cookie; break;
			default:									logNl(); break;
			}
		}
	// Check for buttons
		else if( usagePage == kHIDPage_Button /* 9 */ )
		{
			if(usage>=1&&usage<=12)		// note: buttons are numbered starting with N = 1
			{
				buttons.grow(usage);
				buttons[usage-1] = cookie;
				log("button %i\n", int(usage));
			}
			else logNl();
		}
		else logNl();
	}
	return is_joystick;
}


// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//
//				Find USB joysticks:
//

/* ----	Search the IO registry for all HID devices ----
*/
void findUsbJoysticks()
{
	xxlogNl();
	xlogIn("findUsbJoysticks");

// create and disconnect all usb joysticks
	for( int i=0; i<num_usb; i++ )
	{
		Joystick*& joy = joysticks[i];
		if(joy) reinterpret_cast<UsbJoystick*>(joy)->disconnect();	// disconnect existing
		else  joy = new UsbJoystick();
	}

// data we need:
	const mach_port_t port  = kIOMasterPortDefault;	// or from IOMasterPort()
	IOReturn		  err	= kIOReturnSuccess;
	io_object_t		  dev   = 0;
	io_iterator_t     iter	= 0;

// What do we search?
// Create a matching dictionary to search the I/O Registry by class name for all HID class devices
	CFMutableDictionaryRef myMatchDictionary = IOServiceMatching( kIOHIDDeviceKey );
	//CFShow(myMatchDictionary);

// Search I/O Registry for matching devices
	err = IOServiceGetMatchingServices( port, myMatchDictionary, &iter );
	myMatchDictionary = NULL;		// the dictionary was consumed by IOServiceGetMatchingServices()
	if( !iter ) { logline( "findUsbJoysticks:objectIterator = 0" ); return; }
	assert( err == kIOReturnSuccess );

// Loop over all devices found:
	int joys=0,devs=0;
	while(( dev=IOIteratorNext(iter) ))
	{
		if(joys==max_joy) break;			// out of slots
		usbJoystick(joys)->connect(dev);
		if( usbJoystick(joys)->isConnected() ) { joys++; } devs++;
	}
	logline("\nfound %i HID device%s, thereof %i joystick%s\n",devs,devs==1?"":"s",joys,joys==1?"":"s");

// Release iterator. Don't need to release iterator objects.
	IOObjectRelease( iter );
}



































