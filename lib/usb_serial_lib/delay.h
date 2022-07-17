#if !defined(__DELAY_H__)
#define __DELAY_H__

#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/rcc.h>

void delay_ms(uint32_t milliseconds);

#endif /* __DELAY_H__ */
