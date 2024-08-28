import logging
import serial
import sliplib

class AyabCommunication(object):
  """Class Handling the serial communication protocol."""

  def __init__(self, serial=None, loglevel=logging.INFO):
    """Creates an AyabCommunication object, with an optional serial-like object."""
    self.__logger = logging.getLogger(type(self).__name__)
    self.__logger.setLevel(loglevel)
    self.__ser = serial
    self.__driver = sliplib.Driver()
    self.__rxMsgList = list()

  def __del__(self):
    """Handles on delete behaviour closing serial port object."""
    self.close_serial()

  def open_serial(self, pPortname=None):
    """Opens serial port communication with a portName."""
    if not self.__ser:
      self.__portname = pPortname
      try:
          self.__ser = serial.Serial(self.__portname, 115200, timeout=0.01)
      except:
        self.__logger.error("could not open serial port " + self.__portname)
        raise CommunicationException()
      return True

  def close_serial(self):
    """Closes serial port."""
    if self.__ser is not None and self.__ser.isOpen() is True:
        try:
            self.__ser.close()
            del(self.__ser)
            self.__ser = None
            self.__logger.info("Closing Serial port successful.")
        except:
            self.__logger.warning("Closing Serial port failed. Was it ever open?")

  def get_msg(self):
    """Reads data from serial and tries to parse as SLIP packet."""
    if self.__ser:    
      data = self.__ser.read(1000)
      if len(data) > 0:
        self.__rxMsgList.extend(self.__driver.receive(data))

      if len(self.__rxMsgList) > 0:
        self.__logger.debug(f"Rx msg:{self.__rxMsgList[0].hex(' ')}")
        return self.__rxMsgList.pop(0)
    
    return None

  def send_msg(self, id, payload):
      data = bytearray()
      data.append(id)
      data.extend(payload)
      self.__logger.debug(f"Tx msg:{data.hex(' ')}")
      data = self.__driver.send(bytes(data))
      self.__ser.write(data)

class CommunicationException(Exception):
  pass


