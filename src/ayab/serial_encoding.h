// serial_encoding.h
/*
This file is part of AYAB.

    AYAB is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AYAB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AYAB.  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2013 Christian Obersteiner, Andreas Müller
    http://ayab-knitting.com
*/

#pragma once

void onPacketReceived(const uint8_t *buffer, size_t size);

#define FW_VERSION_MAJ 0
#define FW_VERSION_MIN 95
#define FW_VERSION_PATCH 0

#define API_VERSION 5 // for message description, see below

#define SERIAL_BAUDRATE 115200

typedef enum AYAB_API {
  reqStart_msgid = 0x01,
  cnfStart_msgid = 0xC1,
  reqLine_msgid = 0x82,
  cnfLine_msgid = 0x42,
  reqInfo_msgid = 0x03,
  cnfInfo_msgid = 0xC3,
  reqTest_msgid = 0x04,
  cnfTest_msgid = 0xC4,
  indState_msgid = 0x84,
  debug_msgid = 0xFF
} AYAB_API_t;
