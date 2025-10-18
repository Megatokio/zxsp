// Copyright (c) 2006 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "UsbJoystick.h"
#include "Templates/RCPtr.h"
#include "UsbDevice.h"
#include "zxsp_types.h"
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hid/IOHIDUsageTables.h>

uint				 num_usb_joysticks = 0;
volatile UsbJoystick usb_joysticks[MAX_USB_JOYSTICKS];


static void ShowDeviceProperties(io_registry_entry_t /* io_object_t */ myDevice)
{
	// Print Device Properties
	// ((from the XCode Docs))
	// print the property dictionary
	// for the supplied device.
	// warning: prints really a lot!

	xlogIn("\nShowDeviceProperties");

	kern_return_t		   result;
	CFMutableDictionaryRef properties = nullptr;
	char				   path[512];

	result = IORegistryEntryGetPath(myDevice, kIOServicePlane, path);
	if (result == KERN_SUCCESS) { logline("path = \"%s\"\n", path); }
	else { logline("IORegistryEntryGetPath() error = $%X\n", uint(result)); }

	// Create a CF dictionary representation of the I/O Registry entry's properties
	result = IORegistryEntryCreateCFProperties(myDevice, &properties, kCFAllocatorDefault, kNilOptions);
	if ((result != KERN_SUCCESS) || properties == nullptr)
	{
		logline("IORegistryEntryCreateCFProperties() error = $%X\n", uint(result));
		return;
	}

	// Some common properties of interest:
	//	kIOHIDTransportKey, kIOHIDVendorIDKey, kIOHIDProductIDKey, kIOHIDVersionNumberKey,
	//	kIOHIDManufacturerKey, kIOHIDProductKey, kIOHIDSerialNumberKey, kIOHIDLocationIDKey,
	//	kIOHIDPrimaryUsageKey, kIOHIDPrimaryUsagePageKey, kIOHIDElementKey.
	CFShow(properties);

	CFRelease(properties);
	logNl();
}

bool UsbJoystick::connect(io_object_t dev)
{
	// connect to USB device
	// create device interface
	// get cookies for the buttons
	// verify that's it a joystick
	// if no joystick, close device interface

	xlogIn("UsbJoystick::connect");
	if (loglevel >= 2) ShowDeviceProperties(dev); // <-- very longish!

	dev_if = newHIDDeviceInterfaceForService(dev);

	if (!dev_if) // not a joystick or error
	{
		xlogline("NewDeviceInterface = nullptr");
		return false;
	}

	if (get_cookies())
	{
		bool err = no;
		if (!x_axis)
		{
			xlogline("the device has no x axis.");
			err = yes;
		}
		if (!y_axis)
		{
			xlogline("the device has no y axis.");
			err = yes;
		}
		if (buttons.count() == 0)
		{
			xlogline("the device has no button.");
			err = yes;
		}
		if (!err)
		{
			IOReturn ioerror = (*dev_if)->open(dev_if, 0);
			if (ioerror == kIOReturnSuccess)
			{
				logline("joystick!\n");
				return true; // success
			}
			logline("UsbJoystick::connect: open(dev_if) failed: error = $%X\n", uint(ioerror));
		}
	}
	else xlogline("this is no joystick!\n");

	// error:
	(*dev_if)->Release(dev_if);
	dev_if = nullptr;
	return false;
}

void UsbJoystick::disconnect()
{
	// close device interface to USB device

	xlogIn("UsbJoystick::disconnect");

	if (dev_if)
	{
		(*dev_if)->close(dev_if);
		(*dev_if)->Release(dev_if);
		dev_if = nullptr;
	}
}

