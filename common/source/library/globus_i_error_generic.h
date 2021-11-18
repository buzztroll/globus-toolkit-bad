
#ifndef GLOBUS_DONT_DOCUMENT_INTERNAL

#ifndef GLOBUS_I_ERROR_GENERIC_H
#define GLOBUS_I_ERROR_GENERIC_H

/**
 * @file globus_i_error_generic.h
 * @brief Globus Generic Error
 */

#include "globus_common_include.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Generic Error object instance data definition
 * @ingroup globus_generic_error_object
 * @internal
 *
 * This structure contains all of the data associated with a Globus
 * Generic Error.
 *
 * @see globus_error_construct_error(),
 *      globus_error_initialize_error(),
 *      globus_l_error_free_globus()
 */

typedef struct globus_l_error_data_s
{
    /** the error type */
    int                                 type;
    /** the short error description */
    char *                              short_desc;
    /** the long error description */
    char *                              long_desc;
    
    /* these are static strings, do NOT free them */
    const char *                        file;
    const char *                        func;
    int                                 line;
}
globus_l_error_data_t;

#ifdef __cplusplus
}
#endif

#endif /* GLOBUS_I_ERROR_GENERIC_H */

#endif /* GLOBUS_DONT_DOCUMENT_INTERNAL */
