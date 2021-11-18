
/**
 * @file attr/destroy.c
 * @brief globus_net_manager_attr_destroy()
 */

#include "globus_net_manager_attr.h"
#include "globus_net_manager.h"

/**
 * @brief Destroy the contents of an attribute
 * @ingroup globus_net_manager_attr
 * @details
 * This function frees the values contained in <b>attr</b> and reinitializes
 * them to NULL. It <em>does not</em> free attr itself.
 *
 * @param[in] attr
 *     Pointer to the attribute to destroy.
 */
void
globus_net_manager_attr_destroy(
    globus_net_manager_attr_t          *attr)
{
    if (attr)
    {
        free(attr->scope);
        free(attr->name);
        free(attr->value);
        *attr = globus_net_manager_null_attr;
    }
}
/* globus_net_manager_attr_destroy() */
