#include "altera_avalon_uart_regs.h"
#include "system.h"

#include "uart.h"

int is_uart_available(void)
{
    return (IORD_ALTERA_AVALON_UART_STATUS(UART_0_BASE) & 0x80) != 0x80;
}

void uart_send_char(char c)
{
    while (!(IORD_ALTERA_AVALON_UART_STATUS(UART_0_BASE) & ALTERA_AVALON_UART_STATUS_TRDY_MSK))
        ;
    IOWR_ALTERA_AVALON_UART_TXDATA(UART_0_BASE, c);
}

void uart_send_string(const char *str)
{
    while (*str)
    {
        uart_send_char(*str++);
    }

    uart_send_char('\n');
}

char uart_receive_char(void)
{
    return IORD_ALTERA_AVALON_UART_RXDATA(UART_0_BASE);
}

void uart_receive_string(char *buffer, int max_length)
{
    int i = 0;
    while (i < max_length - 1)
    {
        char c = uart_receive_char();
        if (c == '\n' || c == '\r')
        {
            break;
        }
        buffer[i++] = c;
    }
    buffer[i] = '\0';
}