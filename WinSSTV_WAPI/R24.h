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

//R24 feels like a missed opportunity, it could have been much faster using the R36 style colouring of sending Y and U on alternating lines

void encodeR24(SSTV::rgb* rgbBuffer) {
    SSTV::addVisCode(0x84);

    int img_width = R24.size.X;
    int img_height = R24.size.Y;

    float mspp_Y = 92.f / (float)img_width;
    float mspp_UV = 46.f / (float)img_width;

    float hSyncMs = 6.f;
    float syncPorchMs = 2.f;
    float separatorMs = 3.f;
    float sepPorchMs = 1.f;

    for (int y = 0; y < img_height; y++) {
        wav::addTone(1200, hSyncMs);
        wav::addTone(1500, syncPorchMs);

        //Y scan
        for (int x = 0; x < img_width; x++) {
            SSTV::yuv c(rgbBuffer[(y * img_width) + x]);
            wav::addTone(1500 + (CFMultiplier * c.y), mspp_Y);
        }

        wav::addTone(1500, separatorMs);
        wav::addTone(1900, sepPorchMs);

        //V scan
        for (int x = 0; x < img_width; x++) {
            SSTV::yuv c(rgbBuffer[(y * img_width) + x]);
            wav::addTone(1500 + (CFMultiplier * c.v), mspp_UV);
        }

        wav::addTone(2300, separatorMs);
        wav::addTone(1900, sepPorchMs);

        //U scan
        for (int x = 0; x < img_width; x++) {
            SSTV::yuv c(rgbBuffer[(y * img_width) + x]);
            wav::addTone(1500 + (CFMultiplier * c.u), mspp_UV);
        }
    }
}