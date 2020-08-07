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

#ifndef EINK_CLICK_SSD16XX_HPP_
#define EINK_CLICK_SSD16XX_HPP_

#include "hal.h"

/** @brief	Base SSD16xx driver for EPD displays. */
class SSD16xx {
protected:
	/**
	 * @name	SSD16xx register addresses
	 * @{
	 */
	typedef enum : uint8_t {
		SSD16xx_DOCTRL		= 0x01, // Driver output control
		SSD16xx_GDVCTR		= 0x03,	// Gate Driving voltage Control
		SSD16xx_SDVCTRL		= 0x04, // Source Driving voltage Control
		SSD16xx_DPCTRL		= 0x07, // Display Control ( select bit depth 1bit or 2 bit )
		SSD16xx_GSOVPC		= 0x0B, // Gate and Source non overlap period Control
		SSD16xx_GSSTPOS		= 0x0F, // Gate scan start position
		SSD16xx_DPSLP		= 0x10, // deep sleep control
		SSD16xx_DEMDS		= 0x11, // Data Entry mode setting
		SSD16xx_SWRESET		= 0x12,
		SSD16xx_WRTEMPSC	= 0x1A, // Write to temperature register
		SSD16xx_RRTEMPSC	= 0x1B, // Read from temperature register
		SSD16xx_WCTEMPSC	= 0x1C, // Write Command to temperature sensor
		SSD16xx_RCTEMPSC	= 0x1D, // Load temperature register with temperature sensor reading
		SSD16xx_ADPUPDSC	= 0x20, // Master Activation; Activate Display Update Sequence
		SSD16xx_DUPCTRL1	= 0x21, // Display Update Control 1
		SSD16xx_DUPCTRL2	= 0x22, // Display Update Sequence; Control 2
		SSD16xx_RAMWR		= 0x24, // Write RAM
		SSD16xx_RAMRD		= 0x25, // Read RAM
		SSD16xx_VCOMS		= 0x28, // VCOM Sense
		SSD16xx_VCOMSD		= 0x29, // VCOM Sense duration
		SSD16xx_PVCOMOTP	= 0x2A, // Program VCOM OTP
		SSD16xx_WVCOMREG	= 0x2C, // Write VCOM register
		SSD16xx_ROTPREG		= 0x2D, // Read OTP registers
		SSD16xx_PWSOTP		= 0x30, // Program WS OTP
		SSD16xx_WLUTREG		= 0x32, // Write LUT register
		SSD16xx_RLUTREG		= 0x33, // Read LUT register
		SSD16xx_POTPSEL		= 0x36, // Program OTP selection
		SSD16xx_OTPSELC		= 0x37, // OTP selection control
		SSD16xx_DLPSET		= 0x3A, // Set dummy line period
		SSD16xx_GLWSET		= 0x3B, // Set Gate line width
		SSD16xx_VBDSET		= 0x3C, // Select border waveform for VBD
		SSD16xx_RASTXSE		= 0x44, // Set RAM X - address Start / End position
		SSD16xx_RASTYSE		= 0x45, // Set RAM Y - address Start / End position
		SSD16xx_RASTXAC		= 0x4E, // Set RAM X address counter
		SSD16xx_RASTYAC		= 0x4F, // Set RAM Y address counter
		SSD16xx_BFS			= 0xF0, // Booster Feedback Selection
		SSD16xx_NOP			= 0xFF, // NOP
	} Command;
	/** @} */

	SPIDriver* _spi;			///< Pointer to click @p SPIDriver SPI driver.
	const SPIConfig* _spiCfg;	///< Pointer to click @p SPIConfig SPI configuration.
	ioline_t _rstLine;			///< Click reset line.
	ioline_t _busyLine;			///< Click busy line.
	ioline_t _dcLine;			///< Click data/command line.

	/**
	 * @brief	Send command.
	 * @details	Sets the register address followed by optional data.
	 * 			Takes care of switching to command mode and back to
	 * 			data mode.
	 *
	 * @param[in] c	command
	 */
	void sendCmd(Command c);

	/**
	 * @brief	Send LUT data.
	 * @details	LUT data represent the waveforms needed for changing GS colors.
	 * @note	This pure virtual member has to implemented for all derived
	 * 			drivers cause its driver dependent.
	 */
	virtual void sendLUTData() = 0;

public:
	SSD16xx(SPIDriver& spi, const SPIConfig& spiCfg, ioline_t rstLine, ioline_t busyLine, ioline_t dcLine);
	virtual ~SSD16xx();

	/**
	 * @brief	Get number of sources.
	 * @details	Sources represent the x RAM address axis and are grouped
	 * 			by 4 sources per RAM byte (2 bits per source to build 4
	 * 			level GS).
	 * @note	This pure virtual member has to implemented for all derived
	 * 			drivers cause its driver dependent.
	 *
	 * @returns	The number of sources.
	 */
	virtual uint16_t sources() const = 0;

	/**
	 * @brief	Get number of gates.
	 * @details	Gates represent the y RAM address axis.
	 * @note	This pure virtual member has to implemented for all derived
	 * 			drivers cause its driver dependent.
	 *
	 * @returns	The number of gates.
	 */
	virtual uint16_t gates() const = 0;

	/**
	 * @brief	Select the SPI chip.
	 * @note	When SPI_USE_MUTUAL_EXCLUSION is enabled also acquire SPI
	 * 			bus and starts the driver.
	 */
	void select();

	/**
	 * @brief	Unselect the SPI chip.
	 * @note	When SPI_USE_MUTUAL_EXCLUSION is enabled also release SPI
	 * 			bus.
	 */
	void unselect();

	/**
	 * @brief	Start the SPI driver and send initialize sequence.
	 */
	void start();

	/**
	 * @brief	Stop the SPI driver, clock and put the device to sleep.
	 */
	void stop();

	/**
	 * @brief	Send the update display command and wait until device
	 *			is ready.
	 * @note	No need to select/unselect chip cause it is already done
	 * 			before waiting for the busy line to go low to release
	 * 			the SPI bus for other applications.
	 */
	void update();

	/**
	 * @brief	Set the RAM start and end address.
	 * @details After this command RAM data need to be send.
	 * @note	Need to call select() before execution.
	 * @note	The RAM x and y axis do not necessary correspond to display
	 * 			axis.
	 *
	 * @param[in] xsa		RAM x start address
	 * @param[in] xea		RAM x end address
	 * @param[in] ysa		RAM y start address
	 * @param[in] yea		RAM y end address
	 */
	void setAddress(uint8_t xsa, uint8_t xea, uint16_t ysa, uint16_t yea);

	/**
	 * @brief	Send data / RAM data.
	 * @note	Need to call unselect() after all RAM data are sent.
	 *
	 * @param[in] b		data byte
	 */
	void sendData(uint8_t b);

	/**
	 * @brief	Send data / RAM data.
	 * @note	Need to call unselect() after all RAM data are sent.
	 *
	 * @param[in] bp	pointer to the data buffer
	 * @param[in] n		number of bytes to send
	 */
	void sendData(const uint8_t* bp, size_t n);
};

#endif /* EINK_CLICK_SSD16XX_HPP_ */
