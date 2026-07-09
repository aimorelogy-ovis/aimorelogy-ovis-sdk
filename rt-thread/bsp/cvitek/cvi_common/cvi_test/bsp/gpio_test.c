#include <rtthread.h>
#include <drv_gpio.h>
#include <board.h>
#include <drivers/dev_pin.h>
#include <mmio.h>
#include "pinctrl.h"
#include <drv_pinmux.h>
#include <stdio.h>

rt_uint16_t pinA, pinB;

static volatile int interrupt_count = 0;
static rt_sem_t interrupt_sem;

static void test_gpio_init(void)
{
    PINMUX_CONFIG(IIC0_SCL, XGPIOA_28);
    pinA = rt_pin_get("A28");

    PINMUX_CONFIG(IIC0_SDA, XGPIOA_29);
    pinB = rt_pin_get("A29");
}

static void gpio_interrupt_handler(void *args)
{
    interrupt_count++;
    rt_sem_release(interrupt_sem);
    rt_kprintf("!INT!");
}

static int gpio_test(int argc, char **argv)
{
    rt_kprintf("\n--- Starting GPIO Driver Test ---\n");

    // init
    test_gpio_init();
    rt_pin_mode(pinA, PIN_MODE_OUTPUT);
    rt_pin_mode(pinB, PIN_MODE_INPUT);

    // output mode
    rt_kprintf("\n[1/3] Basic Output Test:\n");
    for (int i = 0; i < 5; i++) {
        rt_pin_write(pinA, PIN_HIGH);
        rt_kprintf("Iteration %d: PIN_HIGH\n", i + 1);
        rt_thread_mdelay(500);

        rt_pin_write(pinA, PIN_LOW);
        rt_kprintf("Iteration %d: PIN_LOW\n", i + 1);
        rt_thread_mdelay(500);
    }

    // input mode
    rt_kprintf("\n[2/3] Input Mode Test:\n");
    rt_kprintf("Please connect PA0(OUT) to PA1(IN) with a jumper wire\n");
    rt_thread_mdelay(2000);

    int read_value;
    for (int i = 0; i < 5; i++) {
        rt_pin_write(pinA, (i % 2) ? PIN_HIGH : PIN_LOW);

        read_value = rt_pin_read(pinB);

        rt_kprintf("OUT: %s -> IN: %s [%s]\n",
                  (i % 2) ? "HIGH" : "LOW",
                  read_value ? "HIGH" : "LOW",
                  ((i % 2) == read_value) ? "PASS" : "FAIL");
        
        rt_thread_mdelay(1000);
    }

    // interrupt mode
    rt_kprintf("\n[3/3] Interrupt Mode Test:\n");
    rt_kprintf("Please connect PA0(OUT) to PA2(INT) with a jumper wire\n");
    rt_thread_mdelay(3000);

    interrupt_sem = rt_sem_create("gpio_int", 0, RT_IPC_FLAG_FIFO);
    if (!interrupt_sem) {
        rt_kprintf("Error: Failed to create semaphore!\n");
        return -1;
    }

    rt_pin_mode(pinB, PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(pinB, PIN_IRQ_MODE_RISING, 
                      gpio_interrupt_handler, RT_NULL);
    rt_pin_irq_enable(pinB, PIN_IRQ_ENABLE);

    interrupt_count = 0;

    for (int i = 0; i < 5; i++) {
        rt_pin_write(pinA, PIN_LOW);
        rt_thread_mdelay(100);
        rt_pin_write(pinA, PIN_HIGH);

        if (rt_sem_take(interrupt_sem, RT_WAITING_FOREVER) == RT_EOK) {
            rt_kprintf("Interrupt %d triggered\n", i + 1);
        }

        rt_thread_mdelay(500);
    }

    rt_kprintf("Total interrupts: %d (Expected: 5) [%s]\n", 
              interrupt_count, 
              (interrupt_count == 5) ? "PASS" : "FAIL");

    rt_pin_irq_enable(pinB, PIN_IRQ_DISABLE);
    rt_pin_detach_irq(pinB);
    rt_sem_delete(interrupt_sem);

    rt_kprintf("\n--- GPIO Test Completed ---\n");
    return 0;
}

MSH_CMD_EXPORT(gpio_test, GPIO driver test);
