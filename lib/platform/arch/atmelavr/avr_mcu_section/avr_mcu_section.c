#include <Arduino.h>
#include "avr_mcu_section.h"

extern void *__brkval;

AVR_MCU (F_CPU, "atmega328p" );
AVR_MCU_VOLTAGES(3300, 3300, 3300);
AVR_MCU_VCD_FILE("ayab.vcd", 10000000);

const struct avr_mmcu_vcd_trace_t _mytrace[]  _MMCU_ = {
  { AVR_MCU_VCD_SYMBOL("v1"), .what = (void*) &PIND, .mask=(1<<PIND2), },
  { AVR_MCU_VCD_SYMBOL("v2"), .what = (void*) &PIND, .mask=(1<<PIND3), },
  { AVR_MCU_VCD_SYMBOL("BP"), .what = (void*) &PIND, .mask=(1<<PIND4), },
//  { AVR_MCU_VCD_SRAM_16("Stack"), .what = (void*) &SP, },
//  { AVR_MCU_VCD_SRAM_16("Heap"), .what = (void*)(&__brkval), },
};
