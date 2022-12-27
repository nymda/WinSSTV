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

/*
    These are kind of a mess as BW8 and BW12 are not very well documented.
    All timing values were taken from the MMSSTV source code.
*/

void encodeBW(SSTV::rgb* rgbBuffer, double lineMS) {
    int img_width = BW8.size.X;
    int img_height = BW8.size.Y;

    float mspp = lineMS / static_cast<float>(img_width);
    for (int y = 0; y < img_height; y++) {
        wav::addTone(1200, 6.f);
        wav::addTone(1500, 2.f);
        for (int x = 0; x < img_width; x++) {
            SSTV::yuv c(rgbBuffer[(y * img_width) + x]);
            wav::addTone(1500 + (CFMultiplier * c.y), mspp); //Luminance (Y) only, average of RGB also works.
        }
    }
}

void encodeBW8(SSTV::rgb* rgbBuffer) {
    SSTV::addVisCode(0x82);
    encodeBW(rgbBuffer, 58.89709f);
}

void encodeBW12(SSTV::rgb* rgbBuffer) {
    SSTV::addVisCode(0x86);
    encodeBW(rgbBuffer, 92.f);
}