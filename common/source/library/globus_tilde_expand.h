
/**
 * @file globus_tilde_expand.h
 * @brief Home Directory ~ expansion
 */

#ifndef GLOBUS_TILDE_EXPAND_H
#define GLOBUS_TILDE_EXPAND_H 1
 
#include "globus_types.h"
 
#ifdef __cplusplus
extern "C" {
#endif
 
/** @brief Expand ~
 * @ingroup globus_common
 * @hideinitializer
 */
#define GLOBUS_TILDE_EXPAND             2UL
/** @brief Expand ~user
 * @ingroup globus_common
 * @hideinitializer
 */
#define GLOBUS_TILDE_USER_EXPAND        4UL

int
globus_tilde_expand(
    unsigned long options,
    globus_bool_t url_form,
    char *inpath,
    char **outpath);

#ifdef __cplusplus
}
#endif

#endif /* GLOBUS_TILDE_EXPAND_H */
