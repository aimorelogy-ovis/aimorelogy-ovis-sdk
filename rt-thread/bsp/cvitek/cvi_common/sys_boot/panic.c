typedef struct {
    long X1;
    long X2;
    long X3;
    long X4;
    long X5;
    long X6;
    long X7;
    long X8;
    long X9;
    long X10;
    long X11;
    long X12;
    long X13;
    long X14;
    long X15;
    long X16;
    long X17;
    long X18;
    long X19;
    long X20;
    long X21;
    long X22;
    long X23;
    long X24;
    long X25;
    long X26;
    long X27;
    long X28;
    long X29;
    long X30;
    long X31;
    long MS_EPC;
    long MS_STATUS;
    long MS_CAUSE;
    long MS_TVAL;
} fault_context_t;
extern int rt_kprintf(const char *fmt, ...);
extern void rt_hw_cpu_reset(void);
void exceptionHandler(void *context)
{
    fault_context_t *fc = (fault_context_t *)context;
    rt_kprintf("Exception ++++++++++ MS_EPC 0x%lx, MS_STATUS 0x%lx, MS_TVAL 0x%lx, CPU Exception: NO.0x%lx\r\n",
                        fc->MS_EPC, fc->MS_STATUS, fc->MS_TVAL, fc->MS_CAUSE);
    rt_hw_cpu_reset();
}