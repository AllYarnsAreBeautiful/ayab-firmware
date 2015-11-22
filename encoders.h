// encoders.h
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
    https://bitbucket.org/chris007de/ayab-apparat/
*/

#ifndef ENCODERS_H
#define ENCODERS_H

#include "Arduino.h"
#include "settings.h"

class Encoders{
public:
	Encoders();

  void encA_interrupt();

	byte 			getPosition();
	Beltshift_t 	getBeltshift();
	Direction_t 	getDirection();
  Direction_t   getHallActive();
  Carriage_t    getCarriage();

  uint16 getHallValue(Direction_t);

private:
	Direction_t 	m_direction;
  Direction_t   m_hallActive;
	Beltshift_t 	m_beltShift;
  Carriage_t    m_carriage;
	byte   m_encoderPos;

  void encA_rising();
  void encA_falling();
};

#endif