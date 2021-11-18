
#include "globus_xio_util.h"
#include "globus_xio_types.h"
#include "globus_common.h"

globus_bool_t
globus_xio_get_env_pair(
    const char *                        env_name,
    int *                               min,
    int *                               max)
{
    char *                              min_max;
    GlobusXIOName(globus_xio_get_env_pair);

    min_max = globus_module_getenv(env_name);

    if(min_max && sscanf(min_max, " %d , %d", min, max) == 2)
    {
        return GLOBUS_TRUE;
    }

    return GLOBUS_FALSE;
}

globus_bool_t
globus_xio_error_is_eof(
    globus_result_t                     res)
{
    return globus_error_match(
        globus_error_peek(res), GLOBUS_XIO_MODULE, GLOBUS_XIO_ERROR_EOF);
}

globus_bool_t
globus_xio_error_is_canceled(
    globus_result_t                     res)
{
    return globus_error_match(
        globus_error_peek(res), GLOBUS_XIO_MODULE, GLOBUS_XIO_ERROR_CANCELED);
}

globus_bool_t
globus_xio_error_match(
    globus_result_t                     result,
    int                                 type)
{
    return globus_error_match(
        globus_error_peek(result), GLOBUS_XIO_MODULE, type);
}

globus_result_t
globus_xio_contact_copy(
    globus_xio_contact_t *              dst,
    const globus_xio_contact_t *        src)
{
    globus_result_t                     result;
    GlobusXIOName(globus_xio_contact_copy);

    if(dst == NULL)
    {
        result = GlobusXIOErrorParameter("dst");
        goto error;
    }
    if(src == NULL)
    {
        result = GlobusXIOErrorParameter("src");
        goto error;
    }

    memset(dst, '\0', sizeof(globus_xio_contact_t));

    if(src->unparsed)
    {
        dst->unparsed = strdup(src->unparsed);
    }
    if(src->resource)
    {
        dst->resource = strdup(src->resource);
    }
    if(src->host)
    {
        dst->host = strdup(src->host);
    }
    if(src->port)
    {
        dst->port = strdup(src->port);
    }
    if(src->scheme)
    {
        dst->scheme = strdup(src->scheme);
    }
    if(src->user)
    {
        dst->user = strdup(src->user);
    }
    if(src->pass)
    {
        dst->pass = strdup(src->pass);
    }
    if(src->subject)
    {
        dst->subject = strdup(src->subject);
    }

    return GLOBUS_SUCCESS;
error:
    return result;
}

