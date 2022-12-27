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

void encodeSC(SSTV::rgb* rgbBuffer, double lineMS) {
    int img_width = SC1.size.X;
    int img_height = SC1.size.Y;

    float mspp = lineMS / (float)img_width;

    wav::addTone(1200, 9);
    for (int y = 0; y < img_height; y++) {
        wav::addTone(1500, 1.5);
        for (int x = 0; x < img_width; x++) {
            SSTV::rgb c = rgbBuffer[(y * img_width) + x];
            wav::addTone(1500 + (CFMultiplier * c.g), mspp);
        }

        wav::addTone(1500, 1.5);
        for (int x = 0; x < img_width; x++) {
            SSTV::rgb c = rgbBuffer[(y * img_width) + x];
            wav::addTone(1500 + (CFMultiplier * c.b), mspp);
        }

        wav::addTone(1200, 9);
        wav::addTone(1500, 1.5);
        for (int x = 0; x < img_width; x++) {
            SSTV::rgb c = rgbBuffer[(y * img_width) + x];
            wav::addTone(1500 + (CFMultiplier * c.r), mspp);
        }
    }
}

void encodeSC1(SSTV::rgb* rgbBuffer) {
    SSTV::addVisCode(0x3C);
    encodeSC(rgbBuffer, 138.240);
}

void encodeSC2(SSTV::rgb* rgbBuffer) {
    SSTV::addVisCode(0xB8);
    encodeSC(rgbBuffer, 88.064);
}

void encodeSCDX(SSTV::rgb* rgbBuffer) {
    SSTV::addVisCode(0xCC);
    encodeSC(rgbBuffer, 345.600);
}