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

enum MODE
{
  RUNNING,
  SET_TIME,
  SET_ALARM
};

static Date current_time = {25, 3, 2026, 18, 0, 0},
            alarm_time = {25, 3, 2026, 18, 0, 5};
static enum MODE mode = RUNNING;
static int alarm_counter = 0, buzz_state = 0;
static char buffer[100];

void set_time();

void set_alarm();

void Timer_IQR_Handler(void *isr_context)
{
  (void)isr_context;
  datetime_tick(&current_time);

  if (mode == RUNNING)
    lcd_show_datetime(&current_time);
  else if (mode == SET_TIME)
    set_time();
  else if (mode == SET_ALARM)
    set_alarm();

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

    if (IORD(SWITCH_BASE, 0) == 0 && mode != RUNNING)
    {
      mode = RUNNING;
      printf("Mode: RUNNING\n");
    }
    else if (IORD(SWITCH_BASE, 0) == 1 && mode != SET_TIME)
    {
      mode = SET_TIME;
      printf("Mode: SET_TIME\n");
    }
    else if (IORD(SWITCH_BASE, 0) == 2 && mode != SET_ALARM)
    {
      mode = SET_ALARM;
      printf("Mode: SET_ALARM\n");
    }
  }

  return 0;
}

void set_time()
{
  // read time from SWITCH_BASE (exclude the last 2 bits for mode selection), read 7 bits (0-99), then
}