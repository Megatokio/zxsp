#pragma once
/*	Copyright  (c)	Günter Woigk 2004 - 2019
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice, this permission notice and the
	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY,
	NOT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
	A PARTICULAR PURPOSE, AND IN NO EVENT SHALL THE COPYRIGHT HOLDER
	BE LIABLE FOR ANY DAMAGES ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
*/

// Samples(stereo) per dsp exchange buffer

#define DSP_SAMPLES_PER_BUFFER	735		// 735 = 44.1kHz/60Hz
#define	DSP_SAMPLES_STITCHING	4

/*	notes on buffer size:

	512 debug|final: sehr häufige input-drops, häufige output-drops
	735 debug: sporadische in- und out-drops (~alle 2min) zumeist nach runForSound ≥ 15ms
		final: in-drops: einer in 12 Min. Testzeit (nach runForSound 8ms)
	750 final: in-drops: 2 in 37 Minuten Testzeit (jeweils nach unkrit. Laufzeit von runForSound)
		debug: 1 in-drop (unexpected) und 1 out-drop (nach 8ms Laufzeit) in 20 Minuten
*/


// Realzeit:

typedef double 		Time;
typedef double 		Frequency;
typedef float		Sample;

extern	Frequency	samples_per_second;	// DSP-Konstante & Zeitbasis des Systems: samples/second
extern	Time		system_time;		// Realzeit [seconds]

inline double samples_per_dsp_buffer()		{ return DSP_SAMPLES_PER_BUFFER; }
inline Time   seconds_per_dsp_buffer()      { return DSP_SAMPLES_PER_BUFFER/samples_per_second; }
inline Time   seconds_per_dsp_buffer_max()  { return (DSP_SAMPLES_PER_BUFFER+DSP_SAMPLES_STITCHING-0.01)/samples_per_second; }




/*	Es gibt im System zwei Zeitbasen:

	1.	Realzeit [samples]				Realzeit, gemessen in 'Samples'
	2.	virtuelle Maschinenzeit	[cc]	Spielzeit, gemessen in 'cc'

1. Realzeit [samples]
	Die Sample-Frequenz des DSP dient als Zeitbasis für die Realzeit. Diese Zeit wird kontinuierlich weitergestellt.
	Alle Angaben in 'Sekunden' schnellstmöglich in 'Samples' umrechnen.
	Der Takt des PSG ist an die Realzeit gekoppelt. Damit ändert sich die Tonhöhe bei Änderung der virtuellen Zeitbasis nicht.

	::samples_per_second		Sample-Frequenz des Audio-Systems.
	::samples_per_dsp_buffer	Samples per DSP-Buffer bzw. DSP-Interrupt.
	::system_time				Realzeit zu Beginn eines DSP-Interrupts. Wird nach Ende aller Sounderzeugung um
								seconds_per_dsp_buffer erhöht.


2. Virtuelle Maschinenzeit [cc]
	Die Maschinenzeit entspricht normal der Realzeit.
	Sie hat jedoch eine andere Zeitbasis, jenachdem, wann die Maschine gestartet wurde.
	Bei Throttle oder Overdrive weicht die Geschwindigkeit der virtuelle Maschinenzeit von der Realzeit ab.
	Außerdem setzt die Maschinenzeit manchmal aus, wenn die Maschinen-Semaphore blockiert ist.

	Basis der Maschinenzeit ist der CPU-Takt.
	Dieser ergibt sich aus dem ULA-Takt und dem CPU-Takt-Vorteiler der ULA.
	Außerdem wird der CPU-Taktzähler bei jedem CRT-Strahlrücklauf auf 0 zurückgesetzt.

	machine->ula_cycles_per_sec		Aktueller Eingangstakt der ULA [1/sec]					Angabe in Realzeit!
	machine->cpu_cycles_per_sec		Aktueller Eingangstakt der CPU [1/sec]					Angabe in Realzeit!
	machine->ula->cpu_predivider	1, 2, 4, oder 8: ula_cycles_per_sec / cpu_predivider = cpu_cycles_per_sec

	machine->cpu->cc				CPU-Takt-Zähler innerhalb des aktuellen Frames
	machine->cpu->cc_crt			CPU-Takt-Zähler des CRTC in CPU/ULA innerhalb des aktuellen Frames


3. Synchronisation der virtuellen mit der realen Zeit
	Die Notwendigkeit, die virtuelle mit der realen Zeit zu synchronisieren, tritt an allen Schnittstellen
	der virtuellen mit der realen Welt auf.

3.1. Bildschirmausgabe
	Der Bildschirm muss normalerweise mit jedem FFB aktualisiert werden.
	Die Aktualisierung kann optional auch schon am Ende des In-Screen-Bereiches oder des sichtbaren Bildschirms erfolgen.
	Zur Synchronisierung zwischen der Maschine (DSP-Interrupt) und der realen Welt (Ffb Timer) dient eine Semaphore in der Maschine.
	Der Zeitpunkt der tatsächlichen Bildschirm-Aktualisierung wird sehr ungenau gehandhabt.
	Der reale Bildschirm hat eh eine andere Refreshfrequenz und die Bildschirmaktualisierung ist derart Zeitaufwendig,
	dass häufig Frames übersprungen werden müssen.
	Bei stark niedergetakteter Machine muss der Bildschirm häufiger, evtl. jede Rasterzeile oder gar jeden CRTC-Videoram-Zugriff
	aktualisiert werden.
	Bei stark übertakteter Maschine wird nur jeder n-te Frame mittels ffb_sema für die Anzeige freigegeben.

	machine->screen_sema			Wird von der ULA mit jedem FFB einmal entriegelt (optional etwas früher
									z.B. zu Ende des In-Screen-Bereichs)

3.2. Audio-In/output
	Die Synchronisation von Audio-In und Out dreht sich darum, sicher zu stellen, dass auf die richtigen Samples in den DSP-Puffern und
	überhaupt nur auf Daten _in_ den Puffern zugegriffen wird. Da die CPU grundsätzlich nicht exakt am DSP-Puffer-Ende zu stoppen ist, haben
	die DSP-Puffer hinten jeweils kurze Überlaufbereiche.

	::dsp_buffer_start				system_time * samples_per_second			// samples-based timestamp //
	::dsp_buffer_end				system_time * samples_per_second + samples_per_dsp_buffer	// dito //

3.3. Sonstige
	Sonstige Schnittstellen, etwa Tastatur, Joysticks oder RS232, werden in Realzeit aktualisiert.


4. Probleme
	Werden Timing-Grundlagen verstellt, so müssen alle davon abhängigen Werte und Zähler angepasst werden.
	Manche Probleme lassen sich umgehen, indem Änderungen nur durchgeführt werden, wenn die Maschine gerade nicht ausgeführt wird,
	also außerhalb des DSP-Interrupts. Dazu dient die Maschinen-Semaphore 'sema'. Ist diese bei Ausführung des DSP-Interrupts gesperrt,
	so wird die Maschine einfach einmal ausgelassen und bleibt einen Moment lang stehen. Diese Verzögerung wird nicht mehr aufgeholt.

	machine->sema					Maschinensemaphore

4.1. CPU-Vorteiler
	Wird der CPU-Vorteiler der ULA geändert, ändert sich die virtuelle Zeitbasis der Maschine. Alle davon abhängigen Variablen
	müssen aktualisiert werden.
	Zur Änderung des cpu_predivider wird die Maschinensemaphore gesperrt.
	Alle Chips, die cc als Timingparameter auswerten, müssen darüber informiert werden.

	machine->ula->cpu_predivider
	machine->cpu_cycles_per_sec
	machine->cpu->cc
	machine->cpu->cc_crt
	machine->item[]->CpuClockChanged(new_predivider,new_cc_per_sec)

4.2. CRT Timing-Parameter
	Werden ula->lines_before_screen, ula->lines_in_screen, ula->lines_after_screen oder ula->cc_per_row geändert, so ändert sich
	die Framerate frames_per_sec.
	Zur Änderung der CRT-Timing-Parameter wird die Maschinensemaphore gesperrt.
	Es kann passieren, dass bei Eintritt in machine::runForSound() der FFB bereits überschritten ist: cc > cc_per_frame.

4.3. Throttle/Overdrive (ULA-Takt)
	ALT!
	NEU: cc ist die Zeitbasis, der Ula-Eingangstakt wird auf Anfrage von der Ula berechnet!
	!!Wird der Eingangstakt der ULA geändert, ändert sich die virtuelle Zeitbasis der Maschine. Alle davon abhängigen Variablen
	müssen aktualisiert werden.
	!!Zur Änderung des Ula-Taktes wird die Maschinensemaphore gesperrt.
	!!Alle Chips, die cc als Timingparameter auswerten, müssen darüber informiert werden.

	!!machine->item[]->CpuPredividerChanged(old_cpu_predivider,new_cpu_predivider)

4.4. DSP-Buffer-Ende
	Audio-Erzeuger müssen ihren Sound bis zum Ende des DSP-Buffers auffüllen. Außerdem müssen sie ihre Zeitbasis für den nächsten
	DSP-Interrupt zurückstellen.

	machine->item[]->Update( Time seconds, int32 cc )

4.5. input / output
	Item::input() und Item::output() wird die Realzeit und die Spielzeit mitgegeben. Die Realzeit ist der Offset innerhalb des
	DSP-Puffers, die Spielzeit ist der Offset zu letzten FFB. Benötigen die Chips fortlaufende Zeit, müssen sie die system_sampletime
	bzw. cc_ffb addieren.

	machine->item[]->output (time,cc,addr,byte)
	machine->item[]->input  (time,cc,addr)
	machine->item[]->reset  (time,cc)
	machine->item[]->PowerOn(time,cc)

4.6. Store Samples
	Die Utility-Routine zum Speichern von Samples erwartet Zeitangaben basierend auf Realzeit-Offset innerhalb des DSP-Puffers.

	machine::sampleOutput:(Sample)sample fromTime:(Time)aa toTime:(Time)ee
*/














