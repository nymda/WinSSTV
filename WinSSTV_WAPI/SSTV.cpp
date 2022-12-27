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
}