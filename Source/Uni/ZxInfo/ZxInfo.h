//auto generated 2017-11-08 16:08:53
#ifndef ZXINFO_H
#define ZXINFO_H
#include "Language.h"
#include "IsaObject.h"

enum Model{
unknown_model=-1,
zxsp_i1,
zxsp_i2,
zxsp_i3,
zxplus,
zxplus_span,
inves,
zx128,
zx128_span,
zxplus2,
zxplus2_span,
zxplus2_frz,
zxplus3,
zxplus3_span,
zxplus2a,
zxplus2a_span,
tc2048,
tc2068,
ts2068,
u2086,
tk90x,
tk95,
pentagon128,
scorpion,
samcoupe,
zx80,
zx81,
ts1000,
ts1500,
tk85,
jupiter,
num_models
};

struct ZxInfo{
Model    model;
cstr     name;
bool     is_supported;
bool     has_zxsp_bus;
bool     has_zx80_bus;
isa_id   id;
cstr     nickname;
cstr     rom_filename;
cstr     kbd_filename;
cstr     image_filename;
Language language;
uint32   ula_cycles_per_second;
uint     cpu_clock_predivider;
uint32   cpu_cycles_per_second;
uint     cpu_cycles_per_line;
uint     lines_before_screen;
uint     lines_in_screen;
uint     lines_after_screen;
uint32   cpu_cycles_per_frame;
double   frames_per_second;
bool     has_50_60hz_switch;
uint     page_size;
uint     rom_pages;
uint     ram_pages;
uint32   rom_size;
uint32   ram_size;
uint32   contended_rampages;
uint8    waitmap;
uint32   videoram_start;
uint32   videoram_start_2;
uint32   videoram_size;
uint     characterram_start;
uint     characterram_size;
float    earin_threshold_mic_lo;
float    earin_threshold_mic_hi;
uint     tape_load_routine;
uint     tape_save_routine;
uint     tape_load_ret_addr;
bool     has_port_7ffd;
bool     has_port_1ffd;
bool     has_port_F4;
bool     has_port_FF;
bool     has_ay_soundchip;
uint32   ay_cycles_per_second;
bool     has_tape_drive;
bool     has_module_port;
bool     has_floppy_drive;
bool     has_printer_port;
uint     has_serial_ports;
uint     has_joystick_ports;
bool     has_kempston_joystick_ports;
bool     has_sinclair_joystick_ports;
bool	hasWaitmap () const			{ return waitmap!=0; }
float	psgCyclesPerSample() const	{ return ay_cycles_per_second / samples_per_second; }
uint32	cpuClockPredivider() const	{ return ula_cycles_per_second / cpu_cycles_per_second; }
bool	isA(isa_id i) const			{ isa_id j=id; do { if(i==j) return yes; } while((j=isa_pid[j])); return no; }
bool	canAttachDivIDE() const		{ return has_zxsp_bus && !isA(isa_MachineTc2068); }
bool	canAttachZxIf2() const		{ return has_zxsp_bus && !isA(isa_MachineTc2068) && !isa_MachineZxPlus2a; }
bool	canAttachSpectraVideo() const { return model<=zxplus2a_span && model!=inves; }
	};

extern ZxInfo zx_info[num_models];

#endif

