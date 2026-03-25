/*
 * source.c
 *
 *  Created on: Mar 18, 2026
 *      Author: tiesen243
 */

#include "sys/alt_irq.h"
#include <io.h>

#include "system.h"
#include "include/datetime.h"
#include "include/lcd.h"
#include "include/timer_ctrl.h"
#include "include/uart.h"

static Date current = {25, 3, 2026, 18, 0, 0};
static Date alarm = {25, 3, 2026, 18, 0, 10};

void Timer_IQR_Handler(void *isr_context)
{
  (void)isr_context;
  datetime_tick(&current);
  lcd_show_datetime(&current);
  timer_clear_timeout();
}

int main(void)
{
  lcd_init();
  lcd_power_on();
  lcd_show_datetime(&current);

  timer_init_1s();
  alt_ic_isr_register(0, TIMER_IRQ, Timer_IQR_Handler, (void *)0, (void *)0);

  while (1)
  {
    static char buffer[100];
    if (is_uart_available())
    {
      uart_receive_string(buffer, sizeof(buffer));
      uart_send_string(buffer);
    }

    if (is_alarm(&current, &alarm))
    {
      uart_send_string("Alarm!\n");
      IOWR(BUZZ_BASE, 0, 1);
    }
  }

  return 0;
}
