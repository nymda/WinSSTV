 /*
 * This file is part of WinSSTV (https://github.com/nymda/WinSSTV).
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

encMode BW8 =   { EM_BW8,   L"BW8",   L"Monochrome 8s",   {160, 120}, (encCall)&encodeBW8,    {(char )0x82}  };
encMode BW12 =  { EM_BW12,  L"BW12",  L"Monochrome 12s",  {160, 120}, (encCall)&encodeBW12,   {(char )0x86}   };
encMode R24 =   { EM_R24,   L"R24",   L"Robot24",         {160, 120}, (encCall)&encodeR24,    {(char )0x84}   };
encMode R36 =   { EM_R36,   L"R36",   L"Robot36",         {320, 240}, (encCall)&encodeR36,    {(char )0x88}   };
encMode R72 =   { EM_R72,   L"R72",   L"Robot72",         {320, 240}, (encCall)&encodeR72,    {(char )0x0C}   };
encMode SC1 =   { EM_SC1,   L"SC1",   L"Scottie 1",       {320, 256}, (encCall)&encodeSC1,    {(char )0x3C}   };
encMode SC2 =   { EM_SC2,   L"SC2",   L"Scottie 2",       {320, 256}, (encCall)&encodeSC2,    {(char )0xB8}   };
encMode SCDX =  { EM_SCDX,  L"SCDX",  L"Scottie DX",      {320, 256}, (encCall)&encodeSCDX,   {(char )0xCC}   };
encMode MR1 =   { EM_MR1,   L"MR1",   L"Martin 1",        {320, 256}, (encCall)&encodeMA1,    {(char )0xAC}   };
encMode MR2 =   { EM_MR2,   L"MR2",   L"Martin 2",        {320, 256}, (encCall)&encodeMA2,    {(char )0x28}   };
encMode AVT90 = { EM_AVT90, L"AVT90", L"AVT 90",          {320, 240}, (encCall)&encodeAVT90,  {(char )0x44}   };
encMode MR73 =  { EM_MR73,  L"MR73",  L"MR 73",           {320, 240}, (encCall)&encodeMR73,   {(short)0x4523} };
encMode MR90 =  { EM_MR90,  L"MR90",  L"MR 90",           {320, 240}, (encCall)&encodeMR90,   {(short)0x4623} };
encMode MR115 = { EM_MR115, L"MR115", L"MR 115",          {320, 240}, (encCall)&encodeMR115,  {(short)0x4923} };
encMode MR140 = { EM_MR140, L"MR140", L"MR 140",          {320, 240}, (encCall)&encodeMR140,  {(short)0x4a23} };
encMode MR175 = { EM_MR175, L"MR175", L"MR 175",          {320, 240}, (encCall)&encodeMR175,  {(short)0x4c23} };
encMode PD50 =  { EM_PD50,  L"PD50",  L"PD50",            {320, 256}, (encCall)&encodePD50,   {(char )0xDD}   };
encMode PD90 =  { EM_PD90,  L"PD90",  L"PD90",            {320, 256}, (encCall)&encodePD90,   {(char )0x63}   };
encMode PD120 = { EM_PD120, L"PD120", L"PD120",           {640, 496}, (encCall)&encodePD120,  {(char )0x5F}   };
encMode PD160 = { EM_PD160, L"PD160", L"PD160",           {512, 400}, (encCall)&encodePD160,  {(char )0xE2}   };
encMode PD180 = { EM_PD180, L"PD180", L"PD180",           {640, 496}, (encCall)&encodePD180,  {(char )0x60}   };
encMode PD240 = { EM_PD240, L"PD240", L"PD240",           {640, 496}, (encCall)&encodePD240,  {(char )0xE1}   };
encMode PD290 = { EM_PD290, L"PD290", L"PD290",           {800, 616}, (encCall)&encodePD290,  {(char )0xDE}   };
encMode modes[] = { BW8, BW12, R24, R36, R72, SC1, SC2, SCDX, MR1, MR2, AVT90, MR73, MR90, MR115, MR140, MR175, PD50, PD90, PD120, PD160, PD180, PD240, PD290 };