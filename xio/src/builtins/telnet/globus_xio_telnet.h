
/**
 * @file globus_xio_telnet.h
 * @brief XIO Telnet Driver
 */

#if !defined GLOBUS_XIO_TELNET_H
#define GLOBUS_XIO_TELNET_H 1

#include "globus_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum globus_xio_telnet_attr_type_e
{
    GLOBUS_XIO_TELNET_FORCE_SERVER,
    GLOBUS_XIO_TELNET_BUFFER,
    GLOBUS_XIO_TELNET_ALLOW_BINARY,
} globus_xio_telnet_attr_type_t;

#ifdef __cplusplus
}
#endif

#endif /* GLOBUS_XIO_TELNET_H */
