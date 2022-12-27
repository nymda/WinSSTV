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

void encodeMA(SSTV::rgb* rgbBuffer, double lineMS) {
    int img_width = MR1.size.X;
    int img_height = MR1.size.Y;

    float mspp = lineMS / (float)img_width;
    float syncMs = 4.862f;
    float seprMs = 0.572f;

    for (int y = 0; y < img_height; y++) {
        wav::addTone(1200, syncMs);
        wav::addTone(1500, seprMs);

        for (int x = 0; x < img_width; x++) {
            SSTV::rgb c = rgbBuffer[(y * img_width) + x];
            wav::addTone(1500 + (CFMultiplier * c.g), mspp);
        }
        wav::addTone(1500, seprMs);

        for (int x = 0; x < img_width; x++) {
            SSTV::rgb c = rgbBuffer[(y * img_width) + x];
            wav::addTone(1500 + (CFMultiplier * c.b), mspp);
        }
        wav::addTone(1500, seprMs);

        for (int x = 0; x < img_width; x++) {
            SSTV::rgb c = rgbBuffer[(y * img_width) + x];
            wav::addTone(1500 + (CFMultiplier * c.r), mspp);
        }
        wav::addTone(1500, seprMs);
    }
}

void encodeMA1(SSTV::rgb* rgbBuffer) {
    SSTV::addVisCode(0xAC);
    encodeMA(rgbBuffer, 146.432f);
}

void encodeMA2(SSTV::rgb* rgbBuffer) {
    SSTV::addVisCode(0x28);
    encodeMA(rgbBuffer, 73.216f);
}