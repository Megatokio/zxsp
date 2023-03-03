#!/usr/local/bin/vipsi


/*	Copyright  (c)	Günter Woigk 2009 - 2017
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


	this script must be started in folder
		zxsp-osx/build-qt580-debug
	(or similar)

	increment version below!
*/


// zxsp version:
//
var version = "0.8.30 beta"
log "TODO: should read version from settings.h",nl
//exit 1


var t0 = now

var projectdir = "../"
var sdccdir = projectdir#"zasm/sdcc/"

if !exists dir "zxsp.app/" new dir "zxsp.app/" then
if !exists dir "zxsp.app/Contents/" new dir "zxsp.app/Contents/" then
if !exists dir "zxsp.app/Contents/Resources/" new dir "zxsp.app/Contents/Resources/" then
if !exists dir "zxsp.app/Contents/sdcc/" new dir "zxsp.app/Contents/sdcc/" then


// Resource files:
// these will be copied from $PROJECT/Resources/ to zxsp.app/Contents/Resources/
//
var resources = { "Roms/*.rom",
				  "Files/*.icns",
				  "Snapshots/*.z80",	"EmptyFiles/*",
				  "Keyboards/*.jpg",	"Keyboards/*.png",
				  "Images/*.jpg",		"Images/*.png",
				  "Backgrounds/*.jpg", 	"Backgrounds/*.gif",
				  "Overlays/*.png",
				  "Audio/*.raw"
				 }


proc StringForText(t)
{
		return "\t<string>"#t#"</string>\n"
}

proc ArrayForList(l)
{
	if istext l
		return "\t<array>\n" # StringForText(l) # "\t</array>\n"
	else
		var r="", i=0
		do
			while ++i <= count l
			r #= StringForText(l[i])
		loop
		return "\t<array>\n" # r # "\t</array>\n"
	then
}

proc Def(key,value)
{
	return "\t<key>"#key#"</key>\n" # ( islist value ? ArrayForList(value) : StringForText(value) )
}

proc BundleForFile(extensions,icon,mimetypes,mimetext,ostypes,role)
{
	var i=count extensions,l,u
	do l=extensions[i] u=upperstr(l) if l!=u extensions##={u} then while --i>0 loop
	return
		"\t<dict>\n"
			# Def("CFBundleTypeExtensions", extensions )
			# Def("CFBundleTypeIconFile",   icon )
			# Def("CFBundleTypeMIMETypes",  {mimetypes} )
			# Def("CFBundleTypeName",		mimetext )
			# Def("CFBundleTypeOSTypes",	{ostypes} )
			# Def("CFBundleTypeRole",		role )
			# "\t<key>LSTypeIsPackage</key>\n\t<false/>\n"
		# "\t</dict>\n"
}


// create PkgInfo file:
//
put "create PkgInfo",nl
file "zxsp.app/Contents/PkgInfo" = "APPLzxsp"


// create Info.plist file:
//
put "create Info.plist",nl
file "zxsp.app/Contents/Info.plist" =
«<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
»	#
"<dict>
"	# Def("CFBundleDevelopmentRegion","English" ) #
"	<key>CFBundleDocumentTypes</key>
	<array>
"	# BundleForFile( {"z80"},			"Files/z80.icns",		"application/zxsp-z80",		"ZX Spectrum Snapshot",		"Z80",  "Viewer" )
	# BundleForFile( {"sna"},			"Files/sna.icns",		"application/zxsp-sna",		"ZX Spectrum Snapshot",		"Snap", "Viewer" )
	# BundleForFile( {"ace"},			"Files/ace.icns",		"application/zxsp-ace",		"Jupiter ACE Snapshot",		"Ace",  "Viewer" )
	# BundleForFile( {"p","81","p81"},	"Files/zx81.icns",		"application/zx81-tape",	"ZX81 Tape",				"Tap1", "Viewer" )
	# BundleForFile( {"o","80"},		"Files/zx80.icns",		"application/zx80-tape",	"ZX80 Tape",				"Tap0", "Viewer" )
	# BundleForFile( {"tap","tape"},	"Files/tap.icns",		"application/zxsp-tape",	"ZX Spectrum Tape",			"Tape", "Editor" )
	# BundleForFile( {"tzx"},			"Files/tzx.icns",		"application/zxsp-tzx",		"ZX Spectrum Tape",			"TZX",	"Viewer" )
	# BundleForFile( {"dsk"},			"Files/dsk.icns",		"application/zxsp-disk",	"ZX Spectrum Disk",			"DISK", "Editor" )
	# BundleForFile( {"rom"},			"Files/rom.icns",		"application/zxsp-rom",		"ZX Spectrum Rom",			"ROM",	"Viewer" )
	# BundleForFile( {"dck"},			"Files/dck.icns",		"application/zxsp-dck",		"Timex Command Cartridge",	"DOCK", "Viewer" )
	# BundleForFile( {"scr"},			"Files/scr.icns",		"application/zxsp-scr",		"ZX Spectrum Screenshot",	"SCR",	"Viewer" )
	# BundleForFile( {"ass","asm","src","s"}, "Files/ass.icns",	"text/x-asm",				"Assembler Source",			"TEXT", "Viewer" )
	# BundleForFile( {"rzx"},			"Files/rzx.icns",		"application/zxsp-rzx",		"ZX Spectrum Recording",	"RZX!",	"Editor" )
	# BundleForFile( {"gif"},			"Files/blank.icns",		"image/gif",				"GIF Image",				"GIF",	"Viewer" )
	# BundleForFile( {"c"},				"Files/blank.icns",		"text/x-c",					"C Source",					"TEXT",	"Viewer" )
	# BundleForFile( {"img"},			"Files/img.icns",		"application/zxsp-img",		"Hard Disk Image",			"IMG",	"Editor" )
	# BundleForFile( {"dmg"},			"Files/dmg.icns",		"application/zxsp-dmg",		"Hard Disk Image",			"DMG",	"Editor" )
	# BundleForFile( {"iso"},			"Files/iso.icns",		"application/zxsp-iso",		"CDRom Disk Image",			"ISO",	"Viewer" )
	# BundleForFile( {"hdf"},			"Files/hdf.icns",		"application/zxsp-hdf",		"Hard Disk Image",			"HDF",	"Editor" )
	# BundleForFile( {"bin"},			"Files/blank.icns",		"application/zxsp-bin",		"ZX Spectrum Rom",			"BIN",	"Viewer" )
	# BundleForFile( {"aif","aiff","aifc"}, "Files/tap.icns",	"audio/aiff",				"Apple audio file",			"AIFF",	"Viewer" )
	# BundleForFile( {"wav"},			"Files/tap.icns",		"audio/wav",				"Microsoft audio file",		"WAV",	"Viewer" )
	# BundleForFile( {"mp2"},			"Files/tap.icns",		"audio/mpeg",				"MPEG-2 audio file",		"MP2",	"Viewer" )
	# BundleForFile( {"mp3"},			"Files/tap.icns",		"audio/mpeg3",				"MPEG-3 audio file",		"MP3",	"Viewer" )
	# BundleForFile( {"m4a"},			"Files/tap.icns",		"audio/m4a",				"MPEG-4 audio file",		"M4A",	"Viewer" )
	#
