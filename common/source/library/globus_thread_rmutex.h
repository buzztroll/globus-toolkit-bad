
/**
 * @file globus_thread_rmutex.h
 * @brief Recursive Mutex
 */

#ifndef GLOBUS_THREAD_RMUTEX_H
#define GLOBUS_THREAD_RMUTEX_H

#include "globus_common_include.h"
#include "globus_thread.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Recursive Mutex
 * @ingroup globus_mutex
 * 
 * @see globus_rmutex_init(), globus_rmutex_destroy(), globus_rmutex_lock(), globus_rmutex_unlock()
 */
typedef struct
{
    globus_mutex_t			mutex;
    globus_cond_t			cond;
    globus_thread_t			thread_id;
    int					level;
    int                                 waiting;
} globus_rmutex_t;

/**
 * @brief Recursive mutex attribute
 * @ingroup globus_mutex
 */
typedef int                             globus_rmutexattr_t;

int
globus_rmutex_init(
    globus_rmutex_t *                   rmutex,
    globus_rmutexattr_t *               attr);

int
globus_rmutex_lock(
    globus_rmutex_t *                   rmutex);

int
globus_rmutex_unlock(
    globus_rmutex_t *                   rmutex);

int
globus_rmutex_destroy(
    globus_rmutex_t *                   rmutex);


#ifdef __cplusplus
}
#endif
#endif /* GLOBUS_THREAD_RMUTEX_H */
