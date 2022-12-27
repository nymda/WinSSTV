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
#include "wav.h"
#include "modes.h"

//AVT90, because fuck sync pulses
void encodeAVT90(SSTV::rgb* rgbBuffer) {
    SSTV::addVisCode(0x44);

    int img_width = AVT90.size.X;
    int img_height = AVT90.size.Y;

    float mspp = 125.f / (float)img_width;

    for (int y = 0; y < img_height; y++) {
        //R scan
        for (int x = 0; x < img_width; x++) {
            SSTV::rgb c = rgbBuffer[(y * img_width) + x];
            wav::addTone(1500 + (CFMultiplier * c.r), mspp);
        }

        //G scan
        for (int x = 0; x < img_width; x++) {
            SSTV::rgb c = rgbBuffer[(y * img_width) + x];
            wav::addTone(1500 + (CFMultiplier * c.g), mspp);
        }

        //B scan
        for (int x = 0; x < img_width; x++) {
            SSTV::rgb c = rgbBuffer[(y * img_width) + x];
            wav::addTone(1500 + (CFMultiplier * c.b), mspp);
        }
    }
}