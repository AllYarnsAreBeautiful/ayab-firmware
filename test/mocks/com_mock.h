/*!`
 * \file com_mock.h
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

#ifndef COM_MOCK_H_
#define COM_MOCK_H_

#include <gmock/gmock.h>

#include <com.h>

class ComMock : public ComInterface {
public:
  MOCK_METHOD0(init, void());
  MOCK_METHOD0(update, void());
  MOCK_CONST_METHOD2(CRC8, uint8_t(const uint8_t *buffer, size_t len));
  MOCK_CONST_METHOD2(send, void(uint8_t *payload, size_t length));
  MOCK_METHOD2(sendMsg, void(API_t id, const char *msg));
  MOCK_METHOD2(sendMsg, void(API_t id, char *msg));
  MOCK_CONST_METHOD2(send_reqLine, void(const uint8_t lineNumber, Err_t error));
  MOCK_CONST_METHOD1(send_indState, void(Err_t error));
  MOCK_METHOD2(onPacketReceived, void(const uint8_t *buffer, size_t size));

  MOCK_METHOD1(h_reqInit, void(const uint8_t (&buffer)[3]));
  MOCK_METHOD2(h_reqStart, void(const uint8_t *buffer, size_t size));
  MOCK_METHOD2(h_cnfLine, void(const uint8_t *buffer, size_t size));
  MOCK_CONST_METHOD0(h_reqInfo, void());
  MOCK_CONST_METHOD0(h_reqTest, void());
  MOCK_CONST_METHOD0(h_quitCmd, void());
  MOCK_CONST_METHOD0(h_unrecognized, void());
};

ComMock *comMockInstance();
void releaseComMock();

#endif // COM_MOCK_H_
