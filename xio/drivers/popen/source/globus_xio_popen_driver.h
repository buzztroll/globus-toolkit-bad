
#ifndef GLOBUS_XIO_POPEN_DRIVER_INCLUDE
#define GLOBUS_XIO_POPEN_DRIVER_INCLUDE
#include "globus_xio_system.h"

typedef void
(*globus_xio_popen_preexec_func_t)(
    globus_xio_handle_t                 handle,
    void *                              user_arg);

typedef enum
{
    GLOBUS_XIO_POPEN_SET_PROGRAM,
    GLOBUS_XIO_POPEN_SET_PASS_ENV,
    GLOBUS_XIO_POPEN_SET_PREEXEC_FUNC,
    GLOBUS_XIO_POPEN_SET_BLOCKING_IO,
    GLOBUS_XIO_POPEN_SET_CHILD_ENV,
    GLOBUS_XIO_POPEN_SET_IGNORE_ERRORS
} globus_xio_popen_attr_cmd_t;

#endif
