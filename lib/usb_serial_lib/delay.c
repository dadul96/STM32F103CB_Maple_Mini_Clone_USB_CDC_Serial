#include "delay.h"

void delay_ms(uint32_t milliseconds)
{
	static uint8_t init_done_flag = 0;
	static uint32_t reload_val = 0;
	static uint32_t ticks_per_ms = 0;
	uint32_t num_of_reloads = 0;

	if (init_done_flag == 0)
	{
		/* set SYSCLK to 72MHz using HSE 8MHz oscillator: */
		rcc_clock_setup_in_hse_8mhz_out_72mhz();

		/* check csr clocksource and set ticks_per_ms accordingly: */
		if ((STK_CSR & STK_CSR_CLKSOURCE) == STK_CSR_CLKSOURCE_AHB)
		{
			ticks_per_ms = 72000U; /* 72MHz = 72M ticks/s -> divide by 1000 to get ticks/ms */
		}
		else /* == STK_CSR_CLKSOURCE_AHB_DIV8 */
		{
			ticks_per_ms = 72000U / 8U;
		}

		/* get reload value or set own one if 0: */
		reload_val = systick_get_reload();
		if (reload_val == 0)
		{
			systick_set_reload(ticks_per_ms);
			reload_val = ticks_per_ms;
		}

		/* enable systick and set init_done_flag: */
		systick_counter_enable();
		init_done_flag = 1;
	}

	while ((num_of_reloads * reload_val) < (milliseconds * ticks_per_ms))
	{
		if (systick_get_countflag()) /* flag gets automatically cleared when read */
		{
			num_of_reloads++;
		}
	}
}
