/*
 * Copyright 1999-2014 University of Chicago
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "globus_xio_driver.h"
#include "globus_xio_load.h"
#include "globus_common.h"
#include "globus_xio_net_manager_driver.h"

#include "globus_net_manager.h"
#include "globus_net_manager_context.h"

GlobusDebugDefine(GLOBUS_XIO_NET_MANAGER);
GlobusXIODeclareDriver(net_manager);

#define GlobusXIONetManagerDebugPrintf(level, message)                  \
    GlobusDebugPrintf(GLOBUS_XIO_NET_MANAGER, level, message)

#define GlobusXIONetManagerDebugEnter()                                 \
    GlobusXIONetManagerDebugPrintf(                                     \
        GLOBUS_XIO_NET_MANAGER_DEBUG_TRACE,                              \
        ("[%s] Entering\n", _xio_name))

#define GlobusXIONetManagerDebugExit()                                  \
    GlobusXIONetManagerDebugPrintf(                                     \
        GLOBUS_XIO_NET_MANAGER_DEBUG_TRACE,                              \
        ("[%s] Exiting\n", _xio_name))

typedef enum
{
    GLOBUS_XIO_NET_MANAGER_DEBUG_ERROR = 1,
    GLOBUS_XIO_NET_MANAGER_DEBUG_WARNING = 2,
    GLOBUS_XIO_NET_MANAGER_DEBUG_TRACE = 4,
    GLOBUS_XIO_NET_MANAGER_DEBUG_INFO = 8,
} globus_xio_net_manager_debug_levels_t;


static int
globus_l_xio_net_manager_activate();

static int
globus_l_xio_net_manager_deactivate();

#include "version.h"

GlobusXIODefineModule(net_manager) =
{
    "globus_xio_net_manager",
    globus_l_xio_net_manager_activate,
    globus_l_xio_net_manager_deactivate,
    GLOBUS_NULL,
    GLOBUS_NULL,
    &local_version
};

static
void
globus_l_xio_net_manager_open_cb(
    globus_xio_operation_t              op,
    globus_result_t                     result,
    void *                              user_arg)
{
    globus_xio_driver_finished_open(NULL, op, result);
}

static
globus_result_t
globus_l_xio_net_manager_open(
    const globus_xio_contact_t *        contact_info,
    void *                              driver_link,
    void *                              driver_attr,
    globus_xio_operation_t              op)
{
    globus_result_t                     res;
    res = globus_xio_driver_pass_open(
        op, contact_info, globus_l_xio_net_manager_open_cb, NULL);
    return res;
}

static
void
globus_l_xio_net_manager_close_cb(
    globus_xio_operation_t              op,
    globus_result_t                     result,
    void *                              user_arg)
{
    globus_xio_driver_finished_close(op, result);
}

static
globus_result_t
globus_l_xio_net_manager_close(
    void *                              driver_specific_handle,
    void *                              attr,
    globus_xio_operation_t              op)
{
    globus_result_t                     res;
    res = globus_xio_driver_pass_close(
        op, globus_l_xio_net_manager_close_cb, NULL);
    return res;
}

static
void
globus_l_xio_net_manager_read_cb(
    struct globus_i_xio_op_s *          op,
    globus_result_t                     result,
    globus_size_t                       nbytes,
    void *                              user_arg)
{
    globus_xio_driver_finished_read(op, result, nbytes);
}

static
globus_result_t
globus_l_xio_net_manager_read(
    void *                              driver_specific_handle,
    const globus_xio_iovec_t *          iovec,
    int                                 iovec_count,
    globus_xio_operation_t              op)
{
    globus_size_t                       wait_for;
    globus_result_t                     res;

    wait_for = globus_xio_operation_get_wait_for(op);
    res = globus_xio_driver_pass_read(
        op, (globus_xio_iovec_t *)iovec, iovec_count, wait_for,
        globus_l_xio_net_manager_read_cb, NULL);
    return res;
}

static
globus_result_t
globus_l_xio_net_manager_write(
    void *                              driver_specific_handle,
    const globus_xio_iovec_t *          iovec,
    int                                 iovec_count,
    globus_xio_operation_t              op)
{
    globus_result_t                     res;
    globus_size_t                       wait_for;

    wait_for = globus_xio_operation_get_wait_for(op);
    res = globus_xio_driver_pass_write(
        op, (globus_xio_iovec_t *)iovec, iovec_count, wait_for,
        NULL, NULL);

    return res;
}

static
globus_result_t
globus_l_xio_net_manager_init(
    globus_xio_driver_t *               out_driver)
{
    globus_xio_driver_t                 driver;
    globus_result_t                     res;

    res = globus_xio_driver_init(&driver, "net_manager", NULL);
    if(res != GLOBUS_SUCCESS)
    {
        return res;
    }

    globus_xio_driver_set_transform(
        driver,
        globus_l_xio_net_manager_open,
        globus_l_xio_net_manager_close,
        globus_l_xio_net_manager_read,
        globus_l_xio_net_manager_write,
        NULL,
        NULL);

    *out_driver = driver;

    return GLOBUS_SUCCESS;
}

static
void
globus_l_xio_net_manager_destroy(
    globus_xio_driver_t                 driver)
{
    globus_xio_driver_destroy(driver);
}

GlobusXIODefineDriver(
    net_manager,
    globus_l_xio_net_manager_init,
    globus_l_xio_net_manager_destroy);

static
int
globus_l_xio_net_manager_activate(void)
{
    int                                 rc;

    GlobusDebugInit(GLOBUS_XIO_NET_MANAGER, TRACE);

    rc = globus_module_activate(GLOBUS_XIO_MODULE);
    if(rc == GLOBUS_SUCCESS)
    {
        GlobusXIORegisterDriver(net_manager);
    }
    
    return rc;
}

static
int
globus_l_xio_net_manager_deactivate(void)
{
    GlobusXIOUnRegisterDriver(net_manager);
    return globus_module_deactivate(GLOBUS_XIO_MODULE);
}
