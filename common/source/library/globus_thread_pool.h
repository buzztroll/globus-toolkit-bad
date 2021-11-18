
/**
 * @file globus_thread_pool.h
 * @brief Thread Pooling
 */

/**
 * @defgroup globus_thread_pool Thread Pooling
 * @ingroup globus_common
 * @details
 * The globus_thread_pool interface is an internal module which allows
 * Globus to reuse system threads for different callbacks.
 */

#if !defined(GLOBUS_THREAD_POOL_H)
#define GLOBUS_THREAD_POOL_H 1

#include "globus_common_include.h"
#include "globus_thread.h"

#ifdef __cplusplus
extern "C" {
#endif

int
globus_i_thread_pool_activate(void);

int
globus_i_thread_pool_deactivate(void);

void
globus_i_thread_start(
    globus_thread_func_t                func,
    void *                              user_arg);

int
globus_thread_pool_key_create(  
    globus_thread_key_t *                 key,     
    globus_thread_key_destructor_func_t   func);

/******************************************************************************
                               Module definition
******************************************************************************/
extern globus_module_descriptor_t       globus_i_thread_pool_module;

#define GLOBUS_THREAD_POOL_MODULE (&globus_i_thread_pool_module)

#ifdef __cplusplus
}
#endif
#endif
