#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ti/devices/msp432e4/driverlib/driverlib.h>
#include <ti/drivers/UART.h>
#include "FreeRTOS.h"
#include "task.h"
#include "uart_debug.h"
#include  "main.h"
#include "constants.h"
#include "ti_drivers_config.h"

struct uart_debug_data
{
    UART_Handle uart;
} ud;

void setup_uart_debug()
{
#ifdef DEBUG_APP
    UART_Params params;
    UART_Params_init(&params);
    params.baudRate = 9600;
    params.readMode = UART_MODE_BLOCKING;
    params.writeMode = UART_MODE_BLOCKING;
    params.readTimeout = UART_WAIT_FOREVER;
    params.writeTimeout = UART_WAIT_FOREVER;
    params.readDataMode = UART_DATA_TEXT;
    params.writeDataMode = UART_DATA_TEXT;
    params.readReturnMode =  UART_RETURN_FULL;
    params.readEcho = UART_ECHO_OFF;
    ud.uart = UART_open(CONFIG_UART_0, &params);
#endif
} // end


// This function reads from the debug uart and blocks until a read occurs.
char read_debug_uart()
{
    char c = 0;
    int32_t rv = UART_read(ud.uart, &c, 1);
    if (rv <= 0) return 0;
    return c;
} // end


void print_debug(char *s)
{
#ifdef DEBUG_APP
    // must use writePolling to ensure that text is written when using an RTOS
    UART_writePolling(ud.uart, s, strlen(s));
#endif
} // end


void print_debug_char(char c)
{
#ifdef DEBUG_APP
    UART_writePolling(ud.uart, &c, 1);
#endif
} // end


// Function to print to the console for debugging or logging
// Called by FreeRTOS+TCP
void vLoggingPrintf( const char *pcFormatString, ... )
{

#ifdef DEBUG_APP
    char buffer[UART_BUFFER_SIZ];
    va_list args;
    va_start (args, pcFormatString);
    vsnprintf (buffer, sizeof(buffer), pcFormatString, args);
    va_end (args);
    print_debug(buffer);
#endif

} // end


//---------------------------------------------------------------------------
// Example UART code (driverlib)
//---------------------------------------------------------------------------

void setup_uart_debug_driverlib()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));
    UARTConfigSetExpClk(UART0_BASE, get_clock_speed(),  UART_BAUD,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_NONE));
} // end


void print_debug_driverlib(char *s)
{
    // the critical section is required to ensure that the entire string is transmitted
    portENTER_CRITICAL();
    while(*s)
    {
        UARTCharPut(UART0_BASE, *s++);
        while(UARTBusy(UART0_BASE)==true) {}
    }
    portEXIT_CRITICAL();
} // end

