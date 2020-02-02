
#include "kio/kio.h"



#include "Mouse.h"
Mouse mouse;
Mouse::Mouse():dx(0),dy(0){debugstr("Mouse\n");}
Mouse::~Mouse(){debugstr("~Mouse\n");}
void Mouse::grab(QWidget*){debugstr("Mouse::grab\n");}
void Mouse::ungrab(){debugstr("Mouase::ungrab\n");}


#include "Joystick.h"
void findUsbJoysticks() {debugstr("findUsbJoysticks\n");}


#include "audio/AudioDecoder.h"
AudioDecoder::AudioDecoder(){debugstr("AudioDecoder\n");}
AudioDecoder::~AudioDecoder(){debugstr("~AudioDecoder\n");}
void AudioDecoder::seekSamplePosition(uint32){debugstr("AudioDecoder::seekSamplePosition\n");TODO();}
uint32 AudioDecoder::read(int16*, uint32 max_frames, uint num_channels) {debugstr("AudioDecoder::read\n");TODO();}
void AudioDecoder::open(cstr filename) {debugstr("AudioDecoder::open\n");TODO();}


