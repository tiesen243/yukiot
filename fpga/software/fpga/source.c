/*
 * source.c
 *
 *  Created on: Mar 18, 2026
 *      Author: tiesen243
 */

#include "sys/alt_irq.h"
#include <stdio.h>
#include <io.h>

#include <system.h>
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
// static char buffer[100];
int count = 0, count_2 = 0;

const int HEX_7SEG[16] = {
    0x40, // 0
    0x79, // 1
    0x24, // 2
    0x30, // 3
    0x19, // 4
    0x12, // 5
    0x02, // 6
    0x78, // 7
    0x00, // 8
    0x10, // 9
    0x08, // A
    0x03, // b
    0x46, // C
    0x21, // d
    0x06, // E
    0x0E  // F
};

void set_time();

void set_alarm();

void Timer_IQR_Handler(void *isr_context)
{
  (void)isr_context;

  count++;
  if (count >= 100)
  {
    count = 0;
    datetime_tick(&current_time);
  }

  if (mode == RUNNING)
    lcd_show_datetime(&current_time);
  else if (mode == SET_TIME)
    set_time();
  else if (mode == SET_ALARM)
    set_alarm();

  if (alarm_counter > 0)
  {
    count_2++;
    if (count_2 >= 10)
    {
      count_2 = 0;
      IOWR(BUZZ_BASE, 0, buzz_state);
      buzz_state = !buzz_state;
      alarm_counter--;
    }
  }
  else
    IOWR(BUZZ_BASE, 0, 0);

  timer_clear_timeout();
}

int main(void)
{
  lcd_init();
  lcd_power_on();
  lcd_show_datetime(&current_time);
  IOWR(BUZZ_BASE, 0, 0);

  timer_init(10);
  alt_ic_isr_register(0, TIMER_IRQ, Timer_IQR_Handler, (void *)0, (void *)0);

  printf("Started!");

  while (1)
  {

    if (IORD(BUTTON_BASE, 0) == 6)
    {
      while (IORD(BUTTON_BASE, 0) == 6)
        ;

      mode = RUNNING;
      printf("Tat bao thuc\n");
    }
    else if (IORD(BUTTON_BASE, 0) == 5 && mode == RUNNING)
    {
      while (IORD(BUTTON_BASE, 0) == 5)
        ;

      mode = SET_TIME;
      printf("Mode: SET_TIME\n");
    }
    else if (IORD(BUTTON_BASE, 0) == 3 && mode == RUNNING)
    {
      while (IORD(BUTTON_BASE, 0) == 3)
        ;

      mode = SET_ALARM;
      printf("Mode: SET_ALARM\n");
    }

    if (is_alarm(&current_time, &alarm_time))
      alarm_counter = 1000;
  }

  return 0;
}

int state = 0;

void set_time()
{
  int switch_data = IORD(SWITCH_BASE, 0);

  if (switch_data == 0)
    return;

  if (state == 0) // check year
    switch_data = switch_data + 2000;
  else if (state == 1) // check month
    switch_data = switch_data % 13;
  else if (state == 2) // check day
  {
    int is_leap_year = ((current_time.year % 4 == 0 && current_time.year % 100 != 0) || (current_time.year % 400 == 0));
    int is_month_31_days = (current_time.month == 1 || current_time.month == 3 || current_time.month == 5 || current_time.month == 7 || current_time.month == 8 || current_time.month == 10 || current_time.month == 12);

    if (is_month_31_days)
      switch_data = switch_data % 32;
    else if (current_time.month == 2)
      switch_data = switch_data % (is_leap_year ? 30 : 29);
    else
      switch_data = switch_data % 31;
  }
  else if (state == 3)
    switch_data = switch_data % 24;
  else if (state == 4)
    switch_data = switch_data % 60;
  else if (state == 5)
    switch_data = switch_data % 60;

  IOWR(HEX2_BASE, 0, 0xFF);
  IOWR(HEX1_BASE, 0, HEX_7SEG[(switch_data / 10) % 10]);
  IOWR(HEX0_BASE, 0, HEX_7SEG[switch_data % 10]);

  if (IORD(BUTTON_BASE, 0) == 5)
  {
    while (IORD(BUTTON_BASE, 0) == 5)
      ;

    switch (state)
    {
    case 0:
    {
      printf("Set year: %d\n", (switch_data));
      current_time.year = (switch_data);
      break;
    }
    case 1:
    {
      printf("Set month: %d\n", switch_data);
      current_time.month = switch_data;
      break;
    }
    case 2:
    {
      printf("Set day: %d\n", switch_data);
      current_time.day = switch_data;
      break;
    }
    case 3:
    {
      printf("Set hour: %d\n", switch_data);
      current_time.hour = switch_data;
      break;
    }
    case 4:
    {
      printf("Set minute: %d\n", switch_data);
      current_time.minute = switch_data;
      break;
    }
    case 5:
    {
      printf("Set second: %d\n", switch_data);
      current_time.second = switch_data;
      break;
    }
    case 6:
    {
      printf("Time set: %02d/%02d/%04d %02d:%02d:%02d\n", current_time.day, current_time.month, current_time.year, current_time.hour, current_time.minute, current_time.second);
      mode = RUNNING;
      state = -1;

      IOWR(HEX2_BASE, 0, 0xFF);
      IOWR(HEX1_BASE, 0, 0xFF);
      IOWR(HEX0_BASE, 0, 0xFF);
      break;
    }
    }

    lcd_show_datetime(&current_time);
    state += 1;
  }
}

