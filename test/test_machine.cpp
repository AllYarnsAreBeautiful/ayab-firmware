/*!`
 * \file test_machine.cpp
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

#include <gtest/gtest.h>

#include <machine.h>

using ::testing::_;
using ::testing::Return;

class MachineTest : public ::testing::Test {
protected:
  void SetUp() override {
    m = new Machine();
  }

  void TearDown() override {
    delete m;
  }

  Machine *m;
};

/*!
 * \test
 */
TEST_F(MachineTest, test_constructor) {
  ASSERT_EQ(m->getMachineType(), NO_MACHINE);
}

/*!
 * \test
 */
TEST_F(MachineTest, test_setMachineType) {
  m->setMachineType(Kh930);
  ASSERT_EQ(m->getMachineType(), Kh930);
}

/*!
 * \test
 */
TEST_F(MachineTest, test_numNeedles) {
  m->setMachineType(Kh270);
  ASSERT_EQ(m->numNeedles(), 114);
  m->setMachineType(Kh930);
  ASSERT_EQ(m->numNeedles(), 200);
}

/*!
 * \test
 */
TEST_F(MachineTest, test_lenLineBuffer) {
  m->setMachineType(Kh270);
  ASSERT_EQ(m->lenLineBuffer(), 15);
  m->setMachineType(Kh910);
  ASSERT_EQ(m->lenLineBuffer(), 25);
}

/*!
 * \test
 */
TEST_F(MachineTest, test_offsetL) {
  ASSERT_EQ(m->endOfLineOffsetL(), 12);
}

/*!
 * \test
 */
TEST_F(MachineTest, test_offsetR) {
  ASSERT_EQ(m->endOfLineOffsetR(), 12);
}
