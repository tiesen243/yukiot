#include "datetime.h"

static Date date = {18, 3, 2026, 18, 14, 0};

const Date *datetime_get(void)
{
    return &date;
}

void datetime_tick(void)
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

void datetime_set(const Date *newDate)
{
    if (newDate == 0)
        return;

    date = *newDate;
}