#ifndef __OSAL_ATOMIC_H__
#define __OSAL_ATOMIC_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef struct {
    volatile int counter;
} osal_atomic;

/**
 * @ingroup osal_atomic
 * @brief read atomic variable
 *
 * @par Description:
 * This API is used to implement the atomic read and return the value read from the input parameter @atomic.
 *
 * @par Support System:
 * linux alios rt_thread.
 */
int osal_atomic_read(osal_atomic *atomic);

/**
 * @ingroup osal_atomic
 * @brief set atomic variable
 *
 * @par Description:
 * This API is used to implement the atomic setting operation.
 *
 * @par Support System:
 * linux alios rt_thread.
 */
void osal_atomic_set(osal_atomic *atomic, int i);

/**
 * @ingroup osal_atomic
 * @brief increment atomic variable and return
 *
 * @par Description:
 * This API is used to implement the atomic self-addition and return the self-addition result.
 *
 * @par Support System:
 * linux alios rt_thread.
 */
int osal_atomic_inc_return(osal_atomic *atomic);

/**
 * @ingroup osal_atomic
 * @brief Adds the given value to the atomic variable and returns the increased result.
 *
 * @par Description:
 * Adds the given value to the atomic variable and returns the increased result.
 *
 * @param atomic  [in]  The atomic to be change.
 * @param count   [in]  Number of atomic variables to be add.
 *
 * @par Support System:
 * rt_thread.
 */
int osal_atomic_add_return(osal_atomic *atomic, unsigned int count);

/**
 * @ingroup osal_atomic
 * @brief decrement atomic variable and return
 *
 * @par Description:
 * This API is used to implement the atomic self-decrement and return the self-decrement result.
 *
 * @par Support System:
 * linux alios rt_thread.
 */
int osal_atomic_dec_return(osal_atomic *atomic);

/**
 * @ingroup osal_atomic
 * @brief increment atomic variable
 *
 * @par Description:
 * This API is used to implement the atomic self-addition.
 *
 * @par Support System:
 * linux alios rt_thread.
 */
void osal_atomic_inc(osal_atomic *atomic);

/**
 * @ingroup osal_atomic
 * @brief Reduce a fixed number of atomic variables.
 *
 * @par Description:
 * Reduce a fixed number of atomic variables
 *
 * @param atomic  [in]  The atomic to be change.
 * @param count   [in]  Number of atomic variables to be reduced.
 *
 * @par Support System:
 * rt_thread.
 */
void osal_atomic_sub(osal_atomic *atomic, unsigned int count);

/**
 * @ingroup osal_atomic
 * @brief decrement atomic variable
 *
 * @par Description:
 * This API is used to implement the atomic self-decrement.
 *
 * @par Support System:
 * linux alios rt_thread.
 */
void osal_atomic_dec(osal_atomic *atomic);

/**
 * @ingroup osal_atomic
 * @brief Add a fixed number of atomic variables.
 *
 * @par Description:
 * Add a fixed number of atomic variables
 *
 * @param atomic  [in]  The atomic to be change.
 * @param count   [in]  Number of atomic variables to be add.
 *
 * @par Support System:
 * alios rt_thread.
 */
void osal_atomic_add(osal_atomic *atomic, unsigned int count);

/**
 * @ingroup osal_atomic
 * @brief Perform a bitwise OR operation on an atomic variable and return the value prior to the operation.
 *
 * @par Description:
 * This API is used to perform a bitwise OR operation on an atomic variable.
 *
 * @par Support System:
 * linux alios rt_thread.
 */
int osal_atomic_fetch_or(unsigned int v, osal_atomic *atomic);

/**
 * @ingroup osal_atomic
 * @brief Perform a bitwise AND operation on an atomic variable and return the value prior to the operation.
 *
 * @par Description:
 * This API is used to perform a bitwise AND operation on an atomic variable.
 *
 * @par Support System:
 * linux alios rt_thread.
 */
int osal_atomic_fetch_and(unsigned int v, osal_atomic *atomic);

/**
 * @ingroup osal_atomic
 * @brief Compare and exchange the value of an atomic variable.
 *
 * @par Description:
 * This API is used to compare and exchange the value of an atomic variable.
 *
 * @par Support System:
 * linux alios rt_thread.
 */
int osal_atomic_cmpxchg(osal_atomic *atomic, unsigned int old, unsigned int new);

/**
 * @ingroup osal_atomic
 * @brief Decrement an atomic variable and test if it is zero.
 *
 * @par Description:
 * This API is used to decrement an atomic variable and test if it is zero.
 *
 * @par Support System:
 * linux alios rt_thread.
 */
int osal_atomic_dec_and_test(osal_atomic *atomic);

/**
 * @ingroup osal_atomic
 * @brief Increment an atomic variable and test if it is zero.
 *
 * @par Description:
 * This API is used to increment an atomic variable and test if it is zero.
 *
 * @par Support System:
 * linux alios rt_thread.
 */
int osal_atomic_inc_and_test(osal_atomic *atomic);

/**
 * @ingroup osal_atomic
 * @brief Increment an atomic variable if it is not zero.
 *
 * @par Description:
 * This API is used to increment an atomic variable if it is not zero.
 *
 * @par Support System:
 * linux alios rt_thread.
 */
int osal_atomic_inc_not_zero(osal_atomic *atomic);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* __OSAL_ATOMIC_H__ */
