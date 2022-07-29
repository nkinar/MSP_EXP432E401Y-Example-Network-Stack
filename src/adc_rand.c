#include <ti/drivers/ADC.h>
#include <stdint.h>
#include "ti_drivers_config.h"
#include "adc_rand.h"
#include "constants.h"

struct adc_rand_data
{
    ADC_Params params;
    ADC_Handle adcHandle;
}ad;

// function to setup the adc
void setup_adc_rand()
{
    ADC_Params_init(&ad.params);
    ad.params.isProtected = true;  // ensure threadsafe
    ad.adcHandle = ADC_open(CONFIG_ADC_0, &ad.params);
} // end

// obtain the ADC output
uint16_t get_rand_adc_output()
{
    uint16_t result = 0;
    int_fast16_t rv = ADC_convert(ad.adcHandle, &result);
    if(rv == ADC_STATUS_SUCCESS) return result;
    return 0;
} // end

// Function to obtain 32-bit random number from the last bit of an ADC read
uint32_t obtain_rand32()
{
    uint32_t output = 0;
    for(uint8_t k = 0; k < 32; k++)
    {
        uint16_t num = get_rand_adc_output();
        if(CHECK_BIT(num, 0)) SET_BIT(output, k);
    }
    return output;
} // end











