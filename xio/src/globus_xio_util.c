/*
 * Copyright 1999-2006 University of Chicago
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
    GlobusXIOName(globus_l_xio_tcp_get_env_pair);

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
globus_xio_contact_info_copy(
    globus_xio_contact_t **             dst,
    const globus_xio_contact_t *        src)
{
    globus_result_t                     result;
    globus_xio_contact_t *              ci;
    GlobusXIOName(globus_xio_contact_info_copy);

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

    ci = (globus_xio_contact_t *)
        globus_calloc(1, sizeof(globus_xio_contact_t));
    if(ci == NULL)
    {
        result = GlobusXIOErrorMemory("ci");
        goto error;
    }

    if(src->unparsed)
    {
        ci->unparsed = strdup(src->unparsed);
    }
    if(src->resource)
    {
        ci->resource = strdup(src->resource);
    }
    if(src->host)
    {
        ci->host = strdup(src->host);
    }
    if(src->port)
    {
        ci->port = strdup(src->port);
    }
    if(src->scheme)
    {
        ci->scheme = strdup(src->scheme);
    }
    if(src->user)
    {
        ci->user = strdup(src->user);
    }
    if(src->pass)
    {
        ci->pass = strdup(src->pass);
    }
    if(src->subject)
    {
        ci->subject = strdup(src->subject);
    }

    *dst = ci;

    return GLOBUS_SUCCESS;
error:
    return result;
}

