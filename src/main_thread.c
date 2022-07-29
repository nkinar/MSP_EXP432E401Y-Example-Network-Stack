#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>
#include <ti/drivers/GPIO.h>
// #include <ti/drivers/I2C.h>
// #include <ti/drivers/SPI.h>
// #include <ti/drivers/UART.h>
// #include <ti/drivers/Watchdog.h>


#include "ti_drivers_config.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main_thread.h"
#include "uart_debug.h"
#include "adc_rand.h"


// function to blink an LED
void *mainThreadBlink(void *arg0)
{
    uint32_t time = 1;

    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);

    while (1)
    {
        sleep(time);
        GPIO_toggle(CONFIG_GPIO_LED_0);
    }
} // end


void vTaskBlink( void *pvParameters )
{
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        GPIO_toggle(CONFIG_GPIO_LED_0);
    }
} // end


void vTaskPrintDebug( void *pvParameters )
{
    while(1)
    {
        print_debug("Print to debug...\r\n");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
} // end

//---------------------------------------------------------------------------------

static TaskHandle_t xTask1 = NULL, xTask2 = NULL;
void create_tasks_check_notify()
{
    xTaskCreate( prvTask1, "Task1", 500, NULL, 1, &xTask1 );
    xTaskCreate( prvTask2, "Task2", 500, NULL, 1, &xTask2 );
} // end


void prvTask1( void *pvParameters )
{
    for( ;; )
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        /* Send notification to prvTask2(), bringing it out of the
        Blocked state. */
        xTaskNotifyGiveIndexed( xTask2, 0 );
        print_debug("Task 1 give\r\n");

        /* Block to wait for prvTask2() to notify this task. */
        // ulTaskNotifyTakeIndexed( 0, pdTRUE, portMAX_DELAY );
        // print_debug("Task 1 take\r\n");
    }
} // end


void prvTask2( void *pvParameters )
{
    for( ;; )
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        /* Block to wait for prvTask1() to notify this task. */
        ulTaskNotifyTake( pdTRUE, portMAX_DELAY );
        print_debug("Task 2 take\r\n");

        /* Send a notification to prvTask1(), bringing it out of the
        Blocked state. */
        // xTaskNotifyGive( xTask1 );
        // print_debug("Task 2 give\r\n");
    }
} // end


void vApplicationMallocFailedHook( void )
{
    print_debug("***MALLOC FAILED***\r\n");
    while(1){}

} // end


// stack overflow hook
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
    // Handle FreeRTOS Stack Overflow
    print_debug("Stack overflow...");
    while(1)
    {
    }
} // end










