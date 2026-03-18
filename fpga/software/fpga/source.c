/*
 * source.c
 *
 *  Created on: Mar 18, 2026
 *      Author: tiesen243
 */

#include "sys/alt_irq.h"

#include "system.h"
#include "datetime.h"
#include "lcd.h"
#include "timer_ctrl.h"

void Timer_IQR_Handler(void *isr_context)
{
  (void)isr_context;
  datetime_tick();
  lcd_show_datetime(datetime_get());
  timer_clear_timeout();
}

int main(void)
{
  lcd_init();
  lcd_power_on();
  lcd_show_datetime(datetime_get());

  timer_init_1s();
  alt_ic_isr_register(0, TIMER_IRQ, Timer_IQR_Handler, (void *)0, (void *)0);

  while (1)
    ;

  return 0;
}
