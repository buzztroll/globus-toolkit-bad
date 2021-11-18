
/**
 * @file globus_thread_common.h
 * @brief Common Thread Interface
 */

#ifndef GLOBUS_THREAD_COMMON_H
#define GLOBUS_THREAD_COMMON_H

#include "globus_common_include.h"
#include "globus_module.h"
#include "globus_callback.h"


#ifdef __cplusplus
extern "C" {
#endif

extern globus_module_descriptor_t       globus_i_thread_common_module;

#define GLOBUS_THREAD_COMMON_MODULE     (&globus_i_thread_common_module)

typedef int                                   globus_thread_callback_index_t;

/* function prototypes */
typedef
void
(*globus_thread_blocking_func_t)(
    globus_thread_callback_index_t      ndx,
    globus_callback_space_t             space,
    void *                              user_args);

#define globus_thread_blocking_callback_push(f, u, i)                       \
    globus_thread_blocking_space_callback_push(                             \
        (f), (u), GLOBUS_CALLBACK_GLOBAL_SPACE, (i))
        
int
globus_thread_blocking_space_callback_push(
    globus_thread_blocking_func_t       func,
    void *                              user_args,
    globus_callback_space_t             space,
    globus_thread_callback_index_t *    i);

int
globus_thread_blocking_callback_pop(
    globus_thread_callback_index_t *    i);

int 
globus_thread_blocking_callback_enable(
    globus_thread_callback_index_t *    i);


int 
globus_thread_blocking_callback_disable(
    globus_thread_callback_index_t *    i);

#define globus_thread_blocking_will_block()                             \
    globus_thread_blocking_space_will_block(GLOBUS_CALLBACK_GLOBAL_SPACE)

int 
globus_thread_blocking_space_will_block(
    globus_callback_space_t             blocking_space);

void
globus_thread_blocking_reset();

void thread_print(char * s, ...);


#ifdef __cplusplus
}
#endif

#endif
