
#ifndef GLOBUS_DONT_DOCUMENT_INTERNAL

/**
 * @file globus_i_gass_transfer_keyvalue.h
 * @brief GASS Transfer Key Value
 */

#ifndef GLOBUS_GASS_TRANSFER_KEYVALUE_H
#define GLOBUS_GASS_TRANSFER_KEYVALUE_H

#include "globus_list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    char *					key;
    char *					value;
} globus_gass_transfer_keyvalue_t;

char *
globus_i_gass_transfer_keyvalue_lookup(
    globus_list_t **				list,
    char *					key);

void
globus_i_gass_transfer_keyvalue_insert(
    globus_list_t **				list,
    char *					key,
    char *					value);

void
globus_i_gass_transfer_keyvalue_replace(
    globus_list_t **				list,
    char *					key,
    char *					value);

void
globus_i_gass_transfer_keyvalue_destroy(
    globus_list_t **				list);

#ifdef __cplusplus
}
#endif

#endif /* GLOBUS_GASS_INCLUDE_GLOBUS_GASS_TRANSFER_KEYVALUE_H */

#endif /* GLOBUS_DONT_DOCUMENT_INTERNAL */
