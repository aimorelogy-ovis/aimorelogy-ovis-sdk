#include <rtthread.h>
#include <rtdevice.h>

#define ADC_DEV_NAME        "adc1" // custom to your adc device
#define ADC_CHANNEL         1       // custom to your adc channel
#define REFERENCE_VOLTAGE   3.3f
#define ADC_RESOLUTION      4096

static void adc_test(int argc, char *argv[])
{
    rt_adc_device_t adc_dev;
    rt_uint32_t raw_value;
    rt_uint32_t voltage;
    rt_err_t ret;
    int count = 10;

    if (argc > 1) {
        count = atoi(argv[1]);
        if (count <= 0) count = 10;
    }

    rt_kprintf("\n--- Starting ADC Driver Test ---\n");

    adc_dev = (rt_adc_device_t)rt_device_find(ADC_DEV_NAME);
    if (adc_dev == RT_NULL) {
        rt_kprintf("[FAIL] ADC device %s not found!\n", ADC_DEV_NAME);
        return;
    }

    // enable adc channel
    ret = rt_adc_enable(adc_dev, ADC_CHANNEL);
    if (ret != RT_EOK) {
        rt_kprintf("[FAIL] Enable ADC channel %d failed: %d\n", ADC_CHANNEL, ret);
        return;
    }
    rt_kprintf("ADC channel %d enabled\n", ADC_CHANNEL);

    // sampling
    rt_kprintf("Sampling %d times:\n", count);
    for (int i = 0; i < count; i++) {

        raw_value = rt_adc_read(adc_dev, ADC_CHANNEL);
        
        voltage = (raw_value * REFERENCE_VOLTAGE * 1000) / ADC_RESOLUTION;  // mv
        
        rt_kprintf("[%2d/%d] Raw: %4d -> Voltage: %d.%03dV\n", 
                  i + 1, count, raw_value, voltage / 1000, voltage % 1000);
        
        rt_thread_mdelay(200);
    }

    rt_adc_disable(adc_dev, ADC_CHANNEL);
    rt_kprintf("ADC channel %d disabled\n", ADC_CHANNEL);
    rt_kprintf("[PASS] ADC Driver Test Pass\n");
    rt_kprintf("\n--- END ADC Driver Test ---\n");
}

MSH_CMD_EXPORT(adc_test, ADC device test e.g: adc_test [count=10]);
