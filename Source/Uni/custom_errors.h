


#ifndef EMAC
#define NO_EMAC
enum {
#define EMAC(A,B)	A
#endif


EMAC( disksizeexceeds128GB,		"disc size exceeds 2^28 sectors (128GiB)" ),// IdeDevice
EMAC( disksizelessthan720K,		"not a disc image (less than 720K)" ),		// IdeDevice
EMAC( wrongmagic,				"wrong magic" ),							// IdeDevice
EMAC( filecontainslowbytesonly,	"the file contains low bytes only."),		// IdeDevice

EMAC( zlibversionerror,			"zlib: unsupported version"),			// RzxBlock
EMAC( zlibbuffererror,			"zlib: decompressed data too long"),	// RzxBlock
EMAC( zlibdataerror,			"zlib: data corrupted"),				// RzxBlock
EMAC( zliboutofmemory,			"zlib: out of memory"),					// RzxBlock


#ifdef NO_EMAC
#undef NO_EMAC
#undef EMAC
};
#endif
