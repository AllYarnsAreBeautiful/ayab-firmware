//debug.h
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

#ifndef DEBUG_H
#define DEBUG_H


//#define DEBUG

/*
 *
 */
#ifdef DEBUG
	#define DEBUG_PRINT(str) \
	    Serial.print("#"); \
	    Serial.print(millis()); \
	    Serial.print(": "); \
	    Serial.print(__FUNCTION__); \
	    Serial.print("() in "); \
	    Serial.print(__FILE__); \
	    Serial.print(':'); \
	    Serial.print(__LINE__); \
	    Serial.print(' '); \
	    Serial.println(str);
#else
	#define DEBUG_PRINT(str)
#endif

#endif