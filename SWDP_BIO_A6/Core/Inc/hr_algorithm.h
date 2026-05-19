#ifndef HR_ALGORITHM_H
#define HR_ALGORITHM_H

#include <stdint.h>
#include "arm_math.h"

/*
 * Buffer sizes.
 *
 * The PPG FIFO interrupt fires when 17 samples are available (FIFO_A_FULL = 15 =>
 * 32 - 15 = 17 unread samples). The acquisition buffer is the next power of 2
 * that fits at least 17 samples, i.e. 32.
 *
 * The circular (overlap) buffer is twice as large (64) and always holds the
 * previous 32 samples in the first half and the freshly acquired 32 samples in
 * the second half, implementing a 50 % sliding window.
 */
#define PPG_ACQ_SIZE   128u  /* acquisition buffer  – power of 2; fills in ~1.28 s (7-8 interrupts of 17 samples) */
#define PPG_CIRC_SIZE  256u  /* circular buffer     – 2 × PPG_ACQ_SIZE = 2.56 s; covers 2+ beats at 60 BPM      */

/*
 * PPG sample rate configured in ppg_driver.c:
 *   SPO2_CONFIG = 0x27  =>  100 sps, 18-bit ADC, 411 µs pulse width.
 */
#define PPG_FS         100.0f

void      HR_Algorithm_Init(void);
float32_t HR_Process(const uint32_t *circ_buf);
float32_t HR_GetBPM(void);

#endif /* HR_ALGORITHM_H */
