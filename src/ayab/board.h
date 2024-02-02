/*!
 * \file board.h
 *
 * This file is part of AYAB.
 *
 *    AYAB is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    AYAB is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with AYAB.  If not, see <http://www.gnu.org/licenses/>.
 *
 *    Original Work Copyright 2013-2015 Christian Obersteiner, Andreas Müller
 *    Modified Work Copyright 2020-3 Sturla Lange, Tom Price
 *    http://ayab-knitting.com
 */

#ifndef BOARD_H_
#define BOARD_H_

#include <Arduino.h>

#ifdef DBG_NOMACHINE               // Turn on to use DBG_BTN_PIN as EOL Trigger
constexpr uint8_t DBG_BTN_PIN = 7; // DEBUG BUTTON
#endif

constexpr uint8_t I2Caddr_sol1_8 = 0x0U;  ///< I2C Address of solenoids 1 - 8
constexpr uint8_t I2Caddr_sol9_16 = 0x1U; ///< I2C Address of solenoids 9 - 16

// TODO(Who?): Optimize Delay for various Arduino Models
constexpr uint16_t START_KNITTING_DELAY = 2000U; // ms

// Determine board type
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
// Arduino Uno
// Pin Assignments
constexpr uint8_t EOL_PIN_R = A0; // Analog
constexpr uint8_t EOL_PIN_L = A1; // Analog

constexpr uint8_t ENC_PIN_A = 2;
constexpr uint8_t ENC_PIN_B = 3;

constexpr uint8_t ENC_PIN_C = 4;

constexpr uint8_t LED_PIN_A = 5; // green LED
constexpr uint8_t LED_PIN_B = 6; // yellow LED

constexpr uint8_t PIEZO_PIN = 9;
#elif defined(CONFIG_IDF_TARGET_ESP32S3)// ESP32-S3 (AYAB-ESP32)

// Knitting machine control
constexpr uint8_t EOL_PIN_R_N = 3;  // Right EOL marker, each polarity.
constexpr uint8_t EOL_PIN_R_S = 4;

constexpr uint8_t EOL_PIN_L_N = 1;  // Left EOL marker, each polarity.
constexpr uint8_t EOL_PIN_L_S = 2; 

constexpr uint8_t ENC_PIN_A = 5;    // Carriage movement A
constexpr uint8_t ENC_PIN_B = 6;    // Carriage movement B
constexpr uint8_t ENC_PIN_C = 7;    // Carriage belt phase alignment.

constexpr uint8_t MCP_SDA_PIN = 8;  // Internal I2C bus with MCP GPIO expander.
constexpr uint8_t MCP_SCL_PIN = 9;  // I2C0

// Communication buses
constexpr uint8_t SPI_PIN_COPI = 12; // Internal SPI bus for future display.
constexpr uint8_t SPI_PIN_CIPO = 11; // SPI0
constexpr uint8_t SPI_PIN_SCK = 13;
constexpr uint8_t SPI_PIN_CS = 10;

constexpr uint8_t UART_PIN_TX = 43;  // External bus for debugging and/or user.
constexpr uint8_t UART_PIN_RX = 44;  // UART0

constexpr uint8_t I2C_PIN_SDA = 15;  // External bus for user applications.
constexpr uint8_t I2C_PIN_SCL = 16;  // I2C1

// Misc I/O
#define LED_PIN_A LED_PIN_G
#define LED_PIN_B LED_PIN_R

constexpr uint8_t LED_PIN_R = 33;  
constexpr uint8_t LED_PIN_G = 34;   
constexpr uint8_t LED_PIN_B = 35;    

constexpr uint8_t PIEZO_PIN = 38;

// User I/O
constexpr uint8_t USER_BUTTON = 36;

constexpr uint8_t USER_PIN_14 = 14; // Should these actually be like USER_0... etc?
constexpr uint8_t USER_PIN_17 = 17; // And then on the silk/ enclosure we put friendly numbers (0..8) instead of GPIO name?
constexpr uint8_t USER_PIN_18 = 18;
constexpr uint8_t USER_PIN_21 = 21;

constexpr uint8_t USER_PIN_39 = 39;
constexpr uint8_t USER_PIN_40 = 40;
constexpr uint8_t USER_PIN_41 = 41;
constexpr uint8_t USER_PIN_42 = 42;

#else
#error "untested board"
#endif

#endif // BOARD_H_
