#include <vi_debug.h>
// #include <aos/cli.h>
// #include <aos/kernel.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vip_spinlock.h>
// #include <drv/tick.h>
#include <errno.h>
#ifdef AOS_COMP_FATFS
#include <fatfs_vfs.h>

#include "vfs.h"
#endif
#include <fcntl.h>
#include <semaphore.h>

static sem_t log_wait;
/* index and sequence number of the first record stored in the buffer */
static u64 log_first_seq;
static u32 log_first_idx;

/* index and sequence number of the next record to store in the buffer */
static u64 log_next_seq;
static u32 log_next_idx;

/* the next printk record to read after the last 'clear' command */
static u64 clear_seq;
static u32 clear_idx;

/**
 * log buf struct
 * ------------------------------------------------------------------------
 * |       0         |        1         |        2        |       3       |
 * -----------------------------------------------------------------------|
 */
struct printk_log {
    u64 ts_nsec;   /* timestamp in nanoseconds */
    u16 len;       /* length of entire record */
    u16 text_len;  /* length of text buffer */
    u16 dict_len;  /* length of dictionary buffer */
    u8  facility;  /* syslog facility */
    u8  flags : 5; /* internal record flags */
    u8  level : 3; /* syslog level */
};

#define LOG_ALIGN            __alignof__(struct printk_log)
#define CONFIG_LOG_BUF_SHIFT 16  // 64KB
#define __LOG_BUF_LEN        (1 << CONFIG_LOG_BUF_SHIFT)

#define PREFIX_MAX   32
#define LOG_LINE_MAX (1024 - PREFIX_MAX)
#define MAX(x, y)    (((x) < (y)) ? (y) : (x))

static char       __log_buf[__LOG_BUF_LEN];
static char      *log_buf     = __log_buf;
static u32        log_buf_len = __LOG_BUF_LEN;
static spinlock_t printk_lock;
rt_thread_t       printf_thread;

/* compute the message size including the padding bytes */
static u32 msg_used_size(u16 text_len, u32 *pad_len)
{
    u32 size;

    size     = sizeof(struct printk_log) + text_len;
    *pad_len = (-size) & (LOG_ALIGN - 1);
    size += *pad_len;

    return size;
}

/* human readable text of the record */
static char *log_text(const struct printk_log *msg)
{
    return (char *)msg + sizeof(struct printk_log);
}

/* optional key/value pair dictionary attached to the record */
static char *log_dict(const struct printk_log *msg)
{
    return (char *)msg + sizeof(struct printk_log) + msg->text_len;
}

/* get record by index; idx must point to valid msg */
static struct printk_log *log_from_idx(u32 idx)
{
    struct printk_log *msg = (struct printk_log *)(log_buf + idx);

    /*
     * A length == 0 record is the end of buffer marker. Wrap around and
     * read the message at the start of the buffer.
     */
    if (!msg->len) return (struct printk_log *)log_buf;
    return msg;
}

/* get next record; idx must point to valid msg */
static u32 log_next(u32 idx)
{
    struct printk_log *msg = (struct printk_log *)(log_buf + idx);

    /* length == 0 indicates the end of the buffer; wrap */
    /*
     * A length == 0 record is the end of buffer marker. Wrap around and
     * read the message at the start of the buffer as *this* one, and
     * return the one after that.
     */
    if (!msg->len) {
        msg = (struct printk_log *)log_buf;
        return msg->len;
    }
    return idx + msg->len;
}

/*
 * Check whether there is enough free space for the given message.
 *
 * The same values of first_idx and next_idx mean that the buffer
 * is either empty or full.
 *
 * If the buffer is empty, we must respect the position of the indexes.
 * They cannot be reset to the beginning of the buffer.
 */
static int logbuf_has_space(u32 msg_size, bool empty)
{
    u32 free;

    if (log_next_idx > log_first_idx || empty)
        free = MAX(log_buf_len - log_next_idx, log_first_idx);
    else
        free = log_first_idx - log_next_idx;

    /*
     * We need space also for an empty header that signalizes wrapping
     * of the buffer.
     */
    return free >= msg_size + sizeof(struct printk_log);
}

static int log_make_free_space(u32 msg_size)
{
    while (log_first_seq < log_next_seq && !logbuf_has_space(msg_size, false)) {
        /* drop old messages until we have enough contiguous space */
        log_first_idx = log_next(log_first_idx);
        log_first_seq++;
    }

    if (clear_seq < log_first_seq) {
        clear_seq = log_first_seq;
        clear_idx = log_first_idx;
    }

    /* sequence numbers are equal, so the log buffer is empty */
    if (logbuf_has_space(msg_size, log_first_seq == log_next_seq)) return 0;

    return -1;
}

#define MAX_LOG_TAKE_PART 4
static const char trunc_msg[] = "<truncated>";

static u32 truncate_msg(u16 *text_len, u16 *trunc_msg_len, u32 *pad_len)
{
    /*
     * The message should not take the whole buffer. Otherwise, it might
     * get removed too soon.
     */
    u32 max_text_len = log_buf_len / MAX_LOG_TAKE_PART;

    if (*text_len > max_text_len) *text_len = max_text_len;
    /* enable the warning message */
    *trunc_msg_len = strlen(trunc_msg);
    /* disable the "dict" completely */
    /* compute the size again, count also the warning message */
    return msg_used_size(*text_len + *trunc_msg_len, pad_len);
}

static int log_store(const char *text, u16 text_len)
{
    struct printk_log *msg;
    u32                size, pad_len;
    u16                trunc_msg_len = 0;

    /* number of '\0' padding bytes to next message */
    size = msg_used_size(text_len, &pad_len);

    if (log_make_free_space(size)) {
        /* truncate the message if it is too long for empty buffer */
        size = truncate_msg(&text_len, &trunc_msg_len, &pad_len);
        printf("truncate_msg log_first_idx=%d log_next_idx=%d msg->len=%d\n", log_first_idx,
               log_next_idx, size);
        /* survive when the log buffer is too small for trunc_msg */
        if (log_make_free_space(size)) return 0;
    }

    if (log_next_idx + size + sizeof(struct printk_log) > log_buf_len) {
        /*
         * This message + an additional empty header does not fit
         * at the end of the buffer. Add an empty header with len == 0
         * to signify a wrap around.
         */
        memset(log_buf + log_next_idx, 0, sizeof(struct printk_log));
        log_next_idx = 0;
    }

    /* fill message */
    msg = (struct printk_log *)(log_buf + log_next_idx);
    memcpy(log_text(msg), text, text_len);
    msg->text_len = text_len;
    if (trunc_msg_len) {
        memcpy(log_text(msg) + text_len, trunc_msg, trunc_msg_len);
        msg->text_len += trunc_msg_len;
    }

    msg->ts_nsec = rt_tick_get() * 1000;
    memset(log_dict(msg), 0, pad_len);
    msg->len = size;

    /* insert message */
    log_next_idx += msg->len;
    log_next_seq++;

    return msg->text_len;
}

static int vprintk_store(const char *fmt, va_list args)
{
    static char textbuf[LOG_LINE_MAX];
    char       *text = textbuf;
    size_t      text_len;

    text_len = vsnprintf(text, sizeof(textbuf), fmt, args);

    /* mark and strip a trailing newline */
    if (text_len && text[text_len - 1] == '\n') {
        text_len--;
    }

    return log_store(text, text_len);
}

static int vprintk_default(const char *fmt, va_list args)
{
    unsigned long flags;
    int           printed_len;
    u64           curr_log_seq;
    bool          pending_output;
    static bool   first_init = true;

    if (first_init) {
        sem_init(&log_wait, 0, 0);
        first_init = false;
    }

    // TODO, if we printk in interrupt handler?
    spin_lock_irqsave(&printk_lock, flags);
    curr_log_seq   = log_next_seq;
    printed_len    = vprintk_store(fmt, args);
    pending_output = (curr_log_seq != log_next_seq);
    spin_unlock_irqrestore(&printk_lock, flags);

    if (pending_output) {
        sem_post(&log_wait);
    }

    return printed_len;
}

static int vprintk_func(const char *fmt, va_list args)
{
    // we assume one case, no obstacles, maybe we need other case.
    return vprintk_default(fmt, args);
}

int vi_printk(const char *fmt, ...)
{
    va_list args;
    int     r;

    va_start(args, fmt);
    r = vprintk_func(fmt, args);
    va_end(args);

    return r;
}

#define do_div(n, base)                   \
    ({                                    \
        uint32_t __base = (base);         \
        uint32_t __rem;                   \
        __rem = ((uint64_t)(n)) % __base; \
        (n)   = ((uint64_t)(n)) / __base; \
        __rem;                            \
    })

static size_t print_time(u64 ts, char *buf)
{
    unsigned long rem_nsec;

    rem_nsec = do_div(ts, 1000000000);

    if (!buf) return snprintf(NULL, 0, "[%5lu.000000] ", (unsigned long)ts);

    return sprintf(buf, "[%5lu.%06lu] ", (unsigned long)ts, rem_nsec / 1000);
}

static size_t print_prefix(const struct printk_log *msg, bool syslog, char *buf)
{
    size_t len = 0;
    // unsigned int prefix = (msg->facility << 3) | msg->level;
    /*
    if (syslog) {
        if (buf) {
            len += sprintf(buf, "<%u>", prefix);
        } else {
            len += 3;
            if (prefix > 999)
                len += 3;
            else if (prefix > 99)
                len += 2;
            else if (prefix > 9)
                len++;
        }
    }*/

    len += print_time(msg->ts_nsec, buf ? buf + len : NULL);
    return len;
}

static size_t msg_print_text(const struct printk_log *msg, bool syslog, char *buf, size_t size)
{
    const char *text      = log_text(msg);
    size_t      text_size = msg->text_len;
    size_t      len       = 0;

    do {
        const char *next = memchr(text, '\n', text_size);
        size_t      text_len;

        if (next) {
            text_len = next - text;
            next++;
            text_size -= next - text;
        } else {
            text_len = text_size;
        }

        if (buf) {
            if (print_prefix(msg, syslog, NULL) + text_len + 1 >= size - len) break;

            len += print_prefix(msg, syslog, buf + len);
            memcpy(buf + len, text, text_len);
            len += text_len;
            buf[len++] = '\n';
        } else {
            /* SYSLOG_ACTION_* buffer size only calculation */
            len += print_prefix(msg, syslog, NULL);
            len += text_len;
            len++;
        }

        text = next;
    } while (text);

    return len;
}

#ifdef AOS_COMP_FATFS

static u64    syslog_seq;
static u32    syslog_idx;
static size_t syslog_partial;

static int syslog_print(char *buf, int size)
{
    char              *text;
    struct printk_log *msg;
    int                len = 0;
    unsigned long      flags;

    text = malloc(LOG_LINE_MAX + PREFIX_MAX);
    if (!text) return -ENOMEM;

    while (size > 0) {
        size_t n;
        size_t skip;

        spin_lock_irqsave(&printk_lock, flags);
        if (syslog_seq < log_first_seq) {
            /* messages are gone, move to first one */
            syslog_seq     = log_first_seq;
            syslog_idx     = log_first_idx;
            syslog_partial = 0;
        }
        if (syslog_seq == log_next_seq) {
            spin_unlock_irqrestore(&printk_lock, flags);
            break;
        }

        skip = syslog_partial;
        msg  = log_from_idx(syslog_idx);
        n    = msg_print_text(msg, true, text, LOG_LINE_MAX + PREFIX_MAX);
        // printf("%s %ld syslog_partial[%ld]\n", __func__, n, skip);
        if (n - syslog_partial <= size) {
            /* message fits into buffer, move forward */
            syslog_idx = log_next(syslog_idx);
            syslog_seq++;
            n -= syslog_partial;
            syslog_partial = 0;
        } else if (!len) {
            /* partial read(), remember position */
            n = size;
            syslog_partial += n;
        } else
            n = 0;
        spin_unlock_irqrestore(&printk_lock, flags);

        if (!n) break;

        if (memcpy(buf, text + skip, n)) {
            // if (!len)
            //	len = -EFAULT;
            // break;
        }

        len += n;
        size -= n;
        buf += n;
    }

    free(text);
    return len;
}
#endif

static int syslog_print_all(char *buf, int size, bool clear)
{
    char         *text;
    int           len = 0;
    u64           next_seq;
    u64           seq;
    u32           idx;
    unsigned long flags;

    text = malloc(LOG_LINE_MAX + PREFIX_MAX);
    if (!text) return -ENOMEM;

    spin_lock_irqsave(&printk_lock, flags);
    /*
     * Find first record that fits, including all following records,
     * into the user-provided buffer for this dump.
     */
    seq = clear_seq;
    idx = clear_idx;
    while (seq < log_next_seq) {
        struct printk_log *msg = log_from_idx(idx);

        len += msg_print_text(msg, true, NULL, 0);
        idx = log_next(idx);
        seq++;
    }

    /* move first record forward until length fits into the buffer */
    seq = clear_seq;
    idx = clear_idx;
    while (len > size && seq < log_next_seq) {
        struct printk_log *msg = log_from_idx(idx);

        len -= msg_print_text(msg, true, NULL, 0);
        idx = log_next(idx);
        seq++;
    }

    /* last message fitting into this dump */
    next_seq = log_next_seq;

    len = 0;
    while (len >= 0 && seq < next_seq) {
        struct printk_log *msg = log_from_idx(idx);
        int                textlen;

        textlen = msg_print_text(msg, true, text, LOG_LINE_MAX + PREFIX_MAX);
        if (textlen < 0) {
            len = textlen;
            break;
        }
        idx = log_next(idx);
        seq++;

        spin_unlock_irqrestore(&printk_lock, flags);

        memcpy(buf + len, text, textlen);
        len += textlen;

        spin_lock_irqsave(&printk_lock, flags);

        if (seq < log_first_seq) {
            /* messages are gone, move to next one */
            seq = log_first_seq;
            idx = log_first_idx;
        }
    }

    if (clear) {
        clear_seq = log_next_seq;
        clear_idx = log_next_idx;
    }
    spin_unlock_irqrestore(&printk_lock, flags);

    free(text);
    return len;
}

enum TASK_TYPE {
    TASK_LOG_PRINTF,
    TASK_LOG_SAVE,
    TASK_LOG_STOP,
    TASK_MAX,
};

struct task_attr {
    char  task_name[32];
    char  path[64];
    void *task;
    u32   flag;
    void (*task_handler)(void *arg);
};

static struct task_attr dmeg_task[TASK_MAX];

static void log_printf(void *para)
{
    char *buf;
    int   len = 0;

    buf = malloc(__LOG_BUF_LEN);
    if (!buf) goto fail_exit;

    len = syslog_print_all(buf, __LOG_BUF_LEN, false);

    if (len > 0) {
        *(buf + len) = '\0';
        printf(buf);
    }

    free(buf);
fail_exit:
    rt_thread_delete(printf_thread);
}
#ifdef AOS_COMP_FATFS
static void log_save_to_file(void *para)
{
    char             *buf;
    char              file_name[128];
    int               fd = -1, ret = -1;
    int               len = 0, size = LOG_LINE_MAX + PREFIX_MAX;
    struct task_attr *task = (struct task_attr *)para;

    buf = malloc(LOG_LINE_MAX + PREFIX_MAX);
    if (!buf) goto fail_exit;

    snprintf(file_name, sizeof(file_name), SD_FATFS_MOUNTPOINT "/%s", task->path);

    fd = aos_open(file_name, (O_RDWR | O_CREAT | O_EXCL));
    if (fd < 0) {
        printf("fail to open %s\n", file_name);
        goto exit_log_save;
    }

    while (task->flag) {
        if (sem_wait(&log_wait) != 0) continue;

        if (syslog_seq == log_next_seq) continue;

        len = syslog_print(buf, size);
        // printf("size = %d, len = %d\n", size, len);
        if (len < 0) {
            continue;
        }

        ret = aos_write(fd, buf, len);
        if (ret < 0) {
            printf("fail to write %s\n", file_name);
        }

        aos_sync(fd);
    }

    ret = aos_close(fd);
    if (ret < 0) {
        printf("fail to close %s\n", file_name);
    }

exit_log_save:
    free(buf);
fail_exit:
    aos_task_exit(0);
}
#endif

static void printf_usage(void)
{
    printf("dmesg                  <== print all message info\n");
    printf("dmesg w [file]         <== record log to file\n");
    printf("dmesg s                <== stop record log\n");
}

#define RHINO_CONFIG_USER_PRI_MAX 60

static int create_task(enum TASK_TYPE type)
{
    switch (type) {
    case TASK_LOG_PRINTF: {
        memcpy(dmeg_task[type].task_name, "task_log_printf", sizeof("task_log_printf"));
        dmeg_task[type].task_handler = log_printf;
        break;
    }
#ifdef AOS_COMP_FATFS
    case TASK_LOG_SAVE: {
        memcpy(dmeg_task[type].task_name, "task_log_save", sizeof("task_log_save"));
        dmeg_task[type].task_handler = log_save_to_file;
        dmeg_task[type].flag         = 1;
        break;
    }
#endif
    case TASK_LOG_STOP:
        dmeg_task[TASK_LOG_SAVE].flag = 0;
        return 0;
    default:
        return -1;
    }

    printf_thread = rt_thread_create(dmeg_task[type].task_name, dmeg_task[type].task_handler, NULL,
                                     8192, 1, 10);
    rt_thread_startup(printf_thread);

    return 0;
}

static int dmesg(int argc, char **argv)
{
    int opt = 0;

    if (argc == 1) {
        create_task(TASK_LOG_PRINTF);
        return 0;
    } else if (argc <= 3) {
        opt = *(argv[1]);
        switch (opt) {
        case 'h':
            printf_usage();
            break;
        case 'w':
            memcpy(dmeg_task[TASK_LOG_SAVE].path, argv[2], strlen(argv[2]));
            create_task(TASK_LOG_SAVE);
            break;
        case 's':
            create_task(TASK_LOG_STOP);
            break;
        default:
            printf_usage();
            break;
        }
    } else {
        printf_usage();
    }

    return 0;
}

void dmesg_init(void)
{
    memcpy(dmeg_task[TASK_LOG_SAVE].path, "test1.log", strlen("test1.log"));
    create_task(TASK_LOG_SAVE);
}
MSH_CMD_EXPORT_ALIAS(dmesg, dmesg, dmesg info);
