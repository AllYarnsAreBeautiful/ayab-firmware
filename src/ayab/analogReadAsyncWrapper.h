/*!
 * \file analogReadAsyncWrapper.h
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

#ifndef ANALOGREADASYNCWRAPPER_H_
#define ANALOGREADASYNCWRAPPER_H_

#include <Arduino.h>
#include <analogReadAsync.h>

class AnalogReadAsyncWrapperInterface {
public:
  virtual ~AnalogReadAsyncWrapperInterface() = default;

  // any methods that need to be mocked should go here
  virtual void analogReadAsyncWrapped(uint8_t pin, analogReadCompleteCallback_t cb, const void *data) = 0;
};

// Container class for the static method analogReadAsync.
// Dependency injection is enabled using a reference to a global instance of
// either `AnalogReadAsyncWrapper` or `AnalogReadAsyncWrapperMock`,
// both of which classes implement the
// pure virtual methods of `AnalogReadAsyncWrapperInterface`.

class GlobalAnalogReadAsyncWrapper final {
private:
  // singleton class so private constructor is appropriate
  GlobalAnalogReadAsyncWrapper() = default;

public:
  // reference to global instance whose methods are implemented
  static AnalogReadAsyncWrapperInterface& m_instance;

  static void analogReadAsyncWrapped(uint8_t pin, analogReadCompleteCallback_t cb = nullptr, const void *data = nullptr);
};

/*!
 * \brief Wrapper for analogReadAsync method
 */
class AnalogReadAsyncWrapper : public AnalogReadAsyncWrapperInterface {
public:
  void analogReadAsyncWrapped(uint8_t pin, analogReadCompleteCallback_t cb = nullptr, const void *data = nullptr) final;
};

#endif // ANALOGREADASYNCWRAPPER_H_
