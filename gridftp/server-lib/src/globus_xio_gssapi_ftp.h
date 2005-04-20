/*
 * Portions of this file Copyright 1999-2005 University of Chicago
 * Portions of this file Copyright 1999-2005 The University of Southern California.
 *
 * This file or a portion of this file is licensed under the
 * terms of the Globus Toolkit Public License, found at
 * http://www.globus.org/toolkit/download/license.html.
 * If you redistribute this file, with or without
 * modifications, you must include this notice in the file.
 */

#if !defined GLOBUS_XIO_DRIVER_GSSAPI_FTP_H
#define GLOBUS_XIO_DRIVER_GSSAPI_FTP_H 1

#include "globus_common.h"
#include "globus_error_gssapi.h"

typedef enum
{
    GLOBUS_XIO_GSSAPI_ATTR_TYPE_SUBJECT,
    GLOBUS_XIO_GSSAPI_ATTR_TYPE_START_STATE,
    GLOBUS_XIO_GSSAPI_ATTR_TYPE_ENCRYPT,
    GLOBUS_XIO_GSSAPI_ATTR_TYPE_ALLOW_CLEAR,
    GLOBUS_XIO_GSSAPI_ATTR_TYPE_FORCE_SERVER
} globus_xio_gssapi_attr_type_t;

typedef enum globus_xio_gssapi_handle_cntl_type_e
{
    GLOBUS_XIO_DRIVER_GSSAPI_FTP_GET_AUTH
} globus_xio_gssapi_handle_cntl_type_t;

enum
{
    GLOBUS_XIO_GSSAPI_FTP_BAD_PARAMETER,
    GLOBUS_XIO_GSSAPI_FTP_OUTSTANDING_OP,
    GLOBUS_XIO_GSSAPI_FTP_ERROR_ENCODING,
    GLOBUS_XIO_GSSAPI_FTP_ERROR_ALLOC,
    GLOBUS_XIO_GSSAPI_FTP_ERROR_AUTH,
    GLOBUS_XIO_GSSAPI_FTP_ERROR_QUIT
};

enum
{
    GLOBUS_XIO_GSSAPI_FTP_SECURE,
    GLOBUS_XIO_GSSAPI_FTP_CLEAR,
    GLOBUS_XIO_GSSAPI_FTP_NONE
};

#endif
