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

namespace wav {

    enum generatorType {
        GT_SINE = 0,
        GT_SQUARE = 1,
        GT_TRIANGLE = 2
    };
    
    struct wavHeader {
        char riff[4] = { 'R', 'I', 'F', 'F' };
        int fileSize = 44;
        char wave[4] = { 'W', 'A', 'V', 'E' };
        char fmt[4] = { 'f', 'm', 't', 0x20 };
        int headerSize = 16;
        short format = 1;
        short channels = 1;
        int sampleRate = 48000;
        int SBC = 96000;
        short BC = 2;
        short BPS = 16;
        char data[4] = { 'd', 'a', 't', 'a' };
        int dataSize = 0;
    };

    struct monoSample {
        short S;
    };

    int init(int sampleRate);
    void addTone(short frequency, float duration, generatorType gt = generatorType::GT_SINE);
    int save(FILE* fptr);
    void beginPlayback(int iDeviceID);
    void WASAPIListDevices();
    
    extern wavHeader header;
    extern double expectedDurationMS;
    extern double actualDurationMS;
    extern int balance_AddedSamples;
    extern int balance_SkippedSamples;
}


