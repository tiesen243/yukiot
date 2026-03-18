/*
 * source.c
 *
 *  Created on: Mar 18, 2026
 *      Author: tiesen243
 */

#include <stdio.h>
#include "altera_avalon_pio_regs.h"
#include "altera_avalon_timer_regs.h"
#include "sys/alt_irq.h"

#include "system.h"

char DATA[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

struct Date
{
  int day;
  int month;
  int year;

  int hour;
  int minute;
  int second;
};

static struct Date date = {18, 3, 2026, 18, 14, 0};
static struct Date lastDate = {0, 0, 0, 0, 0, 0};
static int hasLastDate = 0;

void delay(int a)
{
  volatile int i = 0;
  // while (i < a * 10000)
  while (i < a * 1000)
    i++;
}

void command(int data)
{
  IOWR_ALTERA_AVALON_PIO_DATA(LCD_RS_BASE, 0x00);
  IOWR_ALTERA_AVALON_PIO_DATA(LCD_RW_BASE, 0x00);
  IOWR_ALTERA_AVALON_PIO_DATA(LCD_D_BASE, data & 0xFF);
  IOWR_ALTERA_AVALON_PIO_DATA(LCD_EN_BASE, 0x01);
  delay(20);

  IOWR_ALTERA_AVALON_PIO_DATA(LCD_EN_BASE, 0x00);
  delay(20);
}

void lcd_data(char data)
{
  IOWR_ALTERA_AVALON_PIO_DATA(LCD_RS_BASE, 0x01);
  IOWR_ALTERA_AVALON_PIO_DATA(LCD_RW_BASE, 0x00);
  IOWR_ALTERA_AVALON_PIO_DATA(LCD_D_BASE, data & 0xFF);
  IOWR_ALTERA_AVALON_PIO_DATA(LCD_EN_BASE, 0x01);
  delay(20);

  IOWR_ALTERA_AVALON_PIO_DATA(LCD_EN_BASE, 0x00);
  delay(20);
}

void lcd_string(char *str)
{
  int i = 0;
  while (str[i] != 0)
  {
    lcd_data(str[i]);
    i++;
  }
}

void lcd_init()
{
  command(0x38);
  delay(100);
  command(0x0c);
  delay(100);
  command(0x06);
  delay(100);
  command(0x01);
}

void lcd_set_cursor(int row, int col)
{
  int address = (row == 0 ? 0x00 : 0x40) + col;
  command(0x80 | address);
}

void lcd_write_2digits(int value)
{
  lcd_data(((value / 10) % 10) + '0');
  lcd_data((value % 10) + '0');
}

void lcd_write_4digits(int value)
{
  lcd_data((value / 1000) + '0');
  lcd_data(((value / 100) % 10) + '0');
  lcd_data(((value / 10) % 10) + '0');
  lcd_data((value % 10) + '0');
}

void increaseTime()
{
  date.second++;

  if (date.second >= 60)
  {
    date.second = 0;
    date.minute++;
  }

  if (date.minute >= 60)
  {
    date.minute = 0;
    date.hour++;
  }

  if (date.hour >= 24)
  {
    date.hour = 0;
    date.day++;
  }

  if (date.month == 2)
  {
    int isLeapYear = (date.year % 4 == 0 && date.year % 100 != 0) || (date.year % 400 == 0);
    int daysInMonth = isLeapYear ? 29 : 28;

    if (date.day > daysInMonth)
    {
      date.day = 1;
      date.month++;
    }
  }
  else if (date.month == 4 || date.month == 6 || date.month == 9 || date.month == 11)
  {
    if (date.day > 30)
    {
      date.day = 1;
      date.month++;
    }
  }
  else
  {
    if (date.day > 31)
    {
      date.day = 1;
      date.month++;
    }
  }

  if (date.month > 12)
  {
    date.month = 1;
    date.year++;
  }
}

void showDatetime()
{
  if (!hasLastDate)
  {
    command(0x02);
    lcd_write_2digits(date.day);
    lcd_data('/');
    lcd_write_2digits(date.month);
    lcd_data('/');
    lcd_write_4digits(date.year);

    command(0xc0);
    lcd_write_2digits(date.hour);
    lcd_data(':');
    lcd_write_2digits(date.minute);
    lcd_data(':');
    lcd_write_2digits(date.second);

    lastDate = date;
    hasLastDate = 1;
    return;
  }

  if (date.day != lastDate.day)
  {
    lcd_set_cursor(0, 0);
    lcd_write_2digits(date.day);
  }

  if (date.month != lastDate.month)
  {
    lcd_set_cursor(0, 3);
    lcd_write_2digits(date.month);
  }

  if (date.year != lastDate.year)
  {
    lcd_set_cursor(0, 6);
    lcd_write_4digits(date.year);
  }

  if (date.hour != lastDate.hour)
  {
    lcd_set_cursor(1, 0);
    lcd_write_2digits(date.hour);
  }

  if (date.minute != lastDate.minute)
  {
    lcd_set_cursor(1, 3);
    lcd_write_2digits(date.minute);
  }

  if (date.second != lastDate.second)
  {
    lcd_set_cursor(1, 6);
    lcd_write_2digits(date.second);
  }

  lastDate = date;
}

void Timer_IQR_Handler(void *isr_context)
{
  increaseTime();
  showDatetime();

  IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_BASE,
                                  ALTERA_AVALON_TIMER_STATUS_TO_MSK);
}

void Timer_Init(void)
{
  unsigned int period = 0;
  IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_BASE,
                                   ALTERA_AVALON_TIMER_CONTROL_STOP_MSK);

  period = 50000000 - 1;
  IOWR_ALTERA_AVALON_TIMER_PERIODL(TIMER_BASE, period);
  IOWR_ALTERA_AVALON_TIMER_PERIODH(TIMER_BASE, period >> 16);

  IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_BASE,
                                   ALTERA_AVALON_TIMER_CONTROL_CONT_MSK |
                                       ALTERA_AVALON_TIMER_CONTROL_ITO_MSK |
                                       ALTERA_AVALON_TIMER_CONTROL_START_MSK);
}

int main(void)
{
  lcd_init();
  IOWR_ALTERA_AVALON_PIO_DATA(LCD_ON_BASE, 0x01);
  IOWR_ALTERA_AVALON_PIO_DATA(LCD_BLON_BASE, 0x01);
  showDatetime();

  Timer_Init();
  alt_ic_isr_register(0, TIMER_IRQ, Timer_IQR_Handler, (void *)0, (void *)0);

  while (1)
    ;

  return 0;
}
