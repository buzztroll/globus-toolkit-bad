
#ifndef GLOBUS_DONT_DOCUMENT_INTERNAL
/**
 * @file duplicate_name.c
 * @author Sam Meder
 */
#endif

#include "gssapi_openssl.h"
#include "globus_i_gsi_gss_utils.h"
#include "globus_gsi_gss_constants.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/**
 * @brief Duplicate Name
 * @ingroup globus_gsi_gssapi
 * @details
 * Copy a GSSAPI name.
 *
 * @param minor_status
 * @param src_name
 * @param dest_name
 *
 * @return
 */
OM_uint32 
GSS_CALLCONV gss_duplicate_name(
    OM_uint32 *                         minor_status,
    const gss_name_t                    src_name,
    gss_name_t *                        dest_name)
{
    OM_uint32                           major_status = GSS_S_COMPLETE;

    GLOBUS_I_GSI_GSSAPI_DEBUG_ENTER;

    if (minor_status == NULL)
    {
        return GSS_S_FAILURE;
    }

    *minor_status = GLOBUS_SUCCESS;

    if(src_name == GSS_C_NO_NAME)
    {
        GLOBUS_GSI_GSSAPI_ERROR_RESULT(
            minor_status,
            GLOBUS_GSI_GSSAPI_ERROR_BAD_ARGUMENT,
            ("Null source name"));
        GLOBUS_I_GSI_GSSAPI_DEBUG_EXIT;
        return GSS_S_BAD_NAME;
    }

    if(dest_name == NULL)
    {
        GLOBUS_GSI_GSSAPI_ERROR_RESULT(
            minor_status,
            GLOBUS_GSI_GSSAPI_ERROR_BAD_ARGUMENT,
            ("Null destination name"));
        GLOBUS_I_GSI_GSSAPI_DEBUG_EXIT;
        return GSS_S_BAD_NAME;
    }

    major_status =  globus_i_gsi_gss_copy_name_to_name(minor_status,
                                                       dest_name,
                                                       src_name);
    GLOBUS_I_GSI_GSSAPI_DEBUG_EXIT;
    return major_status;
}
