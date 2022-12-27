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

#include "modes.h"
#include "BWX.h" //BW8, BW12
#include "SCX.h" //Scottie1, Scottie2, ScottieDX
#include "R24.h" //Robot24
#include "R36.h" //Robot36
#include "R72.h" //Robot72
#include "PDX.h" //PD50, PD90, PD120
#include "MAX.h" //Martin1, Martin2 
#include "AVT.h" //AVT90
#include "MRX.h" //MR73, MR90, MR115, MR140, MR175

encMode BW8 =   { EM_BW8,   "BW8",   "Monochrome 8s",   {160, 120}, (encCall)&encodeBW8};
encMode BW12 =  { EM_BW12,  "BW12",  "Monochrome 12s",  {160, 120}, (encCall)&encodeBW12 };
encMode R24 =   { EM_R24,   "R24",   "Robot24",         {160, 120}, (encCall)&encodeR24 };
encMode R36 =   { EM_R36,   "R36",   "Robot36",         {320, 240}, (encCall)&encodeR36 };
encMode R72 =   { EM_R72,   "R72",   "Robot72",         {320, 240}, (encCall)&encodeR72 };
encMode SC1 =   { EM_SC1,   "SC1",   "Scottie 1",       {320, 256}, (encCall)&encodeSC1 };
encMode SC2 =   { EM_SC2,   "SC2",   "Scottie 2",       {320, 256}, (encCall)&encodeSC2 };
encMode SCDX =  { EM_SCDX,  "SCDX",  "Scottie DX",      {320, 256}, (encCall)&encodeSCDX };
encMode MR1 =   { EM_MR1,   "MR1",   "Martin 1",        {320, 256}, (encCall)&encodeMA1 };
encMode MR2 =   { EM_MR2,   "MR2",   "Martin 2",        {320, 256}, (encCall)&encodeMA2 };
encMode AVT90 = { EM_AVT90, "AVT90", "AVT 90",          {320, 240}, (encCall)&encodeAVT90 };
encMode MR73 =  { EM_MR73,  "MR73",  "MR 73",           {320, 240}, (encCall)&encodeMR73 };
encMode MR90 =  { EM_MR90,  "MR90",  "MR 90",           {320, 240}, (encCall)&encodeMR90 };
encMode MR115 = { EM_MR115, "MR115", "MR 115",          {320, 240}, (encCall)&encodeMR115 };
encMode MR140 = { EM_MR140, "MR140", "MR 140",          {320, 240}, (encCall)&encodeMR140 };
encMode MR175 = { EM_MR175, "MR175", "MR 175",          {320, 240}, (encCall)&encodeMR175 };
encMode PD50 =  { EM_PD50,  "PD50",  "PD50",            {320, 256}, (encCall)&encodePD50 };
encMode PD90 =  { EM_PD90,  "PD90",  "PD90",            {320, 256}, (encCall)&encodePD90 };
encMode PD120 = { EM_PD120, "PD120", "PD120",           {640, 496}, (encCall)&encodePD120 };
encMode PD160 = { EM_PD160, "PD160", "PD160",           {512, 400}, (encCall)&encodePD160 };
encMode PD180 = { EM_PD180, "PD180", "PD180",           {640, 496}, (encCall)&encodePD180 };
encMode PD240 = { EM_PD240, "PD240", "PD240",           {640, 496}, (encCall)&encodePD240 };
encMode PD290 = { EM_PD290, "PD290", "PD290",           {800, 616}, (encCall)&encodePD290 };
encMode modes[] = { BW8, BW12, R24, R36, R72, SC1, SC2, SCDX, MR1, MR2, AVT90, MR73, MR90, MR115, MR140, MR175, PD50, PD90, PD120, PD160, PD180, PD240, PD290 };