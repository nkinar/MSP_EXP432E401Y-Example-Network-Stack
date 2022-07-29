#include <stdint.h>
#include <string.h>

/* POSIX Header files */
#include <pthread.h>

/* RTOS header files */
#include <FreeRTOS.h>
#include <main.h>
#include <task.h>

#include <ti/devices/msp432e4/driverlib/driverlib.h>
#include <ti/drivers/Board.h>
#include <ti/drivers/emac/EMACMSP432E4.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/ADC.h>

#include "FreeRTOS.h"

#include "gpio.h"
#include "main.h"
#include "main_thread.h"
#include "constants.h"
#include "uart_debug.h"
#include "NetworkInterface.h"
#include "NetworkMiddleware.h"
#include "adc_rand.h"
#include "cli.h"
#include "ident.h"
#include "webserver.h"

#include "ti_drivers_config.h"


/*
NOTES:

(1)
The documentation for driverlib is found in:
C:/ti/simplelink_msp432e4_sdk_4_20_00_12/docs/driverlib/msp432e4/api_guide/html/index.html
The driverlib sources are in: C:\ti\simplelink_msp432e4_sdk_4_20_00_12\source\ti\devices\msp432e4\driverlib

(2)
To use the driverlib, you must have
--define=__MSP432E401Y__
in the compiler defines.

(3) Information on using the UART using driverlib
https://engineering.purdue.edu/ece477/Archive/2014/Spring/S14-Grp1/docs/software/LM4F-LaunchPad-12%20-%20UART.pdf

(4) The driverlib is used with the Ethernet interface.

(5) To start the NDK (if used), the
 ti_ndk_config_Global_startupFxn()
is called.  Here in this code, the function is not used since a custom driver is used for FreeRTOS.

(6) To use driverlib, __MSP432E401Y__ must be defined.

(7)
How to assign multiple IP addresses to a network card on the same interface:
https://ostechnix.com/how-to-assign-multiple-ip-addresses-to-single-network-card-in-linux/
This is required to be able to communicate between the Linux board and the device.
*/


struct main_data
{
    uint32_t ui32SysClock;
} md;


uint32_t get_clock_speed()
{
    return md.ui32SysClock;
} // end


// The clocks are setup later in FreeRTOS, but for initial setup, this function ensures that things
// are setup to the best of our ability.
void setup_clocks()
{
    md.ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                       SYSCTL_OSC_MAIN |
                                       SYSCTL_USE_PLL |
                                       SYSCTL_CFG_VCO_480),  MAIN_SYS_CLOCK);
    #if (__FPU_USED == 1)
      SCB->CPACR |= ((3UL << 10*2) |                 /* set CP10 Full Access */
                     (3UL << 11*2)  );               /* set CP11 Full Access */
    #endif

} // end


// Function to setup wired Ethernet
void setup_wired_ethernet()
{
    //setup the driver
    struct InternalNetworkInterfaceMSP432EConfig config;
    vGetInternalNetworkInterfaceMSP432EConfigDefaults(&config);
    config.setMACAddrInternal = false;
    config.ucMACAddr[0] = 0x70;
    config.ucMACAddr[1] = 0xFF;
    config.ucMACAddr[2] = 0x76;
    config.ucMACAddr[3] = 0x1C;
    config.ucMACAddr[4] = 0xC1;
    config.ucMACAddr[5] = 0xD0;
    vPublicSetupEMACNetwork(config);

    // setup the network stack middleware
    const char *devname = "chione";
    struct InternalNetworkMiddlewareData setupData;
    strncpy(setupData.deviceName, devname, strlen(devname));
    setupData.resetNetworkTaskEveryXSeconds = 60;           // Restart the network every 24 hours (86400 seconds) only when  setupData.resetNetworkTaskRunning == true
    setupData.resetNetworkTaskRunning = false;              // Run the network task to reset the network every so often (i.e. to periodically obtain a new IP address)

    // set the static IP address (uses helper function)
    vConvertOctetsToAddr(setupData.ucIPAddress, 192, 168, 1, 9);
    vConvertOctetsToAddr(setupData.ucNetMask, 255, 255, 255, 0);
    vConvertOctetsToAddr(setupData.ucGatewayAddress, 192, 168, 1, 1);
    vConvertOctetsToAddr(setupData.ucDNSServerAddress, 192, 168, 1, 1);

    vPublicSetupFreeRTOSTasks(setupData);
    // Start the RTOS scheduler by calling vTaskStartScheduler()
    // Use vPublicPreventNetworkReset() to block the network reset during a critical section of the code
    // Set the device name using vPublicSetupDeviceName()
} // end



void setup_hardware()
{
    Board_init();
    setup_clocks();
    setup_gpio();
    UART_init();
    setup_uart_debug();
    ADC_init();
    setup_adc_rand();
    setup_wired_ethernet();
} // end


void setup_tasks()
{
    start_blink_task();
    setup_console();
    setup_ident();
    set_ident_name("chione");
    setup_web();

} // end


int main(void)
{
    setup_hardware();
    setup_tasks();
    IntMasterEnable();

    print_debug("Running...");
    vTaskStartScheduler();
    return 0; // this should never be reached
} // end


void start_blink_task()
{
    /*
    xTaskCreate( vTaskBlink,
    "blink",
    400,
    NULL,
    9,
    NULL );
    */
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF);
} // end


void led_on()
{
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);
} // end

void led_off()
{
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF);
} // end


void start_debug_print_task()
{
    xTaskCreate( vTaskPrintDebug,
    "dp",
    400,
    NULL,
    1,
    NULL );
} // end


// example pThread startup to blink an LED
void pThreadExampleSetup()
{
    pthread_t           thread;
    pthread_attr_t      attrs;
    struct sched_param  priParam;
    int                 retc;

    /* Initialize the attributes structure with default values */
    pthread_attr_init(&attrs);

    /* Set priority, detach state, and stack size attributes */
    priParam.sched_priority = 1;
    retc = pthread_attr_setschedparam(&attrs, &priParam);
    retc |= pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
    retc |= pthread_attr_setstacksize(&attrs, THREADSTACKSIZE);
    if (retc != 0)
    {
        while (1) {}
    }
    retc = pthread_create(&thread, &attrs, mainThreadBlink, NULL);
    if (retc != 0)
    {
        while (1) {}
    }
} // end








