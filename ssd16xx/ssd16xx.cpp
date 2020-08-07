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

#include "ssd16xx.hpp"

SSD16xx::SSD16xx(SPIDriver& spi, const SPIConfig& spiCfg, ioline_t rstLine, ioline_t busyLine, ioline_t dcLine)
: _spi(&spi)
, _spiCfg(&spiCfg)
, _rstLine(rstLine)
, _busyLine(busyLine)
, _dcLine(dcLine)
{
}

SSD16xx::~SSD16xx()
{
}

void SSD16xx::sendCmd(Command c)
{
	palClearLine(_dcLine);
	spiSend(_spi, 1, &c);
	palSetLine(_dcLine);
}

void SSD16xx::sendData(uint8_t b)
{
	spiSend(_spi, 1, &b);
}

void SSD16xx::sendData(const uint8_t* bp, size_t n)
{
	spiSend(_spi, n, bp);
}

void SSD16xx::select()
{
#if	SPI_USE_MUTUAL_EXCLUSION
	spiAcquireBus(_spi);
	spiStart(_spi, _spiCfg);
#endif

	spiSelect(_spi);
}

void SSD16xx::unselect()
{
	spiUnselect(_spi);

#if	SPI_USE_MUTUAL_EXCLUSION
	spiReleaseBus(_spi);
#endif
}

void SSD16xx::start()
{
	// set PWM analog pin to push-pull
	palSetLineMode(_dcLine, PAL_MODE_OUTPUT_PUSHPULL);

	// panel reset
	palClearLine(_rstLine);
	chThdSleepMilliseconds(10);
	palSetLine(_rstLine);
	chThdSleepMilliseconds(10);

#if	SPI_USE_MUTUAL_EXCLUSION
	spiAcquireBus(_spi);
#endif
	spiStart(_spi, _spiCfg);

	spiSelect(_spi);

	// data entry mode setting, increment X, decrement Y
	sendCmd(SSD16xx_DEMDS);
	sendData(0x01);

	// write VCOM register
	sendCmd(SSD16xx_WVCOMREG);
	sendData(0xA0);

	// board waveform, board voltage
	sendCmd(SSD16xx_VBDSET);
	sendData(0x63);

	// enable sequence, CLK->CP->
	sendCmd(SSD16xx_DUPCTRL2);
	sendData(0xC4);

	// write LUT register
	sendCmd(SSD16xx_WLUTREG);
	sendLUTData();

	spiUnselect(_spi);

#if	SPI_USE_MUTUAL_EXCLUSION
	spiReleaseBus(_spi);
#endif
}

void SSD16xx::stop()
{
#if	SPI_USE_MUTUAL_EXCLUSION
	spiAcquireBus(_spi);
	spiStart(_spi, _spiCfg);
#endif

	spiSelect(_spi);

	// disable sequence, CLK->CP->
	sendCmd(SSD16xx_DUPCTRL2);
	sendData(0x03);

	// enter deep sleep mode
	sendCmd(SSD16xx_DPSLP);
	sendData(0x01);

	spiUnselect(_spi);

	spiStop(_spi);

#if	SPI_USE_MUTUAL_EXCLUSION
	spiReleaseBus(_spi);
#endif
}

void SSD16xx::update()
{
#if	SPI_USE_MUTUAL_EXCLUSION
	spiAcquireBus(_spi);
	spiStart(_spi, _spiCfg);
#endif

	spiSelect(_spi);

	// update display
	sendCmd(SSD16xx_ADPUPDSC);

	spiUnselect(_spi);

#if	SPI_USE_MUTUAL_EXCLUSION
	spiReleaseBus(_spi);
#endif

	// wait until ready
	while (palReadLine(_busyLine) == PAL_HIGH)
		chThdSleepMilliseconds(10);
}

void SSD16xx::setAddress(uint8_t xsa, uint8_t xea, uint16_t ysa, uint16_t yea)
{
	osalDbgAssert((xsa < (sources() >> 2)) && (xea < (sources() >> 2)) &&
			(ysa < gates()) && (yea < gates()),
			"SSD16xx::setAddress(), invalid address");

	// set RAM X-address start/end position
	sendCmd(SSD16xx_RASTXSE);
	sendData(xsa);
	sendData(xea);

	// set RAM Y-address start/end position
	sendCmd(SSD16xx_RASTYSE);
	sendData(ysa);
	if (gates() > 0xFF)
		sendData(ysa >> 8);
	sendData(yea);
	if (gates() > 0xFF)
		sendData(yea >> 8);

	// set RAM X address count
	sendCmd(SSD16xx_RASTXAC);
	sendData(xsa);

	// set RAM Y address count
	sendCmd(SSD16xx_RASTYAC);
	sendData(ysa);
	if (gates() > 0xFF)
		sendData(yea >> 8);

	// data write into RAM after this command
	sendCmd(SSD16xx_RAMWR);
}
