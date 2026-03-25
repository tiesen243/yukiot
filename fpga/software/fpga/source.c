/*
 * source.c
 *
 *  Created on: Mar 18, 2026
 *      Author: tiesen243
 */

#include "sys/alt_irq.h"
#include <io.h>
#include <unistd.h>

#include "system.h"
#include "include/datetime.h"
#include "include/lcd.h"
#include "include/timer_ctrl.h"
#include "include/uart.h"

static Date current_time = {25, 3, 2026, 18, 0, 0},
            alarm_time = {25, 3, 2026, 18, 0, 5};
static int alarm_counter = 0, buzz_state = 0;
static char buffer[100];

void Timer_IQR_Handler(void *isr_context)
{
  (void)isr_context;
  datetime_tick(&current_time);
  lcd_show_datetime(&current_time);

  if (alarm_counter > 0)
  {
    IOWR(BUZZ_BASE, 0, buzz_state);
    buzz_state = !buzz_state;
    alarm_counter--;
  }

  timer_clear_timeout();
}

int main(void)
{
  lcd_init();
  lcd_power_on();
  lcd_show_datetime(&current_time);
  IOWR(BUZZ_BASE, 0, 0);

  timer_init(1000); // Initialize timer for 1-second intervals
  alt_ic_isr_register(0, TIMER_IRQ, Timer_IQR_Handler, (void *)0, (void *)0);

  while (1)
  {
    if (is_uart_available())
    {
      uart_receive_string(buffer, sizeof(buffer));
      uart_send_string(buffer);
    }

    if (is_alarm(&current_time, &alarm_time) && alarm_counter == 0)
    {
      alarm_counter = 10;
    }

    if (IORD(BUTTON_BASE, 0) == 0) // Turn off alarm when button is pressed
    {
      while (IORD(BUTTON_BASE, 0) == 0)
        ;

      alarm_counter = 0;
      IOWR(BUZZ_BASE, 0, 0);
      printf("Alarm turned off\n");
    }
  }

  return 0;
}
