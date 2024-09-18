from ayab_communication import AyabCommunication
from crc8 import crc8
import logging
import struct
import sys

# Default parameters
SERIAL_PORT = '/dev/ttyACM0'
SERIAL_PORT = '/dev/pts/4'

API_REQUEST_RESET  = 0x00
API_REQUEST_START  = 0x01
API_REQUEST_INIT   = 0x05
API_CONFIRM_INIT   = 0xc5
API_CONFIRM_START  = 0xc1
API_REQUEST_LINE   = 0x82
API_CONFIRM_LINE   = 0x42
API_REQUEST_INFO   = 0x03
API_CONFIRM_INFO   = 0xc3
API_REQUEST_STATE = 0x44
API_INDICATE_STATE = 0x84

API_DEBUG_BASE         = 0xf0
API_DEBUG_REQUEST_PEEK = 0xf8
API_DEBUG_CONFIRM_PEEK = 0xf9
API_DEBUG_REQUEST_POKE = 0xfa

directionMap = ['?', 'Left ', 'Right']
beltShiftMap = ['?', 'Regular', 'Shifted', 'Regular (L)', 'Shifted (L)']
carriageMap = ['None', 'K', 'L', 'G']

# Setup logger
loglevel = logging.INFO
logging.basicConfig(stream=sys.stdout, level=loglevel)

logger = logging.getLogger("Analyzer")

# Open serial interface (triggers an Arduino reset)
#ayab=AyabCommunication(loglevel=logging.DEBUG)
ayab=AyabCommunication(loglevel=logging.INFO)
ayab.open_serial(SERIAL_PORT)

def rxTask():
  try:
    while True:
      msg = ayab.get_msg()
      if msg:
        if msg[0] == API_INDICATE_STATE:
          error = msg[1]
          state = msg[2]
          hallLeft  = struct.unpack("<H", msg[3:5])[0]
          hallRight = struct.unpack("<H", msg[5:7])[0]
          carriage  = carriageMap[msg[7]]
          position  = msg[8]
          direction = directionMap[msg[9]]
          hallActive = directionMap[msg[10]]
          beltShift = beltShiftMap[msg[11]]
          crc = msg[12]
          crcOK = "OK" if crc == crc8(0, msg[0:12]) else "NOK"
          logger.info(f"IndState: {error:1d} {state:1d} {position:3d} {carriage:4s}({beltShift:11s},{direction:5s}) Hall:{hallActive:5s} ({hallLeft:5d}, {hallRight:5d}) [{crc:02x} {crcOK}]")

        elif msg[0] == API_CONFIRM_INIT:
           error = msg[1]
           crc = msg[2]
           crcOK = "OK" if crc == crc8(0, msg[0:2]) else "NOK"
           logger.info(f"Confirm init: {error} [{crc:02x} {crcOK}]")

        elif msg[0] == API_CONFIRM_START:
           error = msg[1]
           crc = msg[2]
           crcOK = "OK" if crc == crc8(0, msg[0:2]) else "NOK"
           logger.info(f"Confirm start: {error} [{crc:02x} {crcOK}]")

        elif msg[0] == API_CONFIRM_INFO:
           api_version  = msg[1]
           fw_maj = msg[2]
           fw_min = msg[3]
           fw_patch = msg[4]
           fw_version = msg[5:21]
           crc = msg[21]
           crcOK = "OK" if crc == crc8(0, msg[0:21]) else "NOK"
           logger.info(f"Confirm Info: API {api_version}, FW {fw_maj}.{fw_min}.{fw_patch} {fw_version}[{crc:02x} {crcOK}]")

        elif msg[0] == API_REQUEST_LINE:
           rowNumber = msg[1]
           error = msg[2]
           crc = msg[3]
           crcOK = "OK" if crc == crc8(0, msg[0:3]) else "NOK"
           logger.info(f"Pattern request for line: {rowNumber} [{crc:02x} {crcOK}]")

        elif msg[0] == API_DEBUG_CONFIRM_PEEK:
           value = msg[1]
           crc = msg[2]
           crcOK = "OK" if crc == crc8(0, msg[0:2]) else "NOK"
           logger.info(f"Peek value: 0x{value:02x} [{crc:02x} {crcOK}]")

        else:
           logger.info(f"UNKNOWN: {msg.hex()}")
                       
  except KeyboardInterrupt:
    pass

def api_requestReset():
   ayab.send_msg(API_REQUEST_RESET, [])

def api_requestInfo():
   ayab.send_msg(API_REQUEST_INFO, [])

def api_requestState():
   ayab.send_msg(API_REQUEST_STATE, [])

def api_requestInit(machine):
   ayab.send_msg(API_REQUEST_INIT, [machine])

def api_requestStart(startNeedle=70, stopNeedle=129, isContinuousReporting=False):
   payload = bytes([startNeedle, stopNeedle, 0x01 if isContinuousReporting else 0x00])
   ayab.send_msg(API_REQUEST_START, payload)

def api_confirmLine(lineNumber, flags= 0, needleList=[83, 99,100, 116]):
  payload = [0x00]*25
  for needle in needleList:
    index = needle >> 3
    offset = needle % 8
    payload[index] = payload[index] | (1 << offset)
  ayab.send_msg(API_CONFIRM_LINE, bytes([lineNumber] + payload + [flags]))

def api_peek(address):
   address_low = address & 0xff
   address_high = (address >> 8) & 0xff
   ayab.send_msg(API_DEBUG_REQUEST_PEEK, bytes([address_low, address_high]));

def api_poke(address, value):
   address_low = address & 0xff
   address_high = (address >> 8) & 0xff
   value = value & 0xff
   ayab.send_msg(API_DEBUG_REQUEST_POKE, bytes([address_low, address_high, value]));

oddPattern = [x for x in range(200) if (x%2) != 0]
evenPattern = [x for x in range(200) if (x%2) == 0]