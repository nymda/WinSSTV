/*
 * This file is part of WinSSTV (https://github.com/nymda/WinSSTV).
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
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

//used in the sum (1500 + X * CFMultiplier) to convert a byte to the approximate SSTV complient colour / luminocity frequency
const double CFMultiplier = 3.1372549;

namespace SSTV {
    int clampUC(int input);

    enum RGBMode {
        RGB = 0,
        R = 1,
        G = 2,
        B = 3,
        MONO = 4,
    };

    struct vec2 {
        int X;
        int Y;

        bool operator == (const vec2& rhs)
        {
            if (X == rhs.X && Y == rhs.Y) {
                return true;
            }
            return false;
        }

        bool operator != (const vec2& rhs)
        {
            return !(*this == rhs);
        }
    };

    struct fVec2 {
        float X;
        float Y;

        bool operator == (const fVec2& rhs)
        {
            if (X == rhs.X && Y == rhs.Y) {
                return true;
            }
            return false;
        }

        bool operator != (const fVec2& rhs)
        {
            return !(*this == rhs);
        }
    };
    
    struct rgb {
        unsigned char r;
        unsigned char g;
        unsigned char b;
        
        bool operator == (const rgb& rhs)
        {
            if (r == rhs.r && g == rhs.g && b == rhs.b) {
                return true;
            }
            return false;
        }

        bool operator != (const rgb& rhs)
        {
            return !(*this == rhs);
        }
        
        rgb(unsigned char R, unsigned char G, unsigned char B) {
            r = R;
            g = G;
            b = B;
        }

        rgb() {
            r = 0;
            g = 0;
            b = 0;
        }
    };

    struct yuv {
        unsigned char y;
        unsigned char u;
        unsigned char v;

        yuv(rgb rgb) {
            y = clampUC(((66 * (rgb.r) + 129 * (rgb.g) + 25 * (rgb.b) + 128) >> 8) + 16);
            u = clampUC(((-38 * (rgb.r) - 74 * (rgb.g) + 112 * (rgb.b) + 128) >> 8) + 128);
            v = clampUC(((112 * (rgb.r) - 94 * (rgb.g) - 18 * (rgb.b) + 128) >> 8) + 128);
        }
    };

    struct simpleBitmap {
        SSTV::vec2 size = { 0, 0 };
        SSTV::rgb* data = 0;
    };
    
    struct simpleBitmapYUV {
        SSTV::vec2 size;
        SSTV::yuv* data;
    };
    
    //adds the beeboobeebooBEEBOOBEEBOO
    void addVoxTone();

    //adds all 8 bytes of the input, without calculating the parity bit
    void addVisCode(char visCode);

    //adds a 16 bit vis code
    void addLongVisCode(short visCode);

    //resizes an image. Input should be a fully formed simpleBitmap, output's data will be freed and reallocated.
    int resizeNN(SSTV::simpleBitmap* input, SSTV::simpleBitmap* output);

    //sets an image to only use one channel
    int setColours(SSTV::simpleBitmap* image, SSTV::RGBMode mode);
}

