/* TinyUSB config for a single CDC (USB serial) device on the RP2040. */
#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define BOARD_TUD_RHPORT      0
#define BOARD_TUD_MAX_SPEED   OPT_MODE_FULL_SPEED

#define CFG_TUSB_RHPORT0_MODE (OPT_MODE_DEVICE | BOARD_TUD_MAX_SPEED)

/* pico-sdk provides the TinyUSB OS/board port. */
#define CFG_TUSB_OS           OPT_OS_PICO

#define CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_ALIGN    __attribute__((aligned(4)))

/* Device classes: two CDC ports (AYAB data + debug log). */
#define CFG_TUD_ENABLED        1
#define CFG_TUD_CDC             2

#define CFG_TUD_CDC_RX_BUFSIZE 1024
#define CFG_TUD_CDC_TX_BUFSIZE 256

#ifdef __cplusplus
}
#endif
#endif
