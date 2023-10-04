/*!
 * \file analogReadAsyncWrapper.cpp
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
 *    Original Work Copyright 2013 Christian Obersteiner, Andreas Müller
 *    Modified Work Copyright 2020-3 Sturla Lange, Tom Price
 *    http://ayab-knitting.com
 */

#include "analogReadAsyncWrapper.h"

/*!
 * \brief Wrapper for analogReadAsync
 */
void AnalogReadAsyncWrapper::analogReadAsyncWrapped(uint8_t pin, analogReadCompleteCallback_t cb, const void *data) {
#ifndef AYAB_TESTS
  analogReadAsync(pin, cb, data);
#else
  (void) pin;
  (void) cb;
  (void) data;
#endif
}
