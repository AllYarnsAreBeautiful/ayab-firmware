USER_LIB_PATH=./libraries

ifeq "$(MACHINETYPE)" "KH910"
CPPFLAGS += -DKH910
else
ifeq "$(MACHINETYPE)" "KH930"
CPPFLAGS += -DKH930
else
$(info "Available values for MACHINETYPE: KH910 KH930")
endif
endif

include $(ARDMK_DIR)/Arduino.mk