void set_alarm()
{
  int switch_data = IORD(SWITCH_BASE, 0);

  if (switch_data == 0)
    return;

  if (state == 0) // check year
    switch_data = switch_data + 2000;
  else if (state == 1) // check month
    switch_data = switch_data % 13;
  else if (state == 2) // check day
  {
    int is_leap_year = ((current_time.year % 4 == 0 && current_time.year % 100 != 0) || (current_time.year % 400 == 0));
    int is_month_31_days = (current_time.month == 1 || current_time.month == 3 || current_time.month == 5 || current_time.month == 7 || current_time.month == 8 || current_time.month == 10 || current_time.month == 12);

    if (is_month_31_days)
      switch_data = switch_data % 32;
    else if (current_time.month == 2)
      switch_data = switch_data % (is_leap_year ? 30 : 29);
    else
      switch_data = switch_data % 31;
  }
  else if (state == 3)
    switch_data = switch_data % 24;
  else if (state == 4)
    switch_data = switch_data % 60;
  else if (state == 5)
    switch_data = switch_data % 60;

  IOWR(HEX2_BASE, 0, 0xFF);
  IOWR(HEX1_BASE, 0, HEX_7SEG[(switch_data / 10) % 10]);
  IOWR(HEX0_BASE, 0, HEX_7SEG[switch_data % 10]);

  if (IORD(BUTTON_BASE, 0) == 5)
  {
    while (IORD(BUTTON_BASE, 0) == 5)
      ;

    switch (state)
    {
    case 0:
    {
      printf("Set year: %d\n", (switch_data));
      alarm_time.year = (switch_data);
      break;
    }
    case 1:
    {
      printf("Set month: %d\n", switch_data);
      alarm_time.month = switch_data;
      break;
    }
    case 2:
    {
      printf("Set day: %d\n", switch_data);
      alarm_time.day = switch_data;
      break;
    }
    case 3:
    {
      printf("Set hour: %d\n", switch_data);
      alarm_time.hour = switch_data;
      break;
    }
    case 4:
    {
      printf("Set minute: %d\n", switch_data);
      alarm_time.minute = switch_data;
      break;
    }
    case 5:
    {
      printf("Set second: %d\n", switch_data);
      alarm_time.second = switch_data;
      break;
    }
    case 6:
    {
      printf("Alarm set: %02d/%02d/%04d %02d:%02d:%02d\n", alarm_time.day, alarm_time.month, alarm_time.year, alarm_time.hour, alarm_time.minute, alarm_time.second);
      mode = RUNNING;
      state = -1;

      IOWR(HEX2_BASE, 0, 0xFF);
      IOWR(HEX1_BASE, 0, 0xFF);
      IOWR(HEX0_BASE, 0, 0xFF);
      break;
    }
    }

    lcd_show_datetime(&alarm_time);
    state += 1;
  }
}
