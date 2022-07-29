#include <ti/drivers/GPIO.h>
#include "ti_drivers_config.h"
#include "gpio.h"
#include "constants.h"


void setup_gpio()
{
    GPIO_init();
    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

} // end


