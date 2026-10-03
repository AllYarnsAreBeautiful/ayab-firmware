/* TinyUSB device descriptors: two CDC (USB serial) ports.
 *
 *   interface 0 = AYAB Desktop data (SLIP, 115200)
 *   interface 1 = debug log (only meaningful in a RP2040AYAB_DEBUG build)
 *
 * AYAB Desktop does not filter by VID/PID (it enumerates all serial ports),
 * so these identifiers are only cosmetic.
 */
#include <string.h>
#include "tusb.h"

/* ---- device descriptor -------------------------------------------------- */
tusb_desc_device_t const desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = TUSB_CLASS_MISC,
    .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol    = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = 0xCafe,
    .idProduct          = 0x4002,
    .bcdDevice          = 0x0100,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01
};

uint8_t const *tud_descriptor_device_cb(void) {
    return (uint8_t const *)&desc_device;
}

/* ---- configuration descriptor (two CDC ports) --------------------------- */
enum {
    ITF_NUM_CDC_0 = 0,
    ITF_NUM_CDC_0_DATA,
    ITF_NUM_CDC_1,
    ITF_NUM_CDC_1_DATA,
    ITF_NUM_TOTAL
};

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + 2 * TUD_CDC_DESC_LEN)

#define EPNUM_CDC_0_NOTIF 0x81
#define EPNUM_CDC_0_OUT   0x02
#define EPNUM_CDC_0_IN    0x82
#define EPNUM_CDC_1_NOTIF 0x83
#define EPNUM_CDC_1_OUT   0x04
#define EPNUM_CDC_1_IN    0x84

uint8_t const desc_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_0, 4, EPNUM_CDC_0_NOTIF, 8, EPNUM_CDC_0_OUT, EPNUM_CDC_0_IN, 64),
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_1, 4, EPNUM_CDC_1_NOTIF, 8, EPNUM_CDC_1_OUT, EPNUM_CDC_1_IN, 64),
};

uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;
    return desc_configuration;
}

/* ---- string descriptors ------------------------------------------------- */
static uint16_t _desc_str[32 + 1];

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;
    static const char *strings[] = {
        "AYAB",                       /* index 1: Manufacturer */
        "AYAB KH-970 Bridge",         /* index 2: Product */
        "000002",                     /* index 3: Serial */
        "AYAB Debug Port",            /* index 4: CDC interface string */
    };
    uint8_t chr_count;

    if (index == 0) {
        /* Language ID descriptor: bLength=4, type=STRING, English (0x0409) */
        _desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | 4);
        _desc_str[1] = 0x0409;
        return _desc_str;
    }
    if (index > sizeof(strings) / sizeof(strings[0])) return NULL;

    const char *str = strings[index - 1];
    chr_count = (uint8_t)strlen(str);
    if (chr_count > 31) chr_count = 31;
    for (uint8_t i = 0; i < chr_count; i++) _desc_str[1 + i] = str[i];

    _desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));
    return _desc_str;
}
