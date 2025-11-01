// #pragma once
// #include <stdint.h>
// #include <stdbool.h>

// /* Clocking façade:
//  * - Uses PIT0 for sample-rate Fs callbacks
//  * - Uses PIT1 for bit-rate Rb callbacks
//  */

// typedef void (*clock_cb_t)(void);

// typedef struct {
//     uint32_t bus_hz;   // PIT clock
//     uint32_t fs_hz;    // sample rate
//     uint32_t rb_hz;    // bit rate (baud)
//     uint8_t  prio_fs;  // NVIC priority for Fs (0=highest)
//     uint8_t  prio_rb;  // NVIC priority for Rb
// } clocking_cfg_t;

// // Initialize PIT0/PIT1 with given rates and callbacks (does not start)
// bool clocking_init(const clocking_cfg_t* cfg, clock_cb_t fs_cb, clock_cb_t rb_cb);

// // Start/stop both clocks
// void clocking_start(void);
// void clocking_stop(void);

// // Change rates on the fly
// bool clocking_set_fs(uint32_t fs_hz);
// bool clocking_set_rb(uint32_t rb_hz);
