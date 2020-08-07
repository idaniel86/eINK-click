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

#ifndef EINK_CLICK_EPD_HPP_
#define EINK_CLICK_EPD_HPP_

#include "ssd16xx.hpp"
#include <functional>

class EPD {
public:
	/**
	 * @brief	Display GS colors.
	 */
	typedef enum {
		COLOR_BLACK = 0, 		///< Black color.
		COLOR_DARG_GRAY	= 1, 	///< Dark gray color.
		COLOR_LIGHT_GRAY = 2,	///< Light gray color.
		COLOR_WHITE = 3, 		///< White color.
	} Color;

	/**
	 * @brief	Text horizontal alignment.
	 */
	typedef enum {
		ALIGN_LEFT = 0,		///< Left horizontal alignment.
		ALIGN_CENTER = 1,	///< Center horizontal alignment.
		ALIGN_RIGHT = 2		///< Right horizontal alignment.
	} Align;

	/**
	 * @brief	Defines the font character range, height etc...
	 */
	typedef struct __attribute__((packed)) {
		uint8_t reserved1;		///< Reserved for future usage (must be set to '0').
		uint8_t user_id;		///< User-assigned ID number.
		uint16_t first_char;	///< Character code of the first font character.
		uint16_t last_char;		///< Character code of the last font character.
		uint8_t height;			///< Character height in pixels.
		uint8_t reserved2;		///< Reserved for future usage (must be set to '0').
	} FontHeader;

	/**
	 * @brief	Defines the character width and offset from the
	 * 			start of the font image.
	 */
	typedef struct __attribute__((packed)) {
		uint32_t width : 8;		///< Width in pixels.
		uint32_t offset : 24;	///< Offset from the start of font image.
	} CharTable;

	/**
	 * @brief	Defines the font image format based on Microchip AN1182.
	 */
	typedef struct __attribute__((packed)) {
		FontHeader header;		///< Font header.
		CharTable char_table[];	///< Font character table.
	} Font;

private:
	/**
	 * @brief	Defines bitmap function.
	 */
	typedef std::function<bool(uint16_t width, uint16_t height, uint16_t w, uint16_t h)> BmpFnc;

	SSD16xx& _ssd;			///< Underlying SSD16xx IC.
	const uint16_t _width;	///< Display width in pixels.
	const uint16_t _height;	///< Display height in pixels.
	const uint8_t* _fntp;	///< Pointer to current font used.
	Color _bkgColor;		///< Current background color.

	/**
	 * @brief	Draw a bitmap on the display based on the bitmap function.
	 * @details	The bitmap function return value represents the actual color
	 * 			used to color a pixel. Logical true stands for the drawing
	 * 			color whereas logical false for background color.
	 *
	 * @param[in] color		drawing @p Color color
	 * @param[in] x			horizontal display start location
	 * @param[in] y			vertical display start location
	 * @param[in] width		bitmap width
	 * @param[in] height	bitmap height
	 * @param[in] bmpFnc	bitmap function used to color pixels
	 */
	void drawBitmap(Color color, uint16_t x, uint16_t y, uint16_t width, uint16_t height, BmpFnc bmpFnc);

public:

	EPD(SSD16xx& ssd, uint16_t width, uint16_t height, const uint8_t* fntp);
	virtual ~EPD();

	/** @brief	Start the underlying SSD16xx IC. */
	void start();

	/** @brief	Stop the underlying SSD16xx IC. */
	void stop();

	/** @brief	Get display width. */
	uint16_t width() const { return _width; }

	/** @brief	Get display height. */
	uint16_t height() const { return _height; }

	/**
	 * @brief
	 */
	void updateDisplay();

	/**
	 * @brief	Fill display with @p Color color.
	 *
	 * @param[in] color		color
	 */
	void fillDisplay(Color color);

	/**
	 * @brief	Set display background color.
	 *
	 * @param[in] color		background color
	 */
	void setBkgColor(Color color) { _bkgColor = color; }

	/**
	 * @brief	Set current font used.
	 *
	 * @param[in] bp		pointer to the font byte representation
	 */
	void setFont(const uint8_t* bp);

	/**
	 * @brief	Gets text width.
	 *
	 * @param[in] fntp		pointer to the @Font font object
	 * @param[in] str		zero terminated text
	 */
	static size_t getTextWidth(const Font* fntp, const char* str);

	/**
	 * @brief	Draw text.
	 *
	 * @param[in] color		drawing color
	 * @param[in] x			horizontal start location
	 * @param[in] y			vertical start location
	 * @param[in] str		zero terminated text
	 * @param[in] align		text alignment to horizontal x start location
	 */
	void drawText(Color color, uint16_t x, uint16_t y, const char* str, Align align = ALIGN_LEFT);

	/**
	 * @brief	Draw filled rectangle.
	 *
	 * @param[in] color		drawing color
	 * @param[in] x			rectangle horizontal start location
	 * @param[in] y			rectangle vertical start location
	 * @param[in] width		rectangle width
	 * @param[in] heigh		rectangle height
	 */
	void drawFilledRect(Color color, uint16_t x, uint16_t y, uint16_t width, uint16_t height);
};

#endif /* EINK_CLICK_EPD_HPP_ */