"	</array>
"	# Def("CFBundleExecutable","zxsp" )
	# Def("CFBundleIconFile","Files/app.icns")
	# Def("CFBundleIdentifier","kio.zxsp")
	# Def("CFBundleInfoDictionaryVersion","6.0")
	# Def("CFBundlePackageType","APPL")
	# Def("CFBundleSignature","zxsp")
	# Def("CFBundleVersion",version)
	# Def("NOTE","created by makemacstuff.vs")
//	<key>NSMainNibFile</key>
//	<string>MainMenu</string>
//	<key>NSPrincipalClass</key>
//	<string>MainApp</string>
#"</dict>
</plist>\n"


// create icns files:
{
	var qdir = projectdir # "Resources/Files/"
	var d1 = dir(qdir#"*.iconset")
	var d2 = dir(qdir#"*.icns")

	var i = 0
	do
		while ++i <= count d1
		var iconset = replace(d1[i].fname,"/","")	if(iconset=="template file.iconset") next then
		var icns    = iconset[to count(iconset)-8]#".icns"

		var j = 0
		do
			while ++j <= count d2
			until icns == d2[j].fname
		loop

		if(j > count d2) sys "/usr/bin/iconutil", "-c", "icns", qdir#iconset then
	loop
}


// copy resource files:
//
proc CopyFiles( qdir, zdir, mask )
{
	put "copy DIR ",qdir,mask,nl
	if !exists dir zdir new dir zdir then

	if find(mask,"/")
		split mask,"/"
		do
			while count(mask) > 1
			qdir #= mask[1] # "/"
			zdir #= mask[1] # "/"
			del mask[1];
			if !exists dir zdir new dir zdir then
		loop
		mask = mask[1]		// z.B.: "*.gif"
	then

	var d = dir(qdir#mask)
	var i = 0
	do
		while ++i <= count d
		if d[i].flags[1]=="d" next then
		var fname = d[i].fname
		if fname[1]=="." next then
//		put "  ",qdir,fname," ... "
		file(zdir#fname) = file(qdir#fname)
//		put "ok",nl
	loop

	var d = dir(qdir)
	var i = 0
	do
		while ++i <= count d
		if d[i].flags[1]!="d" next then
		var fname = d[i].fname
		CopyFiles(qdir#fname,zdir#fname,mask)
	loop
}


// _________________________________________________
// copy resources from ../Resources/ into app bundle
//
var i=0
do
	while ++i <= count resources
	CopyFiles(projectdir # "Resources/", "zxsp.app/Contents/Resources/", resources[i])
loop


// _________________________________________________________________________
// copy sdcc and sdcpp from zasm/sdcc/bin/ into app bundle
// these executables are copied into the same dir as the main app executable
//
CopyFiles(sdccdir # "bin/", "zxsp.app/Contents/MacOS/", "sdc*")
sys "chmod", "ug+x", "zxsp.app/Contents/MacOS/sdcc", "zxsp.app/Contents/MacOS/sdcpp"

// copy sdcc header files from zasm/sdcc/include/ into app bundle
//
CopyFiles(sdccdir # "include/", "zxsp.app/Contents/sdcc/include/", "*.h")

// copy sdcc library files from zasm/sdcc/lib/ into app bundle
//
CopyFiles(sdccdir # "lib/", "zxsp.app/Contents/sdcc/lib/", "*.c")	// TODO: copy symlinks as symlinks
CopyFiles(sdccdir # "lib/", "zxsp.app/Contents/sdcc/lib/", "*.s")	// TODO: copy symlinks as symlinks




















