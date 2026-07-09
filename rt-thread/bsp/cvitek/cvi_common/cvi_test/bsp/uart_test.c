#include <rtthread.h>
#include <rtdevice.h>
#include <rthw.h>
#include <rtdbg.h>

#define UART_TEST_DEVICE     "uart2"     /* Test device, custom to your device */
#define TEST_STR             "Hello RT-Thread UART1 Test!\n"
#define TEST_STR_LEN         (sizeof(TEST_STR) - 1)
#define TEST_TIMEOUT         RT_TICK_PER_SECOND * 2

static int uart_test(void)
{
    rt_device_t dev = RT_NULL;
    char uart_dev_name[RT_NAME_MAX] = UART_TEST_DEVICE;
    char recv_buf[64] = {0};
    rt_size_t total_recv = 0;
    rt_tick_t start_time;
    rt_size_t send_len = 0;

    rt_kprintf("\n--- Starting UART Test ---\n");
    rt_kprintf("Testing UART: %s\n", uart_dev_name);
    rt_kprintf("Please short connect the TX and RX pins!\n");

    dev = rt_device_find(uart_dev_name);
    if (!dev)
    {
        rt_kprintf("[FAIL] Device %s not found!\n", uart_dev_name);
        return -RT_ERROR;
    }

    if (rt_device_open(dev, RT_DEVICE_FLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("[FAIL] Open device %s failed!\n", uart_dev_name);
        return -RT_ERROR;
    }

    send_len = rt_device_write(dev, 0, TEST_STR, TEST_STR_LEN);
    if (send_len != TEST_STR_LEN)
    {
        rt_kprintf("[FAIL] Send data failed! Only send %d bytes.\n", send_len);
        rt_device_close(dev);
        return -RT_ERROR;
    }
    rt_kprintf("[PASS] [1/3] Sent %d bytes: %s", send_len, TEST_STR);

    start_time = rt_tick_get();
    total_recv = 0;

    while (total_recv < TEST_STR_LEN)
    {
        rt_size_t recv_len;
        recv_len = rt_device_read(dev, 0, recv_buf + total_recv, TEST_STR_LEN - total_recv);
        if (recv_len > 0)
        {
            total_recv += recv_len;
        }

        if (rt_tick_get() - start_time > TEST_TIMEOUT)
        {
            break;
        }
    }

    if (total_recv != TEST_STR_LEN)
    {
        rt_kprintf("[FAIL] Receive timeout! Only received %d bytes.\n", total_recv);
        rt_device_close(dev);
        return -RT_ERROR;
    }
    rt_kprintf("[PASS] [2/3] Received %d bytes\n", total_recv);

    if (rt_memcmp(TEST_STR, recv_buf, TEST_STR_LEN) == 0)
    {
        rt_kprintf("[PASS] [3/3] Data verification successful!\n");
    }
    else
    {
        rt_kprintf("[FAIL] Received data does not match!\n");
        rt_kprintf("Sent: %s", TEST_STR);
        rt_kprintf("Recv: %.*s\n", total_recv, recv_buf);
        rt_device_close(dev);
        return -RT_ERROR;
    }

    rt_device_close(dev);

    rt_kprintf("\n--- END UART Test ---\n");
    return RT_EOK;
}
MSH_CMD_EXPORT(uart_test, uart test [device name]);
