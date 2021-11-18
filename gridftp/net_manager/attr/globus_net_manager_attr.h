#ifndef GLOBUS_NET_MANAGER_ATTR_H
#define GLOBUS_NET_MANAGER_ATTR_H 1

#include "globus_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file globus_net_manager_attr.h
 * @brief Globus Network Manager Attributes
 */

/**
 * @brief Net Manager Attribute Management Functions
 * @defgroup globus_net_manager_attr Attributes
 * @ingroup globus_net_manager
 */
/**
 * @brief Net Manager Attributes
 * @ingroup globus_net_manager_attr
 * @details
 * The globus_net_manager_attr_t structure defines a scoped (attribute, value)
 * tuple. The scope in most cases is either the name of the transport driver
 * or the "globus_net_manager" scope, for attributes specific to the network
 * manager implementation.
 */
typedef struct globus_net_manager_attr_s
{
    /** Scope of the attribute */  
    char *                              scope;
    /** Name of the attribute */
    char *                              name;
    /** Value of the attribute */
    char *                              value;
}
globus_net_manager_attr_t;

/** 
 * @brief End of attribute array static initializer
 * @ingroup globus_net_manager_attr
 * @hideinitializer
 */
#define GLOBUS_NET_MANAGER_NULL_ATTR { NULL, NULL, NULL }

extern const globus_net_manager_attr_t globus_net_manager_null_attr;

globus_result_t
globus_net_manager_attr_init(
    globus_net_manager_attr_t          *attr,
    const char                         *scope,
    const char                         *name,
    const char                         *value);

void
globus_net_manager_attr_destroy(
    globus_net_manager_attr_t          *attr);


globus_result_t
globus_net_manager_attr_array_from_string(
    globus_net_manager_attr_t         **attr,
    const char                         *scope,
    const char                         *attr_string);

globus_result_t
globus_net_manager_attr_array_copy(
    globus_net_manager_attr_t         **dest_array,
    const globus_net_manager_attr_t    *src_array);

void
globus_net_manager_attr_array_delete(
    globus_net_manager_attr_t          *attrs);

#ifdef __cplusplus
}
#endif

#endif /* #define GLOBUS_NET_MANAGER_ATTR_H */
