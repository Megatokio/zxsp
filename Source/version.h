// Copyright (c) 2023 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once

#define APPL_VERSION_H	  0
#define APPL_VERSION_M	  8
#define APPL_VERSION_L	  33
#define APPL_VERSION_STR  "0.8.33"
#define APPL_VERSION_BETA true

static constexpr char startup_info_message[] =
	"Welcome to the resurrected-from-the-ashes version of zxsp.\n"
	"This version brings RZX record&replay, perfect ZX80/ZX81 timing & video emulation, "
	"a nice setup for the keyboard joystick and bugs.\n"
	"Please report any issue at\n"
	"github.com/Megatokio/zxsp/issues.";

static constexpr char check_update_url[] = "https://k1.spdns.de/cgi-bin/zxsp-check-update.cgi?version=";
