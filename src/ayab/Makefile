# Inspired by
# https://github.com/sudar/Arduino-Makefile/blob/master/examples/MakefileExample/Makefile-example.mk
# and
# https://github.com/ladislas/Bare-Arduino-Project/blob/master/Makefile-Linux.mk

PROJECT_DIR = $(shell dirname $(shell pwd))/..
$(info $(PROJECT_DIR))

ARDUINO_LIBS = Wire Adafruit_MCP23008 SoftI2CMaster PacketSerial SerialCommand
USER_LIB_PATH = $(realpath $(PROJECT_DIR)/libraries)

TARGET = ayab

CPPFLAGS += -DAYAB_QUIET

CXXFLAGS += -Wpedantic -Wextra -Wshadow
# Ignore certain warnings
# TODO(sl): Mask these in the NO_WARN_FILES target variable below.
CXXFLAGS += -Wno-vla
LDFLAGS += -fdiagnostics-color

# Warnings are errors, enabled on a clean build.
CXXFLAGS += -Werror

CURRENT_DIR       = $(shell basename $(CURDIR))
OBJDIR            = $(PROJECT_DIR)/build/raw/$(BOARD_TAG)/$(MACHINETYPE)

# Disable all warnings that do not originate from code maintained by this project.
NO_WARN_FILES=$(OBJDIR)/core/%.o $(OBJDIR)/userlibs/%/%.o
$(NO_WARN_FILES): CXXFLAGS += -w

include $(ARDMK_DIR)/Arduino.mk
