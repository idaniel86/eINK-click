/*
 * Copyright (C) 2020 Daniel Igaz
 *
 * This file is part of the eINK-click project.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "epd.hpp"

EPD::EPD(SSD16xx& ssd, uint16_t width, uint16_t height, const uint8_t* fntp)
: _ssd(ssd)
, _width(width)
, _height(height)
{
	osalDbgAssert(width <= ssd.gates() && height <= ssd.sources(),
			"EPD::EPD, invalid size");

	setFont(fntp);
	setBkgColor(COLOR_WHITE);
}

EPD::~EPD()
{
}

void EPD::start()
{
	_ssd.start();
}

void EPD::stop()
{
	_ssd.stop();
}

void EPD::updateDisplay()
{
	_ssd.update();
}

void EPD::fillDisplay(Color color)
{
	uint8_t b = (color << 6) | (color << 4) | (color << 2) | (color << 0);

	_ssd.select();

	// set address window
	_ssd.setAddress(0, (_height >> 2) - 1, _width - 1, 0);

	// fill with color
	for (uint32_t i = 0; i < uint32_t(_width * (_height >> 2)); i++)
		_ssd.sendData(b);

	_ssd.unselect();
}

void EPD::setFont(const uint8_t* bp)
{
	osalDbgCheck(bp != NULL);

	const Font* fntp = (const Font*)bp;

	osalDbgAssert(fntp->header.first_char <= fntp->header.last_char &&
			fntp->header.height <= _height,
			"ssd1606SetFont(), invalid font");

	_fntp = bp;
}

void EPD::drawBitmap(Color color, uint16_t x, uint16_t y, uint16_t width, uint16_t height, BmpFnc bmpFnc)
{
	uint8_t xsa = y >> 2;
	uint8_t xea = ((y + height + 3) >> 2) - 1;
	uint16_t ysa = _width - 1 - x;
	uint16_t yea = _width - 1 - (x - 1 + width);

	// set background color
	uint8_t b = (_bkgColor << 6) | (_bkgColor << 4) | (_bkgColor << 2) | (_bkgColor << 0);

	_ssd.select();

	// set address window
	_ssd.setAddress(xsa, xea, ysa, yea);

	// draw the character bitmap
	for (uint16_t w = 0; w < width; w++) {
		for (uint16_t h = 0; h < height; h++) {
			if (bmpFnc(width, height, w, h)) {
				// mask out background color
				b &= ~(0x03 << ((3 - ((h + y) & 0x03)) << 1));
				// set new color
				b |= (color << ((3 - ((h + y) & 0x03)) << 1));
			}

			// send data byte to RAM after each 4 pixels or at the last pixel
			if (((h + y) & 0x03) == 0x03 || (h == (height - 1))) {
				_ssd.sendData(b);

				// set background color
				b = (_bkgColor << 6) | (_bkgColor << 4) | (_bkgColor << 2) | (_bkgColor << 0);
			}
		}
	}

	_ssd.unselect();
}

size_t EPD::getTextWidth(const Font* fntp, const char* str)
{
	size_t width = 0;

	while (*str != 0) {
		if (fntp->header.first_char <= *str && *str <= fntp->header.last_char)
			width += fntp->char_table[*str - fntp->header.first_char].width;
		str++;
	}

	return width;
}

void EPD::drawText(Color color, uint16_t x, uint16_t y, const char* str, Align align)
{
	osalDbgCheck(str != NULL);

	const Font* fntp = (const Font*)_fntp;
	uint16_t height = fntp->header.height;
	uint16_t width;
	const uint8_t* bp;

	// return if outside vertical display area
	if (y + height > _height)
		return;

	// adjust horizontal position based on alignment
	switch (align) {
	case ALIGN_CENTER: {
		size_t textHalfWidth = (getTextWidth(fntp, str) >> 1);
		x -= (x < textHalfWidth) ? x : textHalfWidth;
		break;
	}
	case ALIGN_RIGHT: {
		size_t textWidth = getTextWidth(fntp, str);
		x -= (x < textWidth) ? x : textWidth;
		break;
	}
	default:
		break;
	}

	while (*str != 0) {
		if (fntp->header.first_char <= *str && *str <= fntp->header.last_char) {
			width = fntp->char_table[*str - fntp->header.first_char].width;
			bp = _fntp + fntp->char_table[*str - fntp->header.first_char].offset;

			// return if outside horizontal display area
			if (x + width > _width)
				return;

			BmpFnc bmpFnc = [bp](uint16_t width, uint16_t height, uint16_t w, uint16_t h) -> bool {
				(void) height;
				return bool((bp[(h * ((width + 7) & 0xF8) + (w & 0xF8)) >> 3] >> (w & 0x07)) & 0x01);
			};

			drawBitmap(color, x, y, width, height, bmpFnc);
			x += width;
		}
		str++;
	}
}

void EPD::drawFilledRect(Color color, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	BmpFnc bmpFnc = [](uint16_t width, uint16_t height, uint16_t w, uint16_t h) -> bool {
		(void) width;
		(void) height;
		(void) w;
		(void) h;
		return true;
	};

	drawBitmap(color, x, y, width, height, bmpFnc);
}
