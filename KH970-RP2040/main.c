/* rp2040ayab entry point.
 *
 * core 0 runs the timing-critical CSI bit-bang; core 1 runs TinyUSB plus the
 * AYAB API v6 protocol translation.  TinyUSB is not thread-safe, so every
 * tud_* call happens on core 1; core 0 only touches the shared rings/state.
 */
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "tusb.h"
#include "csi.h"
#include "ayab.h"

int main(void) {
    /* Boot indicator: blink the on-board LED 3 times. */
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);
    for (int i = 0; i < 3; i++) {
        gpio_put(25, 1); sleep_ms(80);
        gpio_put(25, 0); sleep_ms(80);
    }

    stdio_init_all();

    /* stdio_init_all() does not initialise the TinyUSB device stack when
     * PICO_STDIO_USB is off, so it must be done explicitly. */
    tusb_init();

    /* Launch core 1 for USB + AYAB; core 0 continues into the CSI loop. */
    multicore_launch_core1(ayab_core1_main);

    csi_run();   /* never returns */
    return 0;
}
