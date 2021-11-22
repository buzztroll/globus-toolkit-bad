
#ifndef GLOBUS_DONT_DOCUMENT_INTERNAL

/**
 * @file globus_gsi_authz_error.c
 * @brief Globus GSI Authz Library
 * @author Sam Meder
 */

#include "globus_gsi_authz_constants.h"

char * 
globus_l_gsi_authz_error_strings[GLOBUS_GSI_AUTHZ_ERROR_LAST] =
{
/* 0 */   "Success",
/* 1 */   "Error with system call",
/* 2 */   "Invalid parameter",
/* 3 */   "Callout returned an error"
};

#endif /* GLOBUS_DONT_DOCUMENT_INTERNAL */
