#ifndef __OSAL_TIMER_H__
#define __OSAL_TIMER_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef struct {
    void *timer;
    void (*handler)(unsigned long);
    unsigned long data;    // data for handler
    unsigned int interval; // timer timing duration, unit: ms.
} osal_timer;

typedef struct {
    long tv_sec;
    long tv_usec;
} osal_timeval;

/* Return values for the timer callback function */
typedef enum {
    OSAL_HRTIMER_NORESTART, /* < The timer will not be restarted. */
    OSAL_HRTIMER_RESTART    /* < The timer must be restarted. */
} osal_hrtimer_restart;

/* hrtimer struct */
typedef struct osal_hrtimer {
    void *timer;
    osal_hrtimer_restart (*handler)(void *timer);
    unsigned long interval; /* Unit ms */
} osal_hrtimer;

/**
 * @ingroup osal_timer
 * @brief Initialize the timer.
 *
 * @par Description:
 * Initialize the timer.
 *
 * @param timer [in/out] The timer to be initialized.
 * Assign values to timer->handler and timer->data before calling this interface,
 * because you will not be able to change them after that.
 *
 * @attention
 * When mod exit, must call osal_timer_destroy to free the timer, otherwise will lead to memory leak;
 *
 * @return OSAL_SUCCESS/OSAL_FAILURE
 *
 * @par Support System:
 * linux alios rt_thread.
 */
int osal_timer_init(osal_timer *timer);

/**
 * @ingroup osal_timer
 * @brief start a timer.
 *
 * @par Description:
 * start a timer.
 *
 * @attention
 * The kernel will do a ->function(@timer) callback from the timer interrupt at the ->expires point in the future.
 * The timer's ->expires, ->function fields must be set prior calling this function.
 * Timers with an ->expires field in the past will be executed in the next timer tick.
 *
 * @param timer [in] the timer to be start.
 *
 * @return OSAL_SUCCESS/OSAL_FAILURE
 *
 * @par Support System:
 * linux alios rt_thread.
 */
int osal_timer_start(osal_timer *timer);

/**
 * @ingroup osal_timer
 * @brief modify a timer's timeout.
 *
 * @par Description:
 * modify a timer's timeout. This API is a more efficient way to update the expire field of an active timer
 * (if the timer is inactive it will be activated).
 *
 * @param timer [in] the timer to be modified.
 * @param interval [in] new timeout, unit: ms.
 *
 * @return OSAL_SUCCESS/OSAL_FAILURE
 *
 * @par Support System:
 * linux alios rt_thread.
 */
int osal_timer_mod(osal_timer *timer, unsigned int interval);

/**
 * @ingroup osal_timer
 * @brief Start a timer on a particular CPU.
 *
 * @par Description:
 * Start a timer on a particular CPU.
 *
 * @param timer [in] the timer to be added.
 * @param cpu [in] the CPU to start it on.
 *
 * @return OSAL_SUCCESS/OSAL_FAILURE
 *
 * @par Support System:
 * linux.
 */
int osal_timer_start_on(osal_timer *timer, unsigned long delay, int cpu);

/**
 * @ingroup osal_timer
 * @brief Deactivate a timer.
 *
 * @par Description:
 * Deactivate a timer. This works on both active and inactive timers.
 *
 * @param timer [in] the timer to be deactivated.
 *
 * @retval 1 stop success, timer is pengding. (Only Linux is supported return 1)
 * @retval OSAL_SUCCESS stop success, timer is already stoped.
 * @retval OSAL_FAILURE
 *
 * @par Support System:
 * linux alios rt_thread.
 */
int osal_timer_stop(osal_timer *timer);

/**
 * @ingroup osal_timer
 * @brief destroy the timer.
 *
 * @par Description:
 * destroy the timer.
 *
 * @param timer [in] The timer to be destroyed.
 *
 * @attention
 * When mod exit, this function must be called to free the timer, otherwise will lead to memory leak;
 *
 * @return OSAL_SUCCESS/OSAL_FAILURE
 *
 * @par Support System:
 * linux alios rt_thread.
 */
int osal_timer_destroy(osal_timer *timer);

/**
 * @ingroup osal_timer
 * @brief get_private_data by timer.
 *
 * @par Description:
 * The parameters of the timer callback function cannot be directly used.
 * You need to invoke this interface in the callback function to obtain the parameters that can be used.
 *
 * @param sys_data [in] Parameters passed to the callback function.
 *
 * @return Returns the parameters that can be used directly.
 *
 * @par Support System:
 * linux rt_thread alios.
 */
unsigned long osal_timer_get_private_data(const void *sys_data);

/**
 * @ingroup osal_timer
 * @brief deactivate a timer and wait for the handler to finish.
 *
 * @par Description:
 * This function only differs from del_timer() on SMP,
 * besides deactivating the timer it also makes sure the handler has finished executing on other CPUs.
 *
 * @attention
 * Synchronization rules: Callers must prevent restarting of the timer, otherwise this function is meaningless.
 * It must not be called from interrupt contexts unless the timer is an irqsafe one.
 * The caller must not hold locks which would prevent completion of the timer's handler.
 * The timer's handler must not call add_timer_on().
 * Upon exit the timer is not queued and the handler is not running on any CPU.
 *
 * @return OSAL_SUCCESS/OSAL_FAILURE
 *
 * @par Support System:
 * linux rt_thread alios.
 */
int osal_timer_destroy_sync(osal_timer *timer);

/**
 * @ingroup osal_timer
 * @brief Obtain system time in nanoseconds.
 *
 * @par Description:
 * Obtain system time in nanoseconds.
 *
 * @return returns current time in nanoseconds.
 *
 * @par Support System:
 * linux rt_thread.
 */
unsigned long long osal_sched_clock(void);

/**
 * @ingroup osal_timer
 * @brief Obtain the number of jiffies(in linux).
 *
 * @par Description:
 * Obtain the number of jiffies(in linux).
 *
 * @return Return the number of jiffies(in linux).
 *
 * @par Support System:
 * linux rt_thread.
 */
unsigned long long osal_get_jiffies(void);

/**
 * @ingroup osal_timer
 * @brief Convert milliseconds to Ticks/jiffies.
 *
 * @par Description:
 * Convert milliseconds to Ticks/jiffies.
 *
 * @param m [in] The time to be converted. Unit: ms.
 * @return Ticks/jiffies after conversion
 *
 * @par Support System:
 * linux alios rt_thread.
 */
unsigned long osal_msecs_to_jiffies(const unsigned int m);

/**
 * @ingroup osal_timer
 * @brief Convert Ticks/jiffies to milliseconds.
 *
 * @par Description:
 * Convert Ticks/jiffies to milliseconds.
 *
 * @return Milliseconds obtained through the conversion,
 * if value > 0xFFFFFFFF, the value will be 0xFFFFFFFF.
 *
 * @param n [in] Number of Ticks/jiffies be converted.
 *
 * @par Support System:
 * linux alios rt_thread.
 */
unsigned int osal_jiffies_to_msecs(const unsigned int n);

/**
 * @ingroup osal_timer
 * @brief Obtaining the Current System Kernel Time.
 *
 * @par Description:
 * Obtaining the Current System Kernel Time.
 *
 * @param tv [out] Obtained Current System Kernel Time.
 *
 * @par Support System:
 * linux alios rt_thread.
 */
void osal_gettimeofday(osal_timeval *tv);

/**
 * @ingroup osal_timer
 * @brief Initialize the hrtimer.
 *
 * @par Description:
 * Initialize the hrtimer.
 *
 * @param hrtimer [in/out] The hrtimer to be initialized.
 * Assign values to hrtimer->handler and hrtimer->data before calling this interface,
 * because you will not be able to change them after that.
 *
 * @attention
 * When mod exit, must call osal_hrtimer_destroy to free the hrtimer, otherwise will lead to memory leak;
 *
 * @return OSAL_SUCCESS/OSAL_FAILURE
 *
 * @par Support System:
 * linux alios rt_thread.
 */
int osal_hrtimer_create(osal_hrtimer *hrtimer);

/**
 * @ingroup osal_timer
 * @brief start a hrtimer.
 *
 * @par Description:
 * start a hrtimer.
 *
 * @attention
 * The kernel will do a ->function(@hrtimer) callback from the hrtimer interrupt at the ->expires point in the future.
 * The hrtimer's ->expires, ->function fields must be set prior calling this function.
 * Timers with an ->expires field in the past will be executed in the next hrtimer tick.
 *
 * @param hrtimer [in] the hrtimer to be start.
 *
 * @return OSAL_SUCCESS/OSAL_FAILURE
 *
 * @par Support System:
 * linux alios rt_thread.
 */
int osal_hrtimer_start(osal_hrtimer *hrtimer);

/**
 * @ingroup osal_timer
 * @brief destroy the hrtimer.
 *
 * @par Description:
 * destroy the hrtimer.
 *
 * @param hrtimer [in] The hrtimer to be destroyed.
 *
 * @attention
 * When mod exit, this function must be called to free the hrtimer, otherwise will lead to memory leak;
 *
 * @return OSAL_SUCCESS/OSAL_FAILURE
 *
 * @par Support System:
 * linux alios rt_thread.
 */
int osal_hrtimer_destroy(osal_hrtimer *hrtimer);

/**
 * @ingroup osal_timer
 * @brief Obtaining the current system monotonic time
 *
 * @par Description:
 * Obtaining the current system monotonic time.
 *
 * @param tv [out] Obtained Current System monotonic Time.
 *
 * @par Support System:
 * linux
 */
void osal_time_get_ts64(osal_timeval *tv);

/**
 * @ingroup osal_timer
 * @brief Obtaining the current system real time
 *
 * @par Description:
 * Obtaining the current system v time.
 *
 * @param tv [out] Obtained Current System real Time.
 *
 * @par Support System:
 * linux
 */
void osal_time_get_real_ts64(osal_timeval *tv);

/**
 * @ingroup osal_timer
 * @brief Obtaining the Current System boottime Time.
 *
 * @par Description:
 * Obtaining the Current System boottime Time.
 *
 * @param tv [out] Obtained Current System boottime Time.
 *
 * @par Support System:
 * linux
 */
void osal_time_get_boottime_ts64(osal_timeval *tv);

/**
 * @ingroup osal_timer
 * @brief Obtaining the Current System monotonic-raw Time.
 *
 * @par Description:
 * Obtaining the Current System monotonic-raw Time.
 *
 * @param tv [out] Obtained Current System monotonic-raw Time.
 *
 * @par Support System:
 * linux
 */
void osal_time_get_raw_ts64(osal_timeval *tv);

/**
 * @ingroup osal_timer
 * @brief Obtain two time differences.
 *
 * @par Description:
 * Obtain two time differences.
 *
 * @param end [in] End Time.
 * @param start [in] Start Time.
 *
 * @return Time differences.
 *
 * @par Support System:
 * linux
 */
long osal_timeval_sub(osal_timeval end, osal_timeval start);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* __OSAL_TIMER_H__ */
