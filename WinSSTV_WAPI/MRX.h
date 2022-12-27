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

void encodeMR(SSTV::rgb* rgbBuffer, float lineMS) {

    int img_width = MR73.size.X;
    int img_height = MR73.size.Y;

    float mspp_Y = lineMS / (float)img_width;
    float mspp_UV = (lineMS / (float)img_width) / 2;

    float hSyncMs = 9.f;
    float syncPorchMs = 1.f;

    for (int y = 0; y < img_height; y++) {
        wav::addTone(1200, hSyncMs);
        wav::addTone(1500, syncPorchMs);

        //Y scan
        for (int x = 0; x < img_width; x++) {
            SSTV::yuv c(rgbBuffer[(y * img_width) + x]);
            wav::addTone(1500 + (CFMultiplier * c.y), mspp_Y);
        }
        SSTV::yuv c1(rgbBuffer[(y * img_width) + img_width]);
        wav::addTone(1500 + (CFMultiplier * c1.y), 0.1f);
        
        //V scan
        for (int x = 0; x < img_width; x++) {
            SSTV::yuv c(rgbBuffer[(y * img_width) + x]);
            wav::addTone(1500 + (CFMultiplier * c.v), mspp_UV);
        }
        SSTV::yuv c2(rgbBuffer[(y * img_width) + img_width]);
        wav::addTone(1500 + (CFMultiplier * c2.y), 0.1f);

        //U scan
        for (int x = 0; x < img_width; x++) {
            SSTV::yuv c(rgbBuffer[(y * img_width) + x]);
            wav::addTone(1500 + (CFMultiplier * c.u), mspp_UV);
        }
        SSTV::yuv c3(rgbBuffer[(y * img_width) + img_width]);
        wav::addTone(1500 + (CFMultiplier * c3.y), 0.1f);
    }
}

void encodeMR73(SSTV::rgb* rgbBuffer) {
    SSTV::addLongVisCode(0x4523);
	encodeMR(rgbBuffer, 138.f);
}

void encodeMR90(SSTV::rgb* rgbBuffer) {
    SSTV::addLongVisCode(0x4623);
	encodeMR(rgbBuffer, 171.f);
}

void encodeMR115(SSTV::rgb* rgbBuffer) {
    SSTV::addLongVisCode(0x4923);
	encodeMR(rgbBuffer, 220.f);
}

void encodeMR140(SSTV::rgb* rgbBuffer) {
    SSTV::addLongVisCode(0x4a23);
	encodeMR(rgbBuffer, 269.f);
}

void encodeMR175(SSTV::rgb* rgbBuffer) {
    SSTV::addLongVisCode(0x4c23);
	encodeMR(rgbBuffer, 337.f);
}