#ifndef UART_H
#define UART_H

#include <stdio.h>

#define D(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#define LOG(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)

void uart_putchar(char c, FILE *stream);
char uart_getchar(FILE *stream);

void uart_init(void);

extern FILE uart_output;
extern FILE uart_input;

#endif //UART_H