#include <rtthread.h>
#include <rtdevice.h>

#define PWM_DEV_NAME        "pwm2" // custom to your pwm device
#define PWM_DEV_CHANNEL     10       // custom to your pwm channel

static void pwm_test(int argc, char *argv[])
{
    rt_device_t pwm_dev;
    rt_uint32_t period = 1000000; // 1MHz
    rt_uint32_t pulse = 500000; // duty cycle 50%
    int count = 10;
    int i;

    if (argc > 1) period = atoi(argv[1]);
    if (argc > 2) pulse = atoi(argv[2]);
    if (argc > 3) count = atoi(argv[3]);

    rt_kprintf("\n--- Starting Ethernet Driver Test ---\n");

    pwm_dev = rt_device_find(PWM_DEV_NAME);
    if (pwm_dev == RT_NULL) {
        rt_kprintf("[FAIL] PWM device %s not found!\n", PWM_DEV_NAME);
        return;
    }

    rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL, period, pulse);
    rt_kprintf("Set PWM: period=%dns, pulse=%dns\n", period, pulse);

    // enable pwm
    rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL);
    rt_kprintf("PWM channel %d enabled\n", PWM_DEV_CHANNEL);

    // change duty cycle
    rt_kprintf("Changing pulse width %d times...\n", count);
    for (i = 0; i < count; i++) {

        pulse = period * (100 + i * 80 / count) / 1000;
        rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL, period, pulse);
        
        rt_kprintf("[%2d/%d] Pulse: %dns \n", i+1, count, pulse);
        
        rt_thread_mdelay(2000);
    }

    rt_pwm_disable(pwm_dev, PWM_DEV_CHANNEL);
    rt_kprintf("PWM channel %d disabled\n", PWM_DEV_CHANNEL);
    rt_kprintf("--- [PASS] PWM Driver Test ---\n");
}

MSH_CMD_EXPORT(pwm_test, PWM test e.g: pwm_test [period=1000000] [pulse=500000] [count=10]);
