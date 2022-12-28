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

    int resizeNN(SSTV::simpleBitmap* input, SSTV::simpleBitmap* output) {
        //dont need to run the whole resize if they are both the same, just copy the input to the output
        if (input->size == output->size) {
            printf_s("[Resize not required, copying image]\n");
            if (output->data) { free(output->data); }
            output->data = (SSTV::rgb*)malloc((output->size.X * output->size.Y) * sizeof(SSTV::rgb));
            memcpy(output->data, input->data, (input->size.X * input->size.Y) * sizeof(SSTV::rgb));
            return -1;
        }

        printf_s("[Resizing: %ix%i ==> %ix%i]\n", input->size.X, input->size.Y, output->size.X, output->size.Y);

        if (output->data) { free(output->data); }
        output->data = (SSTV::rgb*)malloc((output->size.X * output->size.Y) * sizeof(SSTV::rgb));
        if (!output->data) { return 0; }

        //calc scale values
        float xScale = (float)output->size.X / (float)input->size.X;
        float yScale = (float)output->size.Y / (float)input->size.Y;

        for (int y = 0; y < output->size.Y; y++) {
            for (int x = 0; x < output->size.X; x++) {
                //get the nearest pixel in the input image using the x and y scale values
                int writeIndex = y * output->size.X + x;
                int readIndex = (int)(y / yScale) * input->size.X + (int)(x / xScale);

                //set the pixel to the closest value, avoid any over/underflows. VS still complains about the possibility.
                if (writeIndex <= (output->size.Y * output->size.X) && readIndex <= (input->size.X * input->size.Y) && writeIndex >= 0 && readIndex >= 0) {
                    output->data[writeIndex] = input->data[readIndex];
                }
                else {
                    printf_s("Copy overrun at index %i -> %i\n", writeIndex, readIndex);
                }
            }
        }

        return 1;
    }

    int setColours(SSTV::simpleBitmap* image, SSTV::RGBMode mode) {
        if (mode == SSTV::RGBMode::R) {
            for (int x = 0; x < image->size.X * image->size.Y; x++) {
                SSTV::rgb* px = &image->data[x];
                px->g = px->r;
                px->b = px->r;
            }
            return 0;
        }
        if (mode == SSTV::RGBMode::G) {
            for (int x = 0; x < image->size.X * image->size.Y; x++) {
                SSTV::rgb* px = &image->data[x];
                px->r = px->g;
                px->b = px->g;
            }
            return 0;
        }
        if (mode == SSTV::RGBMode::B) {
            for (int x = 0; x < image->size.X * image->size.Y; x++) {
                SSTV::rgb* px = &image->data[x];
                px->r = px->b;
                px->g = px->b;
            }
            return 0;
        }
        if (mode == SSTV::RGBMode::MONO) { //mono is not selectable by the user, its just used in the BW modes
            for (int x = 0; x < image->size.X * image->size.Y; x++) {
                unsigned char y = SSTV::yuv(image->data[x]).y;
                image->data[x].r = y;
                image->data[x].g = y;
                image->data[x].b = y;
            }
            return 0;
        }
    }
}