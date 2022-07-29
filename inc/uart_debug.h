#pragma once
void setup_uart_debug();
void print_debug(char *s);
void print_debug_char(char c);
void setup_uart_debug_driverlib();
void print_debug_driverlib(char *s);
void vLoggingPrintf( const char *pcFormatString, ... );
char read_debug_uart();


