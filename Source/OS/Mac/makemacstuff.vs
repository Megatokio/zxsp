#!/usr/local/bin/vipsi

/*	Copyright (c) 2009 - 2023 kio@little-bat.de
	BSD-2-Clause license
	https://opensource.org/licenses/BSD-2-Clause

	this script must be started in folder
		zxsp/build-qt59-debug
	(or similar)
*/


var t0 = now
var projectdir = "../"
var sdccdir = projectdir#"zasm/sdcc/"
var versionfile = projectdir # "Source/version.h"


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



proc ReadVersion()
{
	var text = file versionfile
	split text
	var beta = 1
	var h="",m="",l="",str=""
	var i=0
	do
	while ++i <= count(text)
		var s = text[i]
		if s[to 7] != "#define" next then
		replace s,"\t"," "
		split s," "
		var k = s[2]
		var v = s[count s]
		if k == "APPL_VERSION_H"	  h = v
		elif k == "APPL_VERSION_M"    m = v
		elif k == "APPL_VERSION_L"    l = v
		elif k == "APPL_VERSION_STR"  str = eval(v)
		elif k == "APPL_VERSION_BETA" beta = s=="true" || s=="yes" || s=="1"
		then
	loop

	if h# "." #m# "." #l != str
		log "version.h: version mismatch: " #h# "." #m# "." #l# " versus " #str# nl
		end 1
	then

	return str # (beta ? " beta" : "")
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


// zxsp version:
//
var version = ReadVersion()


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




















