// beeper.h
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

#ifndef BEEPER_H
#define BEEPER_H

#include "Arduino.h"
#include "settings.h"

/*!
 *  Class to actuate a beeper connected to PIEZO_PIN
 */
class Beeper{
public:
	Beeper();

    /*! Beep to indicate readiness */
	void ready();
    /*! Beep to indicate the end of a line */
	void finishedLine();
    /*! Beep to indicate the end the knitting pattern */
	void endWork();

private:
	void beep(byte length);
};

#endif