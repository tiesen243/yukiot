#ifndef DATETIME_H
#define DATETIME_H

typedef struct
{
    int day;
    int month;
    int year;

    int hour;
    int minute;
    int second;
} Date;

const Date *datetime_get(void);
void datetime_tick(void);
void datetime_set(const Date *newDate);

#endif