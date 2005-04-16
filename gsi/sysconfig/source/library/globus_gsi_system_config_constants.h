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

#ifndef GLOBUS_DONT_DOCUMENT_INTERNAL
/**
 * @file globus_gsi_system_config_constants.h
 * @author Sam Lang, Sam Meder
 * 
 * $RCSfile$
 * $Revision$
 * $Date$
 */
#endif

#ifndef _GLOBUS_GSI_SYSTEM_CONFIG_CONSTANTS_H_
#define _GLOBUS_GSI_SYSTEM_CONFIG_CONSTANTS_H_

#ifndef EXTERN_C_BEGIN
#    ifdef __cplusplus
#        define EXTERN_C_BEGIN extern "C" {
#        define EXTERN_C_END }
#    else
#        define EXTERN_C_BEGIN
#        define EXTERN_C_END
#    endif
#endif

EXTERN_C_BEGIN

/**
 * @defgroup globus_gsi_sysconfig_datatypes Datatypes
 */

/**
 * GSI System Config Error codes
 * @ingroup globus_gsi_sysconfig_datatypes
 */
typedef enum
{
    /** Success - never used */
    GLOBUS_GSI_SYSCONFIG_ERROR_SUCCESS = 0,
    /** Unable to determine trusted certificates directory */
    GLOBUS_GSI_SYSCONFIG_ERROR_GETTING_CERT_DIR = 1,
    /** Error while generating certificate filename */
    GLOBUS_GSI_SYSCONFIG_ERROR_GETTING_CERT_STRING = 2,
    /** Error while generating private key filename */
    GLOBUS_GSI_SYSCONFIG_ERROR_GETTING_KEY_STRING = 3,
    /** Unable to determine user's home directory */
    GLOBUS_GSI_SYSCONFIG_ERROR_GETTING_HOME_DIR = 4,
    /** System Error -- see underlying error for details */
    GLOBUS_GSI_SYSCONFIG_ERROR_ERRNO = 5,
    /** Unable to determine whether file exists */
    GLOBUS_GSI_SYSCONFIG_ERROR_CHECKING_FILE_EXISTS = 6,
    /** Unable to determine the location of the certificate file */
    GLOBUS_GSI_SYSCONFIG_ERROR_GETTING_CERT_FILENAME = 7,
    /** Unable to determine the location of the proxy file */
    GLOBUS_GSI_SYSCONFIG_ERROR_GETTING_PROXY_FILENAME = 8,
    /** Unable to determine the location of the delegated proxy file */
    GLOBUS_GSI_SYSCONFIG_ERROR_GETTING_DELEG_FILENAME = 9,
    /** Unable to generate a list of CA certificate filenames */
    GLOBUS_GSI_SYSCONFIG_ERROR_GETTING_CA_CERT_FILENAMES = 10,
    /** Error while dircovering the current working directory */
    GLOBUS_GSI_SYSCONFIG_ERROR_GETTING_CWD = 11,
    /** Failed to remove all proxy files */
    GLOBUS_GSI_SYSCONFIG_ERROR_REMOVING_OWNED_FILES = 12,
    /** Unable to determine the location of the grid map file */
    GLOBUS_GSI_SYSCONFIG_ERROR_GETTING_GRIDMAP_FILENAME = 13,
    /** Failure while checking whether the current user is the super user */
    GLOBUS_GSI_SYSCONFIG_ERROR_CHECKING_SUPERUSER = 14,
    /** Error while trying to set file permissions */
    GLOBUS_GSI_SYSCONFIG_ERROR_SETTING_PERMS = 15,
    /** Unable to determine the location of a signing policy file */
    GLOBUS_GSI_SYSCONFIG_ERROR_GETTING_SIGNING_POLICY = 16,
    /** Could not find password entry for user */
    GLOBUS_GSI_SYSCONFIG_ERROR_GETTING_PW_ENTRY = 17,
    /** Failed to locate the authorization callout configuration file */
    GLOBUS_GSI_SYSCONFIG_ERROR_GETTING_AUTHZ_FILENAME = 18,
    /** File is not a regular file */
    GLOBUS_GSI_SYSCONFIG_ERROR_FILE_NOT_REGULAR = 19,
    /** File does not exist */
    GLOBUS_GSI_SYSCONFIG_ERROR_FILE_DOES_NOT_EXIST = 20,
    /** File has incorrect permissions for operation */
    GLOBUS_GSI_SYSCONFIG_ERROR_FILE_BAD_PERMISSIONS = 21,
    /** File is not owned by current user */
    GLOBUS_GSI_SYSCONFIG_ERROR_FILE_NOT_OWNED = 22,
    /** File is a directory */
    GLOBUS_GSI_SYSCONFIG_ERROR_FILE_IS_DIR = 23,
    /** File has zero length */
    GLOBUS_GSI_SYSCONFIG_ERROR_FILE_ZERO_LENGTH = 24,
    /** Invalid argument */
    GLOBUS_GSI_SYSCONFIG_INVALID_ARG = 25,
    /** File has more than one link */
    GLOBUS_GSI_SYSCONFIG_ERROR_FILE_HAS_LINKS = 26,
    /** File has changed in the meantime */
    GLOBUS_GSI_SYSCONFIG_ERROR_FILE_HAS_CHANGED = 27,
    /** Failed to locate the authorization callout library configuration file */
    GLOBUS_GSI_SYSCONFIG_ERROR_GETTING_AUTHZ_LIB_FILENAME = 28,
    /** Failed to locate the gaa configuration file */
    GLOBUS_GSI_SYSCONFIG_ERROR_GETTING_GAA_FILENAME = 29,
    /** Last marker - never used */
    GLOBUS_GSI_SYSCONFIG_ERROR_LAST = 30
}
globus_gsi_sysconfig_error_t;

/**
 * Enumerator used to keep track of input/output types of filenames
 * @ingroup globus_gsi_sysconfig_datatypes
 *
 */
typedef enum
{
    /** The proxy filename is intended for reading (it should already exist) */
    GLOBUS_PROXY_FILE_INPUT,
    /** The proxy filename is intended for writing (it does not need to exist) */
    GLOBUS_PROXY_FILE_OUTPUT
}
globus_gsi_proxy_file_type_t;

EXTERN_C_END

#endif /* _GLOBUS_GSI_SYSTEM_CONFIG_CONSTANTS_H_ */
