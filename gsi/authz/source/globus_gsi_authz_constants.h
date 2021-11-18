
#ifndef GLOBUS_GSI_AUTHZ_CONSTANTS_H
#define GLOBUS_GSI_AUTHZ_CONSTANTS_H

/**
 * @file globus_gsi_authz_constants.h
 * @brief Globus GSI Authorization Library Constants
 */

/**
 * @defgroup globus_gsi_authz_constants GSI Credential Constants
 * @ingroup globus_gsi_authz
 */
/**
 * GSI Authz Error codes
 * @ingroup globus_gsi_authz_constants
 */
typedef enum
{

    /** Success */
    GLOBUS_GSI_AUTHZ_ERROR_SUCCESS = 0,
    /** Error with system call */
    GLOBUS_GSI_AUTHZ_ERROR_ERRNO = 1,
    /** Invalid parameter */
    GLOBUS_GSI_AUTHZ_ERROR_BAD_PARAMETER = 2,
    /** Callout returned an error */
    GLOBUS_GSI_AUTHZ_ERROR_CALLOUT = 3,
    GLOBUS_GSI_AUTHZ_ERROR_LAST = 4
} globus_gsi_authz_error_t;

#endif
