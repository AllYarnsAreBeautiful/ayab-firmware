# Inspired by
# https://github.com/sudar/Arduino-Makefile/blob/master/examples/MakefileExample/Makefile-example.mk
# and
# https://github.com/ladislas/Bare-Arduino-Project/blob/master/Makefile-Linux.mk

PROJECT_DIR = $(shell dirname $(shell pwd))/..
$(info $(PROJECT_DIR))

USER_LIB_PATH = $(realpath $(PROJECT_DIR)/libraries)

TARGET = ayab
ifeq "$(MACHINETYPE)" "KH910"
CPPFLAGS += -DKH910
else
ifeq "$(MACHINETYPE)" "KH930"
CPPFLAGS += -DKH930
else
$(info "Available values for MACHINETYPE: KH910 KH930")
endif
endif

CXXFLAGS += -pedantic -Wall -Wextra
LDFLAGS += -fdiagnostics-color

CURRENT_DIR       = $(shell basename $(CURDIR))
OBJDIR            = $(PROJECT_DIR)/build/raw/$(BOARD_TAG)/$(MACHINETYPE)

include $(ARDMK_DIR)/Arduino.mk
