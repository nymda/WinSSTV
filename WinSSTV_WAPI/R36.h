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

void encodeR36(SSTV::rgb* rgbBuffer) {
    SSTV::addVisCode(0x88);

    int img_width = R36.size.X;
    int img_height = R36.size.Y;

    float mspp_Y = 88.f / (float)img_width;
    float mspp_UV = 44.f / (float)img_width;

    float hSyncMs = 9.f;
    float syncPorchMs = 3.f;
    float separatorMs = 4.5f;
    float sepPorchMs = 1.5f;

    for (int y = 0; y < img_height; y++) {
        wav::addTone(1200, hSyncMs);
        wav::addTone(1500, syncPorchMs);

        //Y scan
        for (int x = 0; x < img_width; x++) {
            SSTV::yuv c(rgbBuffer[(y * img_width) + x]);
            wav::addTone(1500 + (CFMultiplier * c.y), mspp_Y);
        }

        if (y % 2 == 0) {
            //evens
            wav::addTone(1500, separatorMs);
            wav::addTone(1900, sepPorchMs);

            //V scan
            for (int x = 0; x < img_width; x++) {
                SSTV::yuv c1(rgbBuffer[(y * img_width) + x]);
                SSTV::yuv c2(rgbBuffer[((y + 1) * img_width) + x]);
                wav::addTone(1500 + (CFMultiplier * ((c1.v + c2.v) / 2)), mspp_UV);
            }
        }
        else {
            //odds
            wav::addTone(2300, separatorMs);
            wav::addTone(1900, sepPorchMs);

            //U scan
            for (int x = 0; x < img_width; x++) {
                SSTV::yuv c1(rgbBuffer[(y * img_width) + x]);
                SSTV::yuv c2(rgbBuffer[((y - 1) * img_width) + x]);
                wav::addTone(1500 + (CFMultiplier * ((c1.u + c2.u) / 2)), mspp_UV);
            }
        }
    }
}
