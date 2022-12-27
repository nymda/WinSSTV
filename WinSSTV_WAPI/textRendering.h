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

namespace tr {

	struct mapping {
		char c;
		int mapX;
		int mapY;
	};
	
	extern SSTV::rgb white;
	extern SSTV::rgb black;
	extern SSTV::rgb red;
	extern SSTV::rgb green;
	extern SSTV::rgb blue;
	extern SSTV::rgb yellow;
	extern SSTV::rgb cyan;
	extern SSTV::rgb violet;
	
	void initFont();
	void drawString(SSTV::rgb colour, int fontSize, const char* fmt...);
	void setTextOrigin(vec2 origin);
	void bindToCanvas(SSTV::rgb* canvas, vec2 canvasSize);
	extern SSTV::rgb* rgbFont;

}