#include "globus_xio_load.h"
#include "globus_i_xio.h"

/* provides local_extensions */
#include "extensions.h"

globus_extension_registry_t             globus_i_xio_driver_registry;

int
globus_i_xio_load_init(void)
{
    GlobusXIOName(globus_i_xio_load_init);
    
    GlobusXIODebugInternalEnter();
    
    globus_extension_register_builtins(local_extensions);
    
    GlobusXIODebugInternalExit();
    return GLOBUS_SUCCESS;
}

int
globus_i_xio_load_destroy(void)
{
    GlobusXIOName(globus_i_xio_load_destroy);
    
    GlobusXIODebugInternalEnter();
    
    globus_extension_unregister_builtins(local_extensions);
    
    GlobusXIODebugInternalExit();
    return GLOBUS_SUCCESS;
}

globus_result_t
globus_xio_driver_load(
    const char *                        driver_name,
    globus_xio_driver_t *               out_driver)
{
    char                                buf[256];
    globus_extension_handle_t           handle;
    globus_bool_t                       activated = GLOBUS_FALSE;
    globus_result_t                     result;
    globus_xio_driver_hook_t *          hook;
    GlobusXIOName(globus_xio_driver_load);
    
    GlobusXIODebugEnter();
    
    if(!driver_name)
    {
        result = GlobusXIOErrorParameter("driver_name");
        goto error_param;
    }
    if(!out_driver)
    {
        result = GlobusXIOErrorParameter("out_driver");
        goto error_param;
    }
    
    hook = (globus_xio_driver_hook_t *) globus_extension_lookup(
        &handle, GLOBUS_XIO_DRIVER_REGISTRY, driver_name);
    if(!hook)
    {
        snprintf(buf, 256, GLOBUS_XIO_EXTENSION_FORMAT, driver_name);
        buf[255] = 0;
    
        if(globus_extension_activate(buf) != GLOBUS_SUCCESS)
        {
            result = GlobusXIOErrorInvalidDriver("extension activate failed");
            goto error_activate;
        }
        
        activated = GLOBUS_TRUE;
        hook = (globus_xio_driver_hook_t *) globus_extension_lookup(
            &handle, GLOBUS_XIO_DRIVER_REGISTRY, driver_name);
    }
    
    if(!hook)
    {
        result = GlobusXIOErrorInvalidDriver("driver lookup failed");
        goto error_hook;
    }
    
    result = hook->init(out_driver);
    if(result != GLOBUS_SUCCESS)
    {
        result = GlobusXIOErrorWrapFailed("globus_xio_driver_init_t", result);
        goto error_init;
    }
    
    globus_assert(
        *out_driver && "init returned success but passed back null driver");
    
    (*out_driver)->hook = hook;
    (*out_driver)->extension_handle = handle;
    (*out_driver)->extension_activated = activated;

    return GLOBUS_SUCCESS;
    
error_init:
    globus_extension_release(handle);
error_hook:
    if(activated)
    {
        globus_extension_deactivate(buf);
    }
error_activate:
    *out_driver = GLOBUS_NULL;
error_param:
    GlobusXIODebugExitWithError();
    return result;
}

globus_result_t
globus_xio_driver_unload(
    globus_xio_driver_t                 driver)
{
    globus_xio_driver_hook_t *          hook;
    globus_extension_handle_t           handle;
    globus_bool_t                       activated;
    globus_result_t                     result;
    char                                buf[256];    
    GlobusXIOName(globus_xio_driver_unload);
    
    GlobusXIODebugEnter();
    
    if(!driver)
    {
        result = GlobusXIOErrorParameter("driver");
        goto error_param;
    }
    
    hook = driver->hook;
    handle = driver->extension_handle;
    activated = driver->extension_activated;
    if(activated)
    {
        snprintf(buf, 256, GLOBUS_XIO_EXTENSION_FORMAT, driver->name);
        buf[255] = 0;
    }
    globus_i_xio_close_handles(driver);
    hook->destroy(driver);
    globus_extension_release(handle);
    if(activated)
    {
        globus_extension_deactivate(buf);
    }
    
    GlobusXIODebugExit();
    return GLOBUS_SUCCESS;

error_param:    
    GlobusXIODebugExitWithError();
    return result;
}

globus_bool_t
globus_xio_driver_error_match(
    globus_xio_driver_t                 driver,
    globus_object_t *                   error,
    int                                 type)
{
    globus_bool_t                       match = GLOBUS_FALSE;
    GlobusXIOName(globus_xio_driver_error_match);
    
    GlobusXIODebugEnter();
    
    if(driver && driver->extension_handle)
    {
        match = globus_extension_error_match(
            driver->extension_handle, error, type);
    }

    GlobusXIODebugExit();
    return match;
}
