#include <string.h>
#include <stdio.h>
#include <time.h>

#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"
#include "task.h"

#include "cli.h"
#include "uart_debug.h"
#include "network_time.h"
#include "timeconvert.h"
#include "constants.h"

static TaskHandle_t xTaskCLI = NULL;
static char pcWriteBufferLocal[ configCOMMAND_INT_MAX_OUTPUT_SIZE ];


// Function used to print info to the command line
BaseType_t printInfo( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    snprintf(pcWriteBuffer, xWriteBufferLen, "Chione system\r\n");

    return pdFALSE; // finished return string
} // end


// Function to print runtime stats
// Modified to ensure that the size of the write buffer is preserved.
// https://www.freertos.org/uxTaskGetSystemState.html
void vTaskGetRunTimeStatsLocal()
{
    TaskStatus_t *pxTaskStatusArray;
    UBaseType_t uxArraySize, x;
    uint32_t ulTotalRunTime;

   memset(pcWriteBufferLocal, 0,  configCOMMAND_INT_MAX_OUTPUT_SIZE);
   uxArraySize = uxTaskGetNumberOfTasks();
   pxTaskStatusArray = pvPortMalloc( uxArraySize * sizeof( TaskStatus_t ) );

   if( pxTaskStatusArray != NULL )
   {
      uxArraySize = uxTaskGetSystemState( pxTaskStatusArray,
                                 uxArraySize,
                                 &ulTotalRunTime );
      if( 1 )
      {
         for( x = 0; x < uxArraySize; x++ )
         {
            snprintf( pcWriteBufferLocal,  configCOMMAND_INT_MAX_OUTPUT_SIZE, "%15s %15lu %15hu \r\n",
                                 pxTaskStatusArray[ x ].pcTaskName,
                                 pxTaskStatusArray[ x ].ulRunTimeCounter,
                                 pxTaskStatusArray[ x ].usStackHighWaterMark);
            vLoggingPrintf( pcWriteBufferLocal);
         }
      }
      vPortFree( pxTaskStatusArray );
   }
} // end


BaseType_t printTasks( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    vTaskGetRunTimeStatsLocal();
    snprintf(pcWriteBuffer, xWriteBufferLen, DONE_STRING "\r\n");
    return pdFALSE; // finished
} // end


BaseType_t printNetworkTime( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    char *addr = "192.168.1.2";
    // char *addr = "time.google.com";
    uint64_t epoch = 0;

    struct tm t;
    bool rv = get_ctime_from_ntp(&t, addr, epoch);
    if(rv==false)
    {
        vLoggingPrintf(ERR_STRING);
    }
    else
    {
        char *s = get_timestring(&t);
        vLoggingPrintf("%s\r\n", s);
    }
    snprintf(pcWriteBuffer, xWriteBufferLen, DONE_STRING "\r\n");
    return pdFALSE; // finished
} // end


// Register all of the commands for the CLI
void cli_command_register()
{
    //------------------------------------------------------------
    // DEFINITIONS
    //------------------------------------------------------------
    static const CLI_Command_Definition_t xInfoCommand =
    {
        "info",
        "info: Print info about the system to the terminal\r\n",
        printInfo,
        0
    };
    static const CLI_Command_Definition_t xprintTasksCommand =
     {
         "tasks",
         "tasks: Print info about the tasks\r\n",
         printTasks,
         0
     };
    static const CLI_Command_Definition_t xprintNetworkTime =
     {
         "ntime",
         "ntime: Print the network time\r\n",
         printNetworkTime,
         0
     };
    //------------------------------------------------------------
    // PRINT COMMANDS
    //------------------------------------------------------------
    FreeRTOS_CLIRegisterCommand( &xInfoCommand);
    FreeRTOS_CLIRegisterCommand( &xprintTasksCommand);
    FreeRTOS_CLIRegisterCommand( &xprintNetworkTime);
} // end


//----------------------------------------------------------------
// CLI task
//----------------------------------------------------------------

// Function to print the prompt character
void print_prompt()
{
    print_debug("> ");

} // end


// This is the task that processes the console input
void vCommandConsoleTask( void *pvParameters )
{
    static char pcInputString[ configCOMMAND_INT_MAX_OUTPUT_SIZE ];
    static char pcOutputString[ configCOMMAND_INT_MAX_OUTPUT_SIZE ];
    memset( pcInputString, 0, configCOMMAND_INT_MAX_OUTPUT_SIZE);
    BaseType_t xMoreDataToFollow;
    uint32_t index = 0;
    print_prompt();
    for(;;)
    {
        char c = read_debug_uart();
        if(c == '\n')
        {
            print_debug("\r\n");
            do
            {
                xMoreDataToFollow = FreeRTOS_CLIProcessCommand
                                             (
                                                 pcInputString,
                                                 pcOutputString,
                                                 configCOMMAND_INT_MAX_OUTPUT_SIZE
                                             );
                print_debug(pcOutputString);
            }
            while(xMoreDataToFollow != pdFALSE);
            index = 0;
            memset( pcInputString, 0, configCOMMAND_INT_MAX_OUTPUT_SIZE);
            print_debug("\r\n");
            print_debug("\r\n");
            print_prompt();
        }
        else if (c == '\r') continue;
        else if (c == '\b')
        {
            if(index > 0)
            {
                index--;
                pcInputString[index] = 0x00;
                print_debug_char(0x08); // backspace
                print_debug_char(' ');  // space
                print_debug_char(0x08); // backspace
            }
        }
        else
        {
            if(index < configCOMMAND_INT_MAX_OUTPUT_SIZE-1)
            {
                pcInputString[index++] = c;
                print_debug_char(c);  // echo to terminal
            }
            else
            {   // clear the buffer
                index = 0;
                memset( pcInputString, 0, configCOMMAND_INT_MAX_OUTPUT_SIZE);
            }
        }
    }
} // end


// Call this function to setup the console
void setup_console()
{
    cli_command_register();
    xTaskCreate(  vCommandConsoleTask,
                     "cli",
                     CLI_TASK_STACK,
                     NULL,
                     CLI_TASK_PRIORITY,
                     &xTaskCLI );

} // end






















