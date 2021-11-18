
#include "globus_net_manager.h"

/**
 * @brief Unregister a network manager
 * @ingroup globus_net_manager
 * @details
 * The globus_net_manager_unregister() function removes this network manager
 * from those which will be called by the network manager interface
 * when network events occur. This is typically called by the network
 * manager when its module is deactivated.
 * @param[in] manager
 *     Manager information to unregister.
 *
 */
globus_result_t
globus_net_manager_unregister(
    globus_net_manager_t               *manager)
{
    globus_extension_registry_remove(
        GLOBUS_NET_MANAGER_REGISTRY, (void *) manager->name);
    
    return GLOBUS_SUCCESS;
}
