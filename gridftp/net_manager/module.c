
#include "globus_net_manager.h"
#ifdef ENABLE_PYTHON
#include "globus_net_manager_python.h"
#endif
#include "globus_net_manager_logging.h"

#include "version.h"

static
int
globus_l_net_manager_activate(void)
{
    globus_module_activate(GLOBUS_COMMON_MODULE);
#ifdef ENABLE_PYTHON
    globus_extension_register_builtin(
        "globus_net_manager_python",
        &globus_net_manager_python_module);
#endif
    globus_extension_register_builtin(
        "globus_net_manager_logging",
        &globus_net_manager_logging_module);
    return GLOBUS_SUCCESS;
}

static
int
globus_l_net_manager_deactivate(void)
{
#ifdef ENABLE_PYTHON
    globus_extension_unregister_builtin("globus_net_manager_python");
#endif
    globus_extension_unregister_builtin("globus_net_manager_logging");

    return globus_module_deactivate(GLOBUS_COMMON_MODULE);
}

globus_extension_registry_t             globus_i_net_manager_registry;

globus_module_descriptor_t              globus_i_net_manager_module =
{
    "globus_net_manager",
    globus_l_net_manager_activate,
    globus_l_net_manager_deactivate,
    NULL,
    NULL,
    &local_version
};
