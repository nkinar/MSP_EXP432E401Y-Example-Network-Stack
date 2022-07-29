#pragma once
#include "FreeRTOS.h"

// turn on this debug to print debugging information to UART0
#define DEBUG_APP

#define MAIN_SYS_CLOCK      120000000
#define THREADSTACKSIZE     1024
#define UART_BAUD           9600
#define UART_BUFFER_SIZ     128

#define CLI_TASK_STACK      1000
#define CLI_TASK_PRIORITY   configMAX_PRIORITIES

// set a default device name
#define DEV_NAME_DEFAULT    "chione"
#define ERR_STRING          "ERROR"
#define DONE_STRING         "DONE"

#define CRLF_STR             "\r\n"

// port timeout
static const uint32_t PORT_RX_TIMEOUT = 30000;  // 30 s
static const uint32_t PORT_TX_TIMEOUT = 1000;   // 1 s
// port for network time server (NTP)
static const uint16_t NTP_PORT = 123;
// port on the client for NTP
static const uint16_t NTP_PORT_CLIENT = 1024;
// port on the client for identify port
static const uint16_t IDENT_PORT = 30303;
// backlog for ports (only allow two clients to connect at one time)
static const BaseType_t PORT_BACKLOG = 2;
// max number of bytes used for ident port
#define MAX_IDENT_BYTES     32
// max device name size
#define MAX_DEVICE_NAME_SIZ  32
// ident string
#define IDENT_STR "???"
// max number of bytes sent to web socket
#define MAX_WEB_BYTES   1024
// web port
static const uint16_t WEB_PORT = 80;
// stack size for web
#define THREAD_STACK_SIZE_WEB 2048

// Turn on this define to print out the ping
#define SEND_PING_PRINT_REPLY

// macro to check the bit
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))
// macros
// https://www.embeddedrelated.com/showcode/315.php
#define SET_BIT(x,y) x |= (1 << y)
#define CLEAR_BIT(x,y) x &= ~(1<< y)
#define TOGGLE_BIT(x,y) (x ^= (1<<y))


