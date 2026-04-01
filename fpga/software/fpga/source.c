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
static Date pending_time;
static Date pending_alarm;
static int set_time_field = 1;
static int set_alarm_field = 1;
static int button1_prev_pressed_set_time = 0;
static int button2_prev_pressed_set_alarm = 0;

#define BUTTON0_MASK 0x1
#define BUTTON1_MASK 0x2
#define BUTTON2_MASK 0x4

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

static int is_button_pressed(int button_mask)
{
  int button_state = IORD(BUTTON_BASE, 0);

  return (button_state & button_mask) != 0;
}

void Timer_IQR_Handler(void *isr_context)
{
  (void)isr_context;
  datetime_tick(&current_time);
  lcd_show_datetime(&current_time);

  if (mode == SET_TIME)
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
    if (is_button_pressed(BUTTON1_MASK) && mode == RUNNING)
    {
      while (is_button_pressed(BUTTON1_MASK))
        ;

      pending_time = current_time;
      set_time_field = 1;
      button1_prev_pressed_set_time = 0;
      mode = SET_TIME;
      printf("Mode: SET_TIME\n");
    }

    if (is_button_pressed(BUTTON2_MASK) && mode == RUNNING)
    {
      while (is_button_pressed(BUTTON2_MASK))
        ;

      pending_alarm = alarm_time;
      set_alarm_field = 1;
      button2_prev_pressed_set_alarm = 0;
      mode = SET_ALARM;
      printf("Mode: SET_ALARM\n");
    }

    if (is_uart_available())
    {
      uart_receive_string(buffer, sizeof(buffer));
      uart_send_string(buffer);
    }

    if (is_alarm(&current_time, &alarm_time) && alarm_counter == 0)
    {
      alarm_counter = 10;
    }

    if (is_button_pressed(BUTTON0_MASK)) // Turn off alarm when BUTTON0 is pressed
    {
      while (is_button_pressed(BUTTON0_MASK))
        ;

      alarm_counter = 0;
      IOWR(BUZZ_BASE, 0, 0);
      printf("Alarm turned off\n");
    }
  }

  return 0;
}

static int read_switch_7bit()
{
  int value = IORD(SWITCH_BASE, 0) & 0x7F;

  if (value > 99)
    value = 99;

  return value;
}

static void set_date_field_value(Date *pending, int field, int value)
{
  switch (field)
  {
  case 1:
    pending->day = value;
    break;
  case 2:
    pending->month = value;
    break;
  case 3:
    pending->year = 2000 + value;
    break;
  case 4:
    pending->hour = value;
    break;
  case 5:
    pending->minute = value;
    break;
  case 6:
    pending->second = value;
    break;
  default:
    break;
  }
}

static void run_set_mode(Date *pending, Date *target, int *field, int *prev_pressed, int button_pressed, const char *commit_message)
{
  int value = read_switch_7bit();
  int tens = value / 10;
  int units = value % 10;
  int button_rising_edge = button_pressed && !(*prev_pressed);

  if (*field < 1 || *field > 6)
    *field = 1;

  set_date_field_value(pending, *field, value);

  IOWR(HEX2_BASE, 0, HEX_7SEG[*field]);
  IOWR(HEX1_BASE, 0, HEX_7SEG[tens]);
  IOWR(HEX0_BASE, 0, HEX_7SEG[units]);

  if (button_rising_edge)
  {
    if (*field >= 6)
    {
      *target = *pending;
      mode = RUNNING;
      printf("%s\n", commit_message);
    }
    else
    {
      (*field)++;
    }

    *prev_pressed = 1;
  }
  else
  {
    *prev_pressed = button_pressed;
  }
}

void set_time()
{
  int button1_pressed = is_button_pressed(BUTTON1_MASK);

  run_set_mode(&pending_time, &current_time, &set_time_field, &button1_prev_pressed_set_time,
               button1_pressed, "SET_TIME committed");
}

void set_alarm()
{
  int button2_pressed = is_button_pressed(BUTTON2_MASK);

  run_set_mode(&pending_alarm, &alarm_time, &set_alarm_field, &button2_prev_pressed_set_alarm,
               button2_pressed, "SET_ALARM committed");
}