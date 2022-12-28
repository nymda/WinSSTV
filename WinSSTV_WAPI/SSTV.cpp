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

#include "SSTV.h"
#include "wav.h"

namespace SSTV {
	int clampUC(int input) {
		return (input) > 255 ? 255 : (input) < 0 ? 0 : input;
	}

    void addVoxTone() {
        wav::addTone(1900, 100);
        wav::addTone(1500, 100);
        wav::addTone(1900, 100);
        wav::addTone(1500, 100);
        wav::addTone(2300, 100);
        wav::addTone(1500, 100);
        wav::addTone(2300, 100);
        wav::addTone(1500, 100);
    }

    void addVisCode(char visCode) {
        wav::addTone(1900, 300);
        wav::addTone(1200, 10);
        wav::addTone(1900, 300);
        wav::addTone(1200, 30);

        int bit = 0;
        for (int i = 0; i < 8; i++) {
            bit = (visCode >> i) & 1;
            if (bit) {
                wav::addTone(1100, 30); //1
            }
            else {
                wav::addTone(1300, 30); //0
            }
        }

        wav::addTone(1200, 30);
    }

    void addLongVisCode(short visCode) {
        wav::addTone(1900, 300);
        wav::addTone(1200, 10);
        wav::addTone(1900, 300);
        wav::addTone(1200, 30);

        int bit = 0;
        for (int i = 0; i < 16; i++) {
            bit = (visCode >> i) & 1;
            if (bit) {
                wav::addTone(1100, 30); //1
            }
            else {
                wav::addTone(1300, 30); //0
            }
        }

        wav::addTone(1200, 30);
    }

    SSTV::rgb* resizeNN(SSTV::rgb* input, vec2 inputSize, vec2 newSize) {
        //dont need to do anything if its already the right size, return the origional to save memory
        if (inputSize == newSize) { return 0; }

        printf_s("[Resizing: %ix%i ==> %ix%i]\n", inputSize.X, inputSize.Y, newSize.X, newSize.Y);

        SSTV::rgb* output = new SSTV::rgb[newSize.Y * newSize.X];
        if (!output) { return 0; }

        //calc scale values
        float xScale = (float)newSize.X / (float)inputSize.X;
        float yScale = (float)newSize.Y / (float)inputSize.Y;

        for (int y = 0; y < newSize.Y; y++) {
            for (int x = 0; x < newSize.X; x++) {
                //get the nearest pixel in the input image using the x and y scale values
                int writeIndex = y * newSize.X + x;
                int readIndex = (int)(y / yScale) * inputSize.X + (int)(x / xScale);

                //set the pixel to the closest value, avoid any over/underflows. VS still complains about the possibility.
                if (writeIndex <= (newSize.Y * newSize.X) && readIndex <= (inputSize.X * inputSize.Y) && writeIndex >= 0 && readIndex >= 0) {
                    output[writeIndex] = input[readIndex];
                }
            }
        }

        return output;
    }
}