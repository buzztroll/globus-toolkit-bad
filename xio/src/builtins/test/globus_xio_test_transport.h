#if !defined GLOBUS_XIO_TEST_TRANSPORT_H
#define GLOBUS_XIO_TEST_TRANSPORT_H 1

#include "globus_common.h"

typedef enum globus_xio_test_failure_e
{
    GLOBUS_XIO_TEST_FAIL_PASS_OPEN,
    GLOBUS_XIO_TEST_FAIL_FINISH_OPEN,
    GLOBUS_XIO_TEST_FAIL_PASS_CLOSE,
    GLOBUS_XIO_TEST_FAIL_FINISH_CLOSE,
    GLOBUS_XIO_TEST_FAIL_PASS_READ,
    GLOBUS_XIO_TEST_FAIL_FINISH_READ,
    GLOBUS_XIO_TEST_FAIL_PASS_WRITE,
    GLOBUS_XIO_TEST_FAIL_FINISH_WRITE,
} globus_xio_test_failure_t;

enum
{
    GLOBUS_XIO_TEST_SET_INLINE,
    GLOBUS_XIO_TEST_SET_FAILURES,
    GLOBUS_XIO_TEST_SET_USECS,
    GLOBUS_XIO_TEST_READ_EOF_BYTES,
    GLOBUS_XIO_TEST_CHUNK_SIZE,
};

extern globus_module_descriptor_t              globus_i_xio_test_module;

#define GLOBUS_XIO_TEST_TRANSPORT_DRIVER_MODULE &globus_i_xio_test_module

globus_xio_driver_t
globus_xio_driver_test_transport_get_driver();

#endif
