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

#ifndef EINK_CLICK_SSD1606_HPP_
#define EINK_CLICK_SSD1606_HPP_

#include "ssd16xx.hpp"

/**
 * @brief	SSD1606 driver.
 */
class SSD1606: public SSD16xx {
public:
	SSD1606(SPIDriver& spi, const SPIConfig& spiCfg, ioline_t rstLine, ioline_t busyLine, ioline_t dcLine)
	: SSD16xx(spi, spiCfg, rstLine, busyLine, dcLine)
	{}

	virtual uint16_t sources() const { return 72; }
	virtual uint16_t gates() const { return 172; }

	virtual void sendLUTData() {
		static constexpr uint8_t LUTData[]= {
			0x82,0x00,0x00,0x00,	// step 0
			0xAA,0x00,0x00,0x00,
			0xAA,0xAA,0x00,0x00,
			0xAA,0xAA,0xAA,0x00,
			0x55,0xAA,0xAA,0x00,
			0x55,0x55,0x55,0x55,
			0xAA,0xAA,0xAA,0xAA,
			0x55,0x55,0x55,0x55,
			0xAA,0xAA,0xAA,0xAA,
			0x15,0x15,0x15,0x15,
			0x05,0x05,0x05,0x05,
			0x01,0x01,0x01,0x01,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,	// step 19

			// timing part of the LUT
			0x41,0x45,0xF1,0xFF,0x5F,0x55,0x01,0x00,
			0x00,0x00
		};

		sendData(LUTData, sizeof(LUTData));
	}
};


#endif /* EINK_CLICK_SSD1606_HPP_ */
