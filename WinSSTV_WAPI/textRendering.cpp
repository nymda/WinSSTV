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

#include "textRendering.h"
#include "fontData.h"
#include <cstdarg>

#define CHARX 8
#define CHARY 16

namespace tr {

	//run init before dereferencing this
	SSTV::rgb* rgbFont = 0;
	SSTV::vec2 fontImageSize = { 144, 101 };
	
	SSTV::rgb* boundCanvas = 0;
	SSTV::vec2 boundCanvasSize = { 0, 0 };
	
	int iFontSize = 1;	
	SSTV::vec2 iOrigin = { 0, 0 };

	SSTV::rgb white  = { 255, 255, 255 };
	SSTV::rgb black  = { 0,   0,   0   };
	SSTV::rgb red    = { 255, 0,   0   };
	SSTV::rgb green  = { 0,   255, 0   };
	SSTV::rgb blue   = { 0,   0,   255 };
	SSTV::rgb yellow = { 255, 255, 0   };
	SSTV::rgb cyan   = { 0,   255, 255 };
	SSTV::rgb violet = { 255, 0,   255 };
	
	//this is clearly too many loops within loops but it works and im not sure how to improve it
	int drawCharacter(SSTV::rgb colour, wchar_t c, SSTV::vec2 pos, int fontSize) {
		int cmIndex = 0;

		for (wchar_t cm : fontMap) {
			if (cm == c) {
				
				//i cant remember what these do
				SSTV::vec2 map = { cmIndex % CHARY, cmIndex / CHARY };
				SSTV::vec2 mapPosExpanded = { (map.X * CHARX) + ((map.X * CHARX) / CHARX), (map.Y * CHARY) + ((map.Y * CHARY) / CHARY) };
				
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

	int drawSpacer(int width, SSTV::vec2 pos, int fontSize) {
		for (int y = 0; y < CHARY * fontSize; y++) {
			for (int x = 0; x < width * fontSize; x++) {
				int drawIndex = ((pos.Y + y) * boundCanvasSize.X) + (pos.X + x);
				if (drawIndex <= (boundCanvasSize.X * boundCanvasSize.Y) && drawIndex >= 0) {
					boundCanvas[drawIndex] = black;
				}
			}
		}
		return width * fontSize;
	}

	void setTextOrigin(SSTV::vec2 origin) {
		iOrigin = origin;
	}

	void bindToCanvas(SSTV::simpleBitmap* canvas) {
		boundCanvas = canvas->data;
		boundCanvasSize = canvas->size;
	}
	
	//text overrunning the edge of the provided canvas will be truncated
	void drawString(SSTV::rgb colour, int fontSize, const wchar_t* string) {
		if (!boundCanvas) { return; }

		//current offset from the origins X
		int offset = 0;

		//width of the spaces between characters in subpixels
		int spacerWidth = 2;
		
		//draw the background rectangle	
		offset += drawSpacer(spacerWidth, { iOrigin.X, iOrigin.Y }, fontSize);
		
		//draw the required characters with 1px between them
		for (int i = 0; i < wcslen(string); i++) {
			if ((iOrigin.X + offset + (CHARX * fontSize)) > boundCanvasSize.X || string[i] == L';') {
				iOrigin.Y += 16 * fontSize;
				offset = drawSpacer(spacerWidth, { iOrigin.X, iOrigin.Y }, fontSize);

				if (string[i] == L';') { continue; }
			}
			
			offset += drawCharacter(colour, string[i], { iOrigin.X + offset, iOrigin.Y }, fontSize);
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