bool UsbJoystick::get_cookies()
{
	// Get Cookies for x/y axis and button
	// ((from the XCode docs))
	// retrieves cookies for Joystick::cookies	((struct JoystickCookies))
	// via Joystick::dev_if						((IOHIDDeviceInterface122**))
	// and determines whether it is a joystick

	CFTypeRef		   object;
	uint			   number;
	IOHIDElementCookie cookie;
	uint			   usage;
	uint			   usagePage;
	CFArrayRef		   elements;
	CFDictionaryRef	   element;
	IOReturn		   ioerror;

	x_axis = y_axis = 0;
	buttons.purge();
	if (!dev_if) return no;
	bool is_joystick = no;

	logIn("Joystick:getCookies");

	// O-Ton Apple: Copy all elements, since we're grabbing most of the elements for this device anyway,
	// and thus, it's faster to iterate them ourselves. When grabbing only one or two elements,
	// a matching dictionary should be passed in here instead of NULL.
	ioerror = (*dev_if)->copyMatchingElements(dev_if, nullptr, &elements);
	if (ioerror != kIOReturnSuccess)
	{
		logline("Joystick:getCookies:copyMatchingElements: error = $%X", uint(ioerror));
		return no;
	}
	logline("%i elements", int(CFArrayGetCount(elements)));

	// Loop over elements:
	for (CFIndex i = 0; i < CFArrayGetCount(elements); i++)
	{
		element = CFDictionaryRef(CFArrayGetValueAtIndex(elements, i));
		// logline("\nElement #%i:",int(i));CFShow(element);logNl();

		// Get usage page
		object = CFDictionaryGetValue(element, CFSTR(kIOHIDElementUsagePageKey));
		if (!object || CFGetTypeID(object) != CFNumberGetTypeID()) continue;
		if (!CFNumberGetValue(CFNumberRef(object), kCFNumberIntType, &number)) continue;
		usagePage = number;
		if (usagePage != kHIDPage_GenericDesktop && usagePage != kHIDPage_Button) continue;

		// Get usage
		object = CFDictionaryGetValue(element, CFSTR(kIOHIDElementUsageKey));
		if (!object || CFGetTypeID(object) != CFNumberGetTypeID()) continue;
		if (!CFNumberGetValue(CFNumberRef(object), kCFNumberIntType, &number)) continue;
		usage = number;

		// Get cookie
		object = CFDictionaryGetValue(element, CFSTR(kIOHIDElementCookieKey));
		if (!object || CFGetTypeID(object) != CFNumberGetTypeID()) continue;
		if (!CFNumberGetValue(CFNumberRef(object), kCFNumberIntType, &number)) continue;
		cookie = IOHIDElementCookie(number);

		// Check for x and y axis and application
		log("usagePage = $%04X, usage = $%02X, cookie=$%08X  ", usagePage, usage, uint(cookie));
		if (usagePage == kHIDPage_GenericDesktop /* 1 */)
		{
			switch (usage)
			{
			case kHIDUsage_GD_Pointer: /* 1 */ logline("pointer device"); break;
			case kHIDUsage_GD_Mouse: /* 2 */ logline("mouse"); return no;
			case kHIDUsage_GD_Joystick: /* 4 */
				logline("joystick");
				is_joystick = yes;
				break;
			case kHIDUsage_GD_GamePad: /* 5 */
				logline("game pad");
				is_joystick = yes;
				break;
			case kHIDUsage_GD_Keyboard: /* 6 */ logline("keyboard"); return no;
			case kHIDUsage_GD_Keypad: /* 7 */ logline("keypad"); return no;
			case kHIDUsage_GD_MultiAxisController: /*8*/
				logline("multi-axis controller");
				is_joystick = yes;
				break;
			case kHIDUsage_GD_X: /* $30 */
				logline("x-axis");
				x_axis = cookie;
				break;
			case kHIDUsage_GD_Y: /* $31 */
				logline("y-axis");
				y_axis = cookie;
				break;
			default: logNl(); break;
			}
		}
		// Check for buttons
		else if (usagePage == kHIDPage_Button /* 9 */)
		{
			if (usage >= 1 && usage <= 12) // note: buttons are numbered starting with N = 1
			{
				buttons.grow(usage);
				buttons[usage - 1] = cookie;
				log("button %u\n", usage);
			}
			else logNl();
		}
		else logNl();
	}
	return is_joystick;
}

uint8 UsbJoystick::getState()
{
	// Get Joystick buttons state
	// returns result = %000FUDLR

	xxlogIn("UsbJoystick:getState");

	if (!dev_if) return 0x00;

	HRESULT			 herror;
	IOHIDEventStruct hidEvent;
	int				 x, y, b1 = 0;

	cstr what = "x-axis";
	herror	  = (*dev_if)->getElementValue(dev_if, x_axis, &hidEvent);
	if (herror) goto xx;
	x = hidEvent.value; // seen values: 	min=0  ..  dflt=127..130  ..  max=255

	what   = "y-axis";
	herror = (*dev_if)->getElementValue(dev_if, y_axis, &hidEvent);
	if (herror) goto xx;
	y = hidEvent.value;

	what = "buttons";
	for (uint i = 0; i < buttons.count() && !b1; i++)
	{
		if (buttons[i] == 0) continue;
		herror = (*dev_if)->getElementValue(dev_if, buttons[i], &hidEvent);
		if (herror) goto xx;
		b1 = hidEvent.value;
	}

	xxlogline("x-axis = %i; y-axis = %i; button1 = %i", x, y, b1);

	static constexpr uint8 xval[] = {button_left_mask, 0, 0, button_right_mask};
	static constexpr uint8 yval[] = {button_up_mask, 0, 0, button_down_mask};

	return (b1 ? button_fire1_mask : 0) | xval[(x >> 6) & 3] | yval[(y >> 6) & 3];

// error:
xx:
	logline("UsbJoystick::getState: get %s: error = $%X\n", what, uint(herror));
	disconnect();
	return 0x00;
}

void findUsbJoysticks()
{
	// Search the IO registry for all HID devices
	// first called in Application ctor
	// may be called again to update list of joystick devices

	xxlogNl();
	xlogIn("findUsbJoysticks");

	num_usb_joysticks = 0;

	// Search I/O Registry for HID class devices:
	io_iterator_t iter = 0;
	IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching(kIOHIDDeviceKey), &iter);
	if (!iter)
	{
		logline("findUsbJoysticks:objectIterator = 0");
		return;
	}

	// find joysticks in list of all HID class devices:
	uint devs = 0;
	while (auto dev = IOIteratorNext(iter))
	{
		if (num_usb_joysticks == NELEM(usb_joysticks)) break; // out of slots
		if (nvptr(&usb_joysticks[num_usb_joysticks])->connect(dev)) { num_usb_joysticks++; }
		devs++;
	}

	logline(
		"\nfound %i HID device%s, thereof %i joystick%s\n", devs, devs == 1 ? "" : "s", num_usb_joysticks,
		num_usb_joysticks == 1 ? "" : "s");

	// Release iterator. Don't need to release iterator objects.
	IOObjectRelease(iter);
}
