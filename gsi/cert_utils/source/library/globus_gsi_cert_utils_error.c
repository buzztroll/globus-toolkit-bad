#ifndef GLOBUS_DONT_DOCUMENT_INTERNAL
/**
 * @file globus_gsi_cert_utils_error.c
 * Globus GSI Cert Utils Library
 * @author Sam Lang, Sam Meder
 *
 * $RCSfile$
 * $Revision$
 * $Date$
 */
#endif

#include "globus_i_gsi_cert_utils.h"
#include "globus_gsi_cert_utils_constants.h"

#ifndef GLOBUS_DONT_DOCUMENT_INTERNAL

char * 
globus_l_gsi_cert_utils_error_strings[GLOBUS_GSI_CERT_UTILS_ERROR_LAST] =
{

/* 0 */   "Success",
/* 1 */   "Error getting name entry of subject",
/* 2 */   "Error copying subject name of cert",
/* 3 */   "Error getting the Common Name of the Cert's Subject",
/* 4 */   "Error adding Common Name to the Cert's Subject"
};

/* ERROR FUNCTIONS */

globus_result_t
globus_i_gsi_cert_utils_openssl_error_result(
    int                                 error_type,
    const char *                        filename,
    const char *                        function_name,
    int                                 line_number,
    const char *                        long_desc)
{
    globus_object_t *                   error_object;
    globus_result_t                     result;

    static char *                       _function_name_ =
        "globus_i_gsi_cert_utils_openssl_error_result";
    
    GLOBUS_I_GSI_CERT_UTILS_DEBUG_ENTER;

    error_object = 
        globus_error_wrap_openssl_error(
            GLOBUS_GSI_CERT_UTILS_MODULE,
            error_type,
            "%s:%d: %s: %s",
            filename,
            line_number,
            function_name,
            globus_l_gsi_cert_utils_error_strings[error_type]);    

    globus_error_set_long_desc(error_object, long_desc);

    result = globus_error_put(error_object);
    
    GLOBUS_I_GSI_CERT_UTILS_DEBUG_EXIT;

    return result;
}

globus_result_t
globus_i_gsi_cert_utils_error_result(
    int                                 error_type,
    const char *                        filename,
    const char *                        function_name,
    int                                 line_number,
    const char *                        long_desc)
{
    globus_object_t *                   error_object;
    globus_result_t                     result;

    static char *                       _function_name_ =
        "globus_i_gsi_cert_utils_error_result";

    GLOBUS_I_GSI_CERT_UTILS_DEBUG_ENTER;

    error_object = globus_error_construct_error(
        GLOBUS_GSI_CERT_UTILS_MODULE,
        NULL,
        error_type,
        "%s:%d: %s: %s",
        filename, line_number, function_name, 
        globus_l_gsi_cert_utils_error_strings[error_type]);

    globus_error_set_long_desc(error_object, long_desc);

    result = globus_error_put(error_object);

    GLOBUS_I_GSI_CERT_UTILS_DEBUG_EXIT;

    return result;
}

globus_result_t
globus_i_gsi_cert_utils_error_chain_result(
    globus_result_t                     chain_result,
    int                                 error_type,
    const char *                        filename,
    const char *                        function_name,
    int                                 line_number,
    const char *                        long_desc)
{
    globus_object_t *                   error_object;
    globus_result_t                     result;

    static char *                       _function_name_ =
        "globus_i_gsi_cert_utilsential_error_chain_result";
    
    GLOBUS_I_GSI_CERT_UTILS_DEBUG_ENTER;
    
    error_object =
        globus_error_construct_error(
            GLOBUS_GSI_CERT_UTILS_MODULE,
            globus_error_get(chain_result),
            error_type,
            "%s:%d: %s: %s",
            filename, line_number, function_name, 
            globus_l_gsi_cert_utils_error_strings[error_type]);

    globus_error_set_long_desc(error_object, long_desc);

    result = globus_error_put(error_object);

    GLOBUS_I_GSI_CERT_UTILS_DEBUG_EXIT;

    return result;
}

#endif /* GLOBUS_DONT_DOCUMENT_INTERNAL */
