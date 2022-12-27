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

#include "textRendering.h"
#include "fontData.h"
#include <cstdarg>

#define CHARX 8
#define CHARY 16

namespace tr {

	//run init before dereferencing this
	SSTV::rgb* rgbFont = 0;
	vec2 fontImageSize = { 144, 101 };
	
	SSTV::rgb* boundCanvas = 0;
	vec2 boundCanvasSize = { 0, 0 };
	
	int iFontSize = 1;	
	vec2 iOrigin = { 0, 0 };

	SSTV::rgb white  = { 255, 255, 255 };
	SSTV::rgb black  = { 0,   0,   0   };
	SSTV::rgb red    = { 255, 0,   0   };
	SSTV::rgb green  = { 0,   255, 0   };
	SSTV::rgb blue   = { 0,   0,   255 };
	SSTV::rgb yellow = { 255, 255, 0   };
	SSTV::rgb cyan   = { 0,   255, 255 };
	SSTV::rgb violet = { 255, 0,   255 };
	
	//this is clearly too many loops within loops but it works and im not sure how to improve it
	int drawCharacter(SSTV::rgb colour, char c, vec2 pos, int fontSize) {		
		int cmIndex = 0;
		for (char cm : fontMap) {
			if (cm == c) {
				
				//i cant remember what these do
				vec2 map = { cmIndex % CHARY, cmIndex / CHARY };
				vec2 mapPosExpanded = { (map.X * CHARX) + ((map.X * CHARX) / CHARX), (map.Y * CHARY) + ((map.Y * CHARY) / CHARY) };
				
				//pixel
				for (int y = 0; y < CHARY; y++) {
					for (int x = 0; x < CHARX; x++) {

						//subpixel
						for (int spY = 0; spY < fontSize; spY++) {
							for (int spX = 0; spX < fontSize; spX++) {
								
								//actual drawing
								int canvasOffset = ((pos.Y + (y * fontSize) + spY) * boundCanvasSize.X) + (pos.X + (x * fontSize) + spX);
								int fontOffset = ((mapPosExpanded.Y + y) * fontImageSize.X) + (mapPosExpanded.X + x);
								if ((canvasOffset <= (boundCanvasSize.X * boundCanvasSize.Y) && canvasOffset >= 0) && (fontOffset <= (fontImageSize.X * fontImageSize.Y) && fontOffset >= 0)) {
									boundCanvas[canvasOffset] = rgbFont[fontOffset] == white ? colour : rgbFont[fontOffset];
								}
							}
						}
					}
				}
				
				break;
			}
			cmIndex++;
		}

		return CHARX * fontSize;
	}

	int drawSpacer(int width, vec2 pos, int fontSize) {
		for (int y = 0; y < CHARY * fontSize; y++) {
			for (int x = 0; x < width * fontSize; x++) {
				boundCanvas[((pos.Y + y) * boundCanvasSize.X) + (pos.X + x)] = { 0, 0, 0 };
			}
		}
		return width * fontSize;
	}

	void setTextOrigin(vec2 origin) {
		iOrigin = origin;
	}

	void bindToCanvas(SSTV::rgb* canvas, vec2 canvasSize) {
		boundCanvas = canvas;
		boundCanvasSize = canvasSize;
	}
	
	//text overrunning the edge of the provided canvas will be truncated
	void drawString(SSTV::rgb colour, int fontSize, const char* fmt...) {
		if (!boundCanvas) { return; }
		
		//format string into buffer
		va_list lst;
		va_start(lst, fmt);
		char buffer[64];
		vsnprintf(buffer, 64, fmt, lst);

		//current offset from the origins X
		int offset = 0;

		//width of the spaces between characters in subpixels
		int spacerWidth = 2;
	
		//draw the background rectangle	
		drawSpacer(spacerWidth, { iOrigin.X, iOrigin.Y }, fontSize);
		offset += spacerWidth * fontSize;
		
		//draw the required characters with 1px between them
		for (int i = 0; i < strlen(buffer); i++) {
			if ((iOrigin.X + offset + (CHARX * fontSize)) > boundCanvasSize.X) {
				return;
			}
			
			offset += drawCharacter(colour, buffer[i], { iOrigin.X + offset, iOrigin.Y }, fontSize);
			offset += drawSpacer(spacerWidth, { iOrigin.X + offset, iOrigin.Y }, fontSize);
		}

		iOrigin.Y += 16 * fontSize;
	}
	
	SSTV::rgb* decompressFontData(char* compressed, int size) {
		SSTV::rgb* decompressed = new SSTV::rgb[size * 8];
		for (int i = 0; i < size; i++) {
			for (int b = 0; b < 8; b++) {
				decompressed[(i * 8) + b] = (compressed[i] & (1 << b)) ? white : black;
			}
		}
		return decompressed;
	}
	
	void initFont() {
		rgbFont = decompressFontData((char*)compressedFontData, sizeof(compressedFontData));
	}
}

