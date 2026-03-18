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
#include "uart.h"

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
  {
    static char buffer[100];
    if (is_uart_available())
    {
      uart_receive_string(buffer, sizeof(buffer));
      uart_send_string(buffer);

      if (buffer[0] == 'S' && strlen(buffer) == 16)
      {
        Date newDate;
        newDate.day = (buffer[1] - '0') * 10 + (buffer[2] - '0');
        newDate.month = (buffer[3] - '0') * 10 + (buffer[4] - '0');
        newDate.year = (buffer[5] - '0') * 1000 + (buffer[6] - '0') * 100 + (buffer[7] - '0') * 10 + (buffer[8] - '0');
        newDate.hour = (buffer[9] - '0') * 10 + (buffer[10] - '0');
        newDate.minute = (buffer[11] - '0') * 10 + (buffer[12] - '0');
        newDate.second = (buffer[13] - '0') * 10 + (buffer[14] - '0');
        datetime_set(&newDate);
      }
    }
  }

  return 0;
}
