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

//Timings arent quite right but MMSSTV and RX-SSTV can lock on fine, and its done to specifications
//(https://www.yumpu.com/en/document/read/7259948/proposal-for-sstv-mode-specifications)

void encodePD(SSTV::rgb* rgbBuffer, int img_width, int img_height, double lineMS) {
    float mspp = lineMS / (float)img_width;

    float syncMs = 20.f;
    float porchMs = 2.08f;

    for (int y = 0; y < img_height; y += 2) {
        wav::addTone(1200, syncMs);
        wav::addTone(1500, porchMs);

        //Y of even line
        for (int x = 0; x < img_width; x++) { //Y
            SSTV::yuv c(rgbBuffer[(y * img_width) + x]);
            wav::addTone(1500 + (CFMultiplier * c.y), mspp);
        }

        //Avg V and U average of both lines
        for (int x = 0; x < img_width; x++) { //V
            SSTV::yuv c1(rgbBuffer[(y * img_width) + x]);
            SSTV::yuv c2(rgbBuffer[((y + 1) * img_width) + x]);
            wav::addTone(1500 + (CFMultiplier * ((c1.v + c2.v) / 2)), mspp);
        }

        for (int x = 0; x < img_width; x++) { //U
            SSTV::yuv c1(rgbBuffer[(y * img_width) + x]);
            SSTV::yuv c2(rgbBuffer[((y + 1) * img_width) + x]);
            wav::addTone(1500 + (CFMultiplier * ((c1.u + c2.u) / 2)), mspp);
        }

        //Y of odd line
        for (int x = 0; x < img_width; x++) { //Y
            SSTV::yuv c(rgbBuffer[((y + 1) * img_width) + x]);
            wav::addTone(1500 + (CFMultiplier * c.y), mspp);
        }
    }
}

void encodePD50(SSTV::rgb* rgbBuffer) {
    SSTV::addVisCode(0xDD);
    encodePD(rgbBuffer, PD50.size.X, PD50.size.Y, 91.520f);
}

void encodePD90(SSTV::rgb* rgbBuffer) {
    SSTV::addVisCode(0x63);
    encodePD(rgbBuffer, PD90.size.X, PD90.size.Y, 170.240f);
}

void encodePD120(SSTV::rgb* rgbBuffer) {
    SSTV::addVisCode(0x5F);
    encodePD(rgbBuffer, PD120.size.X, PD120.size.Y, 121.600f);
}

void encodePD160(SSTV::rgb* rgbBuffer) {
    SSTV::addVisCode(0xE2);
    encodePD(rgbBuffer, PD160.size.X, PD160.size.Y, 195.584f);
}

void encodePD180(SSTV::rgb* rgbBuffer) {
    SSTV::addVisCode(0x60);
    encodePD(rgbBuffer, PD180.size.X, PD180.size.Y, 183.040f);
}

void encodePD240(SSTV::rgb* rgbBuffer) {
    SSTV::addVisCode(0xE1);
    encodePD(rgbBuffer, PD240.size.X, PD240.size.Y, 244.480f);
}

void encodePD290(SSTV::rgb* rgbBuffer) {
    SSTV::addVisCode(0xDE);
    encodePD(rgbBuffer, PD290.size.X, PD290.size.Y, 228.800f);
}