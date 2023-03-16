#!/usr/local/bin/vipsi

// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

put "makezxinfo",nl

var sourcefile = "ZxInfo.csv"
var constsfile = "info.h"
var zxinfofile = "ZxInfo"			// basename only

// Daten einlesen:

	var data = file(sourcefile)		// text einlesen
	convert data from utf8;
	split data						// array of lines splitten
	split data,","					// array of array of fields

// Zeile 1 einlesen:

	var prefixe = data[1];			// 1. Zeile = Variablen-Prefixe (Namen)
	data = data[2 to]
	prefixe = prefixe[4 to]			// Spalte 1-3 sind Zeilenbeschriftung etc.

// Spalten 1 und 2 einlesen:

	var types = {}
	var names = {}
	var maxnamelen=0

	var i=0
	do
		while ++i <= count data
		types[i] = data[i,1]		// 1. Spalte = Datentyp
		names[i] = data[i,2]		// 2. Spalte = Variablenname
		data[i] = data[i,4 to]		// 3. Spalte = Kommentar
		maxnamelen = max(maxnamelen,count(names[i]))
	loop							// data[i,*] = Werte


// Datei mit const Definitionen erzeugen:

	put "  create info.h",nl
	var f
	openout#f,constsfile

	put#f,"//auto generated "#datestr(now) ,nl,nl
	put#f,"#ifndef ZXDEFINITIONS_H",nl,
	      "#define ZXDEFINITIONS_H",nl,
	      "#include \"Uni/zxsp_types.h\"",nl,nl

	var x=0
	do
		while ++x <= count prefixe
		var y=0
		do
			while ++y <= count data
			if types[y]=="" put+f,nl next then
			if types[y]=="cstr" data[y,x] = convert(data[y,x] to quoted) then
			put#f, "const ", (types[y]#"      ")[to 9], prefixe[x] # "_" # names[y], spacestr(maxnamelen-count(names[y])), "= ", data[y,x], ";", nl
		loop
		put#f,nl
	loop
	put#f,"#endif",nl,nl
	close#f


// Header-Datei für Struct ZxInfo erzeugen:

	put "  create ZxInfo.h",nl
	openout#f,zxinfofile#".h"
	put#f,«\
//auto generated »#datestr(now)#«
#pragma once
#include "isa_id.h"
#include "kio/kio.h"

enum Language { british, american, french, spanish, portuguese };»,nl,nl

	put#f,"enum Model{",nl
	put#f,"unknown_model=-1,",nl
	x=0
	do
		while ++x <= count prefixe
		put#f,prefixe[x], ",", nl
	loop
	put#f,"num_models",nl
	put#f,"};",nl,nl


	put#f,"struct ZxInfo{",nl,"Model    model;",nl
	y=0
	do
		while ++y <= count data
		if types[y]=="" put+f,nl next then
		put#f, (types[y]#"      ")[to 9], names[y], ";", nl
	loop
	put#f,«\
bool	hasWaitmap () const			{ return waitmap!=0; }
uint32	cpuClockPredivider() const	{ return ula_cycles_per_second / cpu_cycles_per_second; }
bool	isA(isa_id i) const			{ isa_id j=id; do { if(i==j) return yes; } while((j=isa_pid[j])); return no; }
bool	canAttachDivIDE() const		{ return has_zxsp_bus && !isA(isa_MachineTc2068); }
bool	canAttachZxIf2() const		{ return has_zxsp_bus && !isA(isa_MachineTc2068) && !isa_MachineZxPlus2a; }
bool	canAttachSpectraVideo() const { return model<=zxplus2a_span && model!=inves; }
	»
	put#f,"};",nl,nl

	put#f,"extern ZxInfo zx_info[num_models];",nl,nl
	close#f


// cpp-Datei für Struct ZxInfo erzeugen:

	put "  create ZxInfo.cpp",nl
	openout#f,zxinfofile#".cpp"
	put#f,"//auto generated "#datestr(now) ,nl,nl
	put#f,«#include "kio/kio.h"»,nl
	put#f,«#include "»#zxinfofile#«.h"»,nl,nl

	put#f,"ZxInfo zx_info[num_models] = {",nl,nl

	var x=0,s1=""
	do
		while ++x <= count prefixe
		var y=0

		put#f,s1,"{", prefixe[x],nl
		s1=","

		do
			while ++y <= count data
			if types[y]=="" next then
			put#f, ",", data[y,x], nl
		loop
		put#f,"}",nl
	loop

	put#f,"};",nl,nl
	close#f


put "done.",nl












