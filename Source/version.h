// Copyright (c) 2023 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once

#define APPL_VERSION_H	  0
#define APPL_VERSION_M	  8
#define APPL_VERSION_L	  45
#define APPL_VERSION_STR  "0.8.45"
#define APPL_VERSION_BETA true

static constexpr char startup_info_message[] = "Welcome to version " APPL_VERSION_STR
											   " of zxsp.\n"
											   "This version adds support for the Fuller Box, "
											   "without Orator due to lack of documentation,     "
											   "and i need some feedback for the joystick.\n"
											   "Please report any issue at\n"
											   "github.com/Megatokio/zxsp/issues.";

static constexpr char check_update_url[] = "https://k1.spdns.de/cgi-bin/zxsp-check-update.cgi?version=";
