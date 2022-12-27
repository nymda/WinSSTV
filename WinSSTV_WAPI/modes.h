/*
 * This file is part of CLSSTV (https://github.com/nymda/CLSSTV).
 * Copyright (c) 2022 github/nymda
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#include "SSTV.h"

typedef void* (__fastcall* encCall)(SSTV::rgb* bitmap);

enum encModeID {
	EM_BW8,
	EM_BW12,
	EM_R24,
	EM_R36,
	EM_R72,
	EM_SC1,
	EM_SC2,
	EM_SCDX,
	EM_MR1,
	EM_MR2,
	EM_AVT90,
	EM_MR73,
	EM_MR90,
	EM_MR115,
	EM_MR140,
	EM_MR175,
	EM_PD50,
	EM_PD90,
	EM_PD120,
	EM_PD160,
	EM_PD180,
	EM_PD240,
	EM_PD290
};

struct encMode {
	encModeID ID;
	char code[8];
	char desc[128];
	vec2 size;
	encCall ec;
};

extern encMode BW8;
extern encMode BW12;
extern encMode R24;
extern encMode R36;
extern encMode R72;
extern encMode SC1;
extern encMode SC2;
extern encMode SCDX;
extern encMode MR1;
extern encMode MR2;
extern encMode AVT90;
extern encMode MR73;
extern encMode MR90;
extern encMode MR115;
extern encMode MR140;
extern encMode MR175;
extern encMode PD50;
extern encMode PD90;
extern encMode PD120;
extern encMode PD160;
extern encMode PD180;
extern encMode PD240;
extern encMode PD290;

extern encMode modes[64];