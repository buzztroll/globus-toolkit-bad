
/**
 * @file null/globus_net_manager_null.c
 * @brief Null Network Manager Implementation
 */

#include "globus_common.h"
#include "globus_net_manager.h"
#include "version.h"

static
globus_result_t
globus_l_net_manager_null_pre_listen(
    struct globus_net_manager_s        *manager,
    const globus_net_manager_attr_t    *manager_attr_array,
    const char                         *task_id,
    const char                         *transport,
    const globus_net_manager_attr_t    *attr_array,
    globus_net_manager_attr_t         **attr_array_out)
{
    return GLOBUS_SUCCESS;
}
/* globus_l_net_manager_null_pre_listen() */

globus_result_t
globus_l_net_manager_null_post_listen(
    struct globus_net_manager_s        *manager,
    const globus_net_manager_attr_t    *manager_attr_array,
    const char                         *task_id,
    const char                         *transport,
    const char                         *local_contact,
    const globus_net_manager_attr_t    *attr_array,
    char                              **local_contact_out,
    globus_net_manager_attr_t         **attr_array_out)
{
    return GLOBUS_SUCCESS;
}

globus_result_t
globus_l_net_manager_null_end_listen(
    struct globus_net_manager_s        *manager,
    const globus_net_manager_attr_t    *manager_attr_array,
    const char                         *task_id,
    const char                         *transport,
    const char                         *local_contact,
    const globus_net_manager_attr_t    *attr_array)
{
    return GLOBUS_SUCCESS;
}

globus_result_t
globus_l_net_manager_null_pre_accept(
    struct globus_net_manager_s        *manager,
    const globus_net_manager_attr_t    *manager_attr_array,
    const char                         *task_id,
    const char                         *transport,
    const char                         *local_contact,
    const globus_net_manager_attr_t    *attr_array,
    globus_net_manager_attr_t         **attr_array_out)
{
    return GLOBUS_SUCCESS;
}

globus_result_t
globus_l_net_manager_null_post_accept(
    struct globus_net_manager_s        *manager,
    const globus_net_manager_attr_t    *manager_attr_array,
    const char                         *task_id,
    const char                         *transport,
    const char                         *local_contact,
    const char                         *remote_contact,
    const globus_net_manager_attr_t    *attr_array,
    globus_net_manager_attr_t         **attr_array_out)
{
    return GLOBUS_SUCCESS;
}

globus_result_t
globus_l_net_manager_null_pre_connect(
    struct globus_net_manager_s        *manager,
    const globus_net_manager_attr_t    *manager_attr_array,
    const char                         *task_id,
    const char                         *transport,
    const char                         *remote_contact,
    const globus_net_manager_attr_t    *attr_array,
    char                              **remote_contact_out,
    globus_net_manager_attr_t         **attr_array_out)
{
    return GLOBUS_SUCCESS;
}

globus_result_t
globus_l_net_manager_null_post_connect(
    struct globus_net_manager_s        *manager,
    const globus_net_manager_attr_t    *manager_attr_array,
    const char                         *task_id,
    const char                         *transport,
    const char                         *local_contact,
    const char                         *remote_contact,
    const globus_net_manager_attr_t    *attr_array,
    globus_net_manager_attr_t         **attr_array_out)
{
    return GLOBUS_SUCCESS;
}

globus_result_t
globus_l_net_manager_null_pre_close(
    struct globus_net_manager_s        *manager,
    const globus_net_manager_attr_t    *manager_attr_array,
    const char                         *task_id,
    const char                         *transport,
    const char                         *local_contact,
    const char                         *remote_contact,
    const globus_net_manager_attr_t    *attr_array)
{
    return GLOBUS_SUCCESS;
}

globus_result_t
globus_l_net_manager_null_post_close(
    struct globus_net_manager_s        *manager,
    const globus_net_manager_attr_t    *manager_attr_array,
    const char                         *task_id,
    const char                         *transport,
    const char                         *local_contact,
    const char                         *remote_contact,
    const globus_net_manager_attr_t    *attr_array)
{
    return GLOBUS_SUCCESS;
}

static
int
globus_l_net_manager_null_activate(void);

static
int
globus_l_net_manager_null_deactivate(void);

static
globus_net_manager_t                    globus_l_net_manager_null = {
    "null",
    globus_l_net_manager_null_pre_listen,
    globus_l_net_manager_null_post_listen,
    globus_l_net_manager_null_end_listen,
    globus_l_net_manager_null_pre_accept,
    globus_l_net_manager_null_post_accept,
    globus_l_net_manager_null_pre_connect,
    globus_l_net_manager_null_post_connect,
    globus_l_net_manager_null_pre_close,
    globus_l_net_manager_null_post_close
};

GlobusExtensionDefineModule(globus_net_manager_null) = {
    "globus_net_manager_null",
    globus_l_net_manager_null_activate,
    globus_l_net_manager_null_deactivate,
    NULL,
    NULL,
    &local_version
};

static
int
globus_l_net_manager_null_activate(void)
{
    int rc = globus_module_activate(GLOBUS_NET_MANAGER_MODULE);
    if (rc == 0)
    {
        rc = globus_net_manager_register(&globus_l_net_manager_null,
            GlobusExtensionMyModule(globus_net_manager_null));
    }
    return rc;
}

static
int
globus_l_net_manager_null_deactivate(void)
{
    int rc = globus_net_manager_unregister(&globus_l_net_manager_null);
    if (rc == 0)
    {
        rc = globus_module_deactivate(GLOBUS_NET_MANAGER_MODULE);
    }
    return rc;
}
