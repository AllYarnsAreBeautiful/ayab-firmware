/*!
 * \file beeper.h
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
 *    Modified Work Copyright 2020 Sturla Lange, Tom Price
 *    http://ayab-knitting.com
 */

#ifndef BEEPER_H_
#define BEEPER_H_

#include <Arduino.h>

constexpr uint8_t BEEP_DELAY = 50U; // ms

constexpr uint8_t BEEP_NUM_READY = 5U;
constexpr uint8_t BEEP_NUM_FINISHEDLINE = 3U;
constexpr uint8_t BEEP_NUM_ENDWORK = 10U;

constexpr uint8_t BEEP_ON_DUTY = 0U;
constexpr uint8_t BEEP_OFF_DUTY = 20U;
constexpr uint8_t BEEP_NO_DUTY = 255U;

class BeeperInterface {
public:
  virtual ~BeeperInterface(){};

  // any methods that need to be mocked should go here
  virtual void ready() = 0;
  virtual void finishedLine() = 0;
  virtual void endWork() = 0;
};

// Container class for the static methods that control the beeper.
// Dependency injection is enabled using a pointer to a global instance of
// either `Beeper` or `BeeperMock`, both of which classes implement the
// pure virtual methods of `BeeperInterface`.

class GlobalBeeper final {
private:
  // singleton class so private constructor is appropriate
  GlobalBeeper() = default;

public:
  // pointer to global instance whose methods are implemented
  static BeeperInterface *m_instance;

  static void ready();
  static void finishedLine();
  static void endWork();
};

/*!
 * \brief Class to actuate a beeper connected to PIEZO_PIN
 */
class Beeper : public BeeperInterface {
public:
  void ready();
  void finishedLine();
  void endWork();

private:
  void beep(uint8_t length);
};

#endif // BEEPER_H_
