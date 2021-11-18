
#ifndef GLOBUS_DONT_DOCUMENT_INTERNAL
/**
 * @file release_buffer.c
 * @author Sam Lang, Sam Meder
 */
#endif

#include "gssapi_openssl.h"
#include "globus_i_gsi_gss_utils.h"

/**
 * @brief Release Buffer
 * @ingroup globus_gsi_gssapi
 * @details 
 * @param minor_status
 * @param buffer
 * @retval GSS_S_COMPLETE Success
 */
OM_uint32 
GSS_CALLCONV gss_release_buffer(
    OM_uint32 *                         minor_status,
    gss_buffer_t                        buffer)
{
    OM_uint32                           major_status = GSS_S_COMPLETE;

    GLOBUS_I_GSI_GSSAPI_DEBUG_ENTER;

    *minor_status = (OM_uint32) GLOBUS_SUCCESS;

    if (buffer == NULL || buffer == GSS_C_NO_BUFFER) {
        goto exit;
    }

    if (buffer->value && buffer->length) {
        free(buffer->value);
    }

    buffer->length = (size_t) 0 ;
    buffer->value = NULL;

 exit:

    GLOBUS_I_GSI_GSSAPI_DEBUG_EXIT;
    return major_status;

} 
/* gss_release_buffer */
