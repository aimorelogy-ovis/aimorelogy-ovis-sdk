#ifndef RT_CONFIG_H__
#define RT_CONFIG_H__

/* RT-Thread Kernel */

#define RT_NAME_MAX 8
#define RT_CPUS_NR 1
#define RT_ALIGN_SIZE 8
#define RT_THREAD_PRIORITY_8
#define RT_THREAD_PRIORITY_MAX 8
#define RT_TICK_PER_SECOND 1000
#define IDLE_THREAD_STACK_SIZE 8192

/* kservice optimization */

/* end of kservice optimization */

/* klibc optimization */

#define RT_KLIBC_USING_TINY_SIZE
#define RT_KLIBC_USING_PRINTF_LONGLONG
/* end of klibc optimization */

/* Inter-Thread communication */

#define RT_USING_SEMAPHORE
#define RT_USING_MUTEX
/* end of Inter-Thread communication */

/* Memory Management */

#define RT_USING_MEMPOOL
#define RT_USING_SMALL_MEM
#define RT_USING_SMALL_MEM_AS_HEAP
#define RT_USING_HEAP
/* end of Memory Management */
#define RT_USING_DEVICE
#define RT_USING_DEVICE_OPS
#define RT_USING_CONSOLE
#define RT_CONSOLEBUF_SIZE 256
#define RT_CONSOLE_DEVICE_NAME "uart0"
#define RT_VER_NUM 0x50200
#define RT_USING_STDC_ATOMIC
#define RT_BACKTRACE_LEVEL_MAX_NR 32
/* end of RT-Thread Kernel */
#define ARCH_CPU_64BIT
#define ARCH_RISCV
#define ARCH_RISCV64
#define ARCH_USING_NEW_CTX_SWITCH
#define ARCH_USING_RISCV_COMMON64

/* RT-Thread Components */


/* DFS: device virtual file system */

/* end of DFS: device virtual file system */

/* Device Drivers */

#define RT_USING_DEVICE_IPC
#define RT_UNAMED_PIPE_NUMBER 64
#define RT_USING_SERIAL
#define RT_USING_SERIAL_V1
#define RT_SERIAL_USING_DMA
#define RT_SERIAL_RB_BUFSZ 64
#define RT_USING_CPUTIME
#define CPUTIME_TIMER_FREQ 0
#define RT_USING_SPI
#define RT_USING_PIN
#define RT_USING_KTIME
/* end of Device Drivers */

/* C/C++ and POSIX layer */

/* ISO-ANSI C layer */

/* Timezone and Daylight Saving Time */

#define RT_LIBC_USING_LIGHT_TZ_DST
#define RT_LIBC_TZ_DEFAULT_HOUR 8
#define RT_LIBC_TZ_DEFAULT_MIN 0
#define RT_LIBC_TZ_DEFAULT_SEC 0
/* end of Timezone and Daylight Saving Time */
/* end of ISO-ANSI C layer */

/* POSIX (Portable Operating System Interface) layer */

#define RT_USING_POSIX_DELAY
#define RT_USING_POSIX_CLOCK
#define RT_USING_PTHREADS
#define PTHREAD_NUM_MAX 8

/* Interprocess Communication (IPC) */


/* Socket is in the 'Network' category */

/* end of Interprocess Communication (IPC) */
/* end of POSIX (Portable Operating System Interface) layer */
/* end of C/C++ and POSIX layer */

/* Network */

/* end of Network */

/* Memory protection */

/* end of Memory protection */

/* Utilities */

/* end of Utilities */

/* Using USB legacy version */

/* end of Using USB legacy version */
/* end of RT-Thread Components */

/* RT-Thread Utestcases */

/* end of RT-Thread Utestcases */

/* General Drivers Configuration */

#define BSP_USING_UART
#define BSP_USING_UART0
#define BSP_UART0_RX_PINNAME "UART0_RX"
#define BSP_UART0_TX_PINNAME "UART0_TX"
#define BSP_UART_IRQ_BASE 44
#define BSP_USING_SPI
#define BSP_USING_SPI0
#define BSP_SPI0_SCK_PINNAME "SPINOR_SCK"
#define BSP_SPI0_SDO_PINNAME "SPINOR_MOSI"
#define BSP_SPI0_SDI_PINNAME "SPINOR_MISO"
#define BSP_SPI0_CS_PINNAME "SPINOR_CS_X"
#define BSP_USING_SPI_FLASH
/* end of General Drivers Configuration */
#define BSP_USING_CV18XX
#define C906_PLIC_PHY_ADDR 0x70000000
#define IRQ_MAX_NR 101
#define BSP_GPIO_IRQ_BASE 60
#define BSP_SYS_GPIO_IRQ_BASE 70
#define __STACKSIZE__ 8192
#define SOC_TYPE_CV181XC_QFN
#define BOARD_TYPE_CV1810C_WEVB_0006A_SPINOR

#endif
