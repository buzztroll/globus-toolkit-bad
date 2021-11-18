
#include "globus_net_manager.h"

/**
 * @brief Register a network manager
 * @ingroup globus_net_manager
 * @details
 * The globus_net_manager_register() function adds this network manager
 * to those which will be called by the network manager interface
 * when network events occur. This is typically called by the network
 * manager when its module is activated.
 * @param[in] manager
 *     Manager information to register.
 * @param[in] module
 *     Module descriptor to register.  Use GlobusExtensionMyModule(module name)
 *
 */
globus_result_t
globus_net_manager_register(
    globus_net_manager_t               *manager,
    globus_module_descriptor_t         *module)
{
    int                                 rc;
    globus_result_t                     result = GLOBUS_SUCCESS;
    
    rc = globus_extension_registry_add(
        GLOBUS_NET_MANAGER_REGISTRY,
        (void *) manager->name,
        module,
        (void *) manager);
    if(rc)
    {
        result = GlobusNetManagerErrorInit(
            manager->name, "Failed adding to registry.");
    }

    return result;
}
