#include "globus_xio_driver.h"
#include "globus_xio_gridftp_driver.h"
#include "globus_ftp_client.h"	
#include "version.h"

GlobusDebugDefine(GLOBUS_XIO_GRIDFTP);
GlobusXIODeclareDriver(gridftp);

#define GlobusXIOGridftpDebugPrintf(level, message)                           \
    GlobusDebugPrintf(GLOBUS_XIO_GRIDFTP, level, message)

#define GlobusXIOGridftpDebugEnter()                                          \
    GlobusXIOGridftpDebugPrintf(                                              \
        GLOBUS_L_XIO_GRIDFTP_DEBUG_TRACE,                                     \
        ("[%s] Entering\n", _xio_name))

#define GlobusXIOGridftpDebugExit()                                           \
    GlobusXIOGridftpDebugPrintf(                                              \
        GLOBUS_L_XIO_GRIDFTP_DEBUG_TRACE,                                     \
        ("[%s] Exiting\n", _xio_name))

#define GlobusXIOGridftpDebugExitWithError()                                  \
    GlobusXIOGridftpDebugPrintf(                                              \
        GLOBUS_L_XIO_GRIDFTP_DEBUG_TRACE,                                     \
        ("[%s] Exiting with error\n", _xio_name))

enum globus_l_xio_error_levels
{
    GLOBUS_L_XIO_GRIDFTP_DEBUG_TRACE                = 1,
    GLOBUS_L_XIO_GRIDFTP_DEBUG_INTERNAL_TRACE       = 2
};



typedef struct
{

    globus_ftp_client_handle_t 		ftp_handle;
    globus_ftp_client_handleattr_t 	ftp_handle_attr;
    globus_ftp_client_operationattr_t 	ftp_operation_attr;
    globus_bool_t			partial_xfer;

} globus_l_xio_gridftp_attr_t;


static globus_l_xio_gridftp_attr_t	globus_l_xio_gridftp_attr_default =
{
    GLOBUS_NULL,
    GLOBUS_NULL,
    GLOBUS_NULL,
    GLOBUS_FALSE
};

typedef struct
{

    globus_ftp_client_handle_t 		ftp_handle;
    globus_l_xio_gridftp_attr_t *	attr;
    globus_i_xio_gridftp_state_t	state;	
    globus_memory_t 			requestor_memory;
    globus_fifo_t 			pending_ops_q;
    char *				url;	
    /* TRUE - sending, FALSE - receiving */
    globus_bool_t			outstanding_ops_direction; 
    globus_bool_t			pending_ops_direction; 
    globus_xio_operation_t		partial_xfer_op;
    globus_size_t			partial_xfer_len;	
    globus_bool_t			xfer_done;
    /* 
     * this is because get/put callback might be called before read/write 
     * callback
     */		
    int					outstanding_io_count;
    /* 
     * No need for pending_io_count (i use globus_fifo_empty(pending_ops_q)
     * instead)
     */		
    globus_off_t			offset;
    globus_off_t			end_offset;
    globus_off_t			size;
    globus_mutex_t			mutex;
	
} globus_l_xio_gridftp_handle_t;


typedef struct
{

    globus_xio_operation_t		op;
    globus_xio_iovec_t *		iovec;
    globus_l_xio_gridftp_handle_t *	handle;

} globus_i_xio_gridftp_requestor_t;


static
int
globus_l_xio_gridftp_activate(void);

static
int
globus_l_xio_gridftp_deactivate(void);

static
void
globus_i_xio_gridftp_contact_info_copy(
    globus_xio_contact_t *              dst_contact_info,
    const globus_xio_contact_t *        src_contact_info);

static
void
globus_i_xio_gridftp_contact_info_destroy(
    globus_xio_contact_t *              contact_info);

static
globus_l_xio_gridftp_handle_t *
globus_l_xio_gridftp_handle_create(
    const globus_l_xio_gridftp_attr_t * attr,
    const globus_xio_contact_t *        contact_info);

static
globus_result_t
globus_l_xio_gridftp_handle_destroy(
    globus_l_xio_gridftp_handle_t *     handle);

static
void
globus_l_xio_gridftp_open_cb(
    void *                              user_arg,
    globus_ftp_client_handle_t *        ftp_handle,
    globus_object_t *                   error);

static
globus_bool_t
globus_l_xio_gridftp_change_state(
    globus_l_xio_gridftp_handle_t *     handle);

static
globus_result_t
globus_l_xio_gridftp_get(
    globus_l_xio_gridftp_handle_t *     handle);

static
globus_result_t
globus_l_xio_gridftp_put(
    globus_l_xio_gridftp_handle_t *     handle);

static           
void             
globus_l_xio_gridftp_xfer_cb(
    void *                              user_arg,
    globus_ftp_client_handle_t *        ftp_handle,
    globus_object_t *                   error);

static
globus_result_t
globus_l_xio_gridftp_read(
    void *                              driver_specific_handle,
    const globus_xio_iovec_t*           iovec,
    int                                 iovec_count,
    globus_xio_operation_t              op);

static
globus_result_t
globus_l_xio_gridftp_write(
    void *                              driver_specific_handle,
    const globus_xio_iovec_t *          iovec,
    int                                 iovec_count,
    globus_xio_operation_t              op);

static
void
globus_l_xio_gridftp_io_cb(
    void *                              user_arg,
    globus_ftp_client_handle_t *        ftp_handle,
    globus_object_t *                   error,
    globus_byte_t *                     buffer,
    globus_size_t                       length,
    globus_off_t                        offset,
    globus_bool_t                       eof);


static 
void
globus_l_xio_gridftp_write_eof_cb(
    void *                              user_arg,
    globus_ftp_client_handle_t *        ftp_handle,
    globus_object_t *                   error,
    globus_byte_t *                     buffer,
    globus_size_t                       length,
    globus_off_t                        offset, 
    globus_bool_t                       eof);

static
void
globus_l_xio_gridftp_cancel_cb(
    globus_xio_operation_t              op,
    void *                              user_arg);

static
void
globus_i_xio_gridftp_abort_io(
    globus_l_xio_gridftp_handle_t * 	handle);

static      
globus_result_t
globus_i_xio_gridftp_set_authorization(
    globus_ftp_client_operationattr_t * attr,
    va_list                             ap);

static
globus_result_t
globus_i_xio_gridftp_set_parallelism(
    globus_ftp_client_operationattr_t * attr,
    int                                 num_streams);

static
globus_result_t
globus_l_xio_gridftp_attr_init(
    void **                             out_attr);

static
globus_result_t
globus_l_xio_gridftp_attr_copy(
    void **                             dst,
    void *                              src);

static
globus_result_t
globus_l_xio_gridftp_attr_destroy(
    void *                              driver_attr);


GlobusXIODefineModule(gridftp) =
{
    "globus_xio_gridftp",
    globus_l_xio_gridftp_activate,
    globus_l_xio_gridftp_deactivate,
    GLOBUS_NULL,
    GLOBUS_NULL,
    &local_version
};


static
int
globus_l_xio_gridftp_activate(void)
{
    int rc;
    GlobusXIOName(globus_l_xio_gridftp_activate);

    GlobusDebugInit(GLOBUS_XIO_GRIDFTP, TRACE);

    GlobusXIOGridftpDebugEnter();

    rc = globus_module_activate(GLOBUS_XIO_MODULE);
    if (rc != GLOBUS_SUCCESS)
    {
        goto error_xio_system_activate;
    }
    rc = globus_module_activate(GLOBUS_FTP_CLIENT_MODULE);
    if (rc != GLOBUS_SUCCESS)
    {
        goto error_ftp_client_activate;
    }
    GlobusXIORegisterDriver(gridftp);

    GlobusXIOGridftpDebugExit();
    return GLOBUS_SUCCESS;

error_ftp_client_activate:
    globus_module_deactivate(GLOBUS_XIO_MODULE);

error_xio_system_activate:
    GlobusXIOGridftpDebugExitWithError();
    GlobusDebugDestroy(GLOBUS_XIO_GRIDFTP);
    return rc;
}


static
int
globus_l_xio_gridftp_deactivate(void)
{   
    int rc;
    GlobusXIOName(globus_l_xio_gridftp_deactivate);
    
    GlobusXIOGridftpDebugEnter();

    GlobusXIOUnRegisterDriver(gridftp);

    rc = globus_module_deactivate(GLOBUS_FTP_CLIENT_MODULE);
    if (rc != GLOBUS_SUCCESS)
    {   
        goto error_deactivate;
    }
    rc = globus_module_deactivate(GLOBUS_XIO_MODULE);
    if (rc != GLOBUS_SUCCESS)
    {   
        goto error_deactivate;
    }
    
    GlobusXIOGridftpDebugExit();
    GlobusDebugDestroy(GLOBUS_XIO_GRIDFTP);
    return GLOBUS_SUCCESS;

error_deactivate:
    GlobusXIOGridftpDebugExitWithError();
    GlobusDebugDestroy(GLOBUS_XIO_GRIDFTP);
    return rc;
}

static
void
globus_i_xio_gridftp_contact_info_copy(
    globus_xio_contact_t *		dst_contact_info,
    const globus_xio_contact_t *	src_contact_info) 
{
    GlobusXIOName(globus_i_xio_gridftp_contact_info_copy);	

    GlobusXIOGridftpDebugEnter();

    /* 
     * I use globus_xio_contact_info_to_string to construct the url to
     * pass to the client library. Since the client lib does not take the 
     * subject string in the url and unparsed is not used by 
     * globus_xio_contact_info_to_string, those fields are set to NULL
     */
    dst_contact_info->unparsed = GLOBUS_NULL;

    globus_assert(src_contact_info->resource != GLOBUS_NULL);	
    dst_contact_info->resource = strdup(src_contact_info->resource);

    globus_assert(src_contact_info->host != GLOBUS_NULL);	
    dst_contact_info->host = strdup(src_contact_info->host);

    globus_assert(src_contact_info->port != GLOBUS_NULL);	
    dst_contact_info->port = strdup(src_contact_info->port);

    globus_assert(src_contact_info->scheme != GLOBUS_NULL);	
    dst_contact_info->scheme = strdup(src_contact_info->scheme);

    if (src_contact_info->user != GLOBUS_NULL)
    {	
	dst_contact_info->user = strdup(src_contact_info->user);
    }
    else
    {
	dst_contact_info->user = GLOBUS_NULL;
    }
    if (src_contact_info->pass != GLOBUS_NULL)
    {	
        dst_contact_info->pass = strdup(src_contact_info->pass);
    }
    else
    {
	dst_contact_info->pass = GLOBUS_NULL;
    }
    dst_contact_info->subject = GLOBUS_NULL;

    GlobusXIOGridftpDebugExit();
}


static
void
globus_i_xio_gridftp_contact_info_destroy(
    globus_xio_contact_t *			contact_info)
{   
    GlobusXIOName(globus_i_xio_gridftp_contact_info_destroy);
    
    GlobusXIOGridftpDebugEnter();
    
    globus_free(contact_info->resource);
    globus_free(contact_info->host); 
    globus_free(contact_info->port); 
    globus_free(contact_info->scheme); 
    globus_free(contact_info->user); 
    globus_free(contact_info->pass); 
    
    GlobusXIOGridftpDebugExit();
}   

static
globus_result_t
globus_l_xio_gridftp_handle_destroy(
    globus_l_xio_gridftp_handle_t *	handle)
{
    globus_result_t			result;
    GlobusXIOName(globus_l_xio_gridftp_handle_destroy);

    GlobusXIOGridftpDebugEnter();

    /*
     * handle->attr->ftp_handle_attr would be non null only when i create 
     * the ftp_handle. i destroy the ftp_handle only if i had created it.
     * if it was given to me by the user then ftp_handle_attr would be null 
     * and thus i dont destroy the ftp_handle
     */  	
    if (handle->attr->ftp_handle_attr != GLOBUS_NULL)
    {
	result = globus_ftp_client_handleattr_destroy(
			&handle->attr->ftp_handle_attr);
	if (result != GLOBUS_SUCCESS)
	{
	    result = GlobusXIOErrorWrapFailed(
		"globus_ftp_client_handleattr_destroy", result);
	    goto error;
	}
	result = globus_ftp_client_handle_destroy(&handle->attr->ftp_handle);
	if (result != GLOBUS_SUCCESS)
	{
	    result = GlobusXIOErrorWrapFailed(
		"globus_ftp_client_handle_destroy", result);
	    goto error;
	}
    }
    result = globus_ftp_client_operationattr_destroy(
		&handle->attr->ftp_operation_attr);
    if(result != GLOBUS_SUCCESS)
    {
        result = GlobusXIOErrorWrapFailed(
            "globus_ftp_client_operationattr_destroy", result);
        goto error;
    }
    result = globus_l_xio_gridftp_attr_destroy(handle->attr);
    if(result != GLOBUS_SUCCESS)
    {
        result = GlobusXIOErrorWrapFailed(
            "globus_l_xio_gridftp_attr_destroy", result);
        goto error;
    }


    globus_free(handle->url);

    globus_fifo_destroy(&handle->pending_ops_q);
    globus_memory_destroy(&handle->requestor_memory);
    globus_mutex_destroy(&handle->mutex);
    globus_free(handle);

    GlobusXIOGridftpDebugExit();
    return GLOBUS_SUCCESS;

error:
    GlobusXIOGridftpDebugExitWithError();
    return result;
}


/*
 *  allocate the memory for and initialize an internal handle
 */
static 
globus_l_xio_gridftp_handle_t *
globus_l_xio_gridftp_handle_create(
    const globus_l_xio_gridftp_attr_t *	attr,
    const globus_xio_contact_t *	contact_info)
{
    globus_l_xio_gridftp_handle_t *	handle;
    globus_result_t                     result;
    int					node_size;
    int					node_count;
    globus_xio_contact_t		contact_info_local;
    GlobusXIOName(globus_l_xio_gridftp_handle_create);

    GlobusXIOGridftpDebugEnter();

    handle = (globus_l_xio_gridftp_handle_t *)
		globus_malloc(sizeof(globus_l_xio_gridftp_handle_t));
    if (handle == GLOBUS_NULL)
    {
	goto error_handle;
    }
    /* 
     * If attr has been initialized (ie, globus_l_xio_gridftp_attr_init has
     * been called -- globus_l_xio_gridftp_attr_init gets called when user
     * calls globus_xio_attr_cntl -- not when he calls globus_xio_attr_init)
     * then ftp_operation_attr would be non-null
     */
    if (attr->ftp_operation_attr)
    {	
    	result = globus_l_xio_gridftp_attr_copy(
			(void**)&handle->attr, (void*)attr);
    }
    else
    {
	result = globus_l_xio_gridftp_attr_init((void**)&handle->attr);
    }
    if (result != GLOBUS_SUCCESS)
    {
        result = GlobusXIOErrorWrapFailed(
            "globus_l_xio_gridftp_attr_copy", result);
        goto error_attr;
    }
    if (handle->attr->ftp_handle == GLOBUS_NULL)
    {   
	result = globus_ftp_client_handleattr_init(
	    &handle->attr->ftp_handle_attr);
	if (result != GLOBUS_SUCCESS)
	{
	    result = GlobusXIOErrorWrapFailed(
		"globus_ftp_client_handleattr_init", result);
	    goto error_ftp_handle_attr_init;
	}
	result = globus_ftp_client_handle_init(
	    &handle->attr->ftp_handle, 
	    &handle->attr->ftp_handle_attr);
	if (result != GLOBUS_SUCCESS)
	{
	    goto error_ftp_handle_init;
	}
    }	

    globus_i_xio_gridftp_contact_info_copy(&contact_info_local, contact_info);
    result = globus_xio_contact_info_to_string(
		&contact_info_local, 
		&handle->url);
    if (result != GLOBUS_SUCCESS)
    {
	goto error_url;
    }	
    globus_i_xio_gridftp_contact_info_destroy(&contact_info_local);
    result = globus_ftp_client_handle_cache_url_state(
	&handle->attr->ftp_handle, 
	handle->url);
    if (result != GLOBUS_SUCCESS)
    {
	goto error_enable_caching;
    }

    result = globus_fifo_init(&handle->pending_ops_q);
    if (result != GLOBUS_SUCCESS)
    {
	goto error_fifo_init;
    }		
    node_size = sizeof(globus_i_xio_gridftp_requestor_t);	
    node_count = GLOBUS_XIO_GRIDFTP_OP_INFO_COUNT;	
    globus_memory_init(&handle->requestor_memory, node_size, node_count);
    globus_mutex_init(&handle->mutex, NULL);	
    handle->state = GLOBUS_XIO_GRIDFTP_NONE;
    handle->partial_xfer_op = GLOBUS_NULL;
    handle->partial_xfer_len = 0;
    handle->outstanding_io_count = 0; 	
    handle->offset = 0; 
    handle->end_offset = -1; 

    GlobusXIOGridftpDebugExit();
    return handle;

error_fifo_init:
    globus_ftp_client_handle_flush_url_state(
	&handle->attr->ftp_handle, handle->url);
error_enable_caching:
    globus_free(handle->url);	
error_url:
    /* 
     * I check ftp_handle_attr here coz that is the indicator whether the
     * ftp_handle is given by the user or not, if ftp_handle_attr is not
     * null then i'm sure ftp_handle is not given the user. 
     */
    if (handle->attr->ftp_handle_attr)
    {	
	globus_ftp_client_handle_destroy(&handle->attr->ftp_handle);
    }
error_ftp_handle_init:
    if (handle->attr->ftp_handle_attr)
    {	
        globus_ftp_client_handleattr_destroy(&handle->attr->ftp_handle_attr);
    }
error_ftp_handle_attr_init:
    globus_l_xio_gridftp_attr_destroy(handle->attr);
error_attr:
    globus_free(handle);
error_handle:
    GlobusXIOGridftpDebugExitWithError();
    return GLOBUS_NULL;
}


static
void
globus_l_xio_gridftp_open_cb(
    void *                              user_arg,
    globus_ftp_client_handle_t *        ftp_handle,
    globus_object_t *                   error)
{
    globus_i_xio_gridftp_requestor_t *  requestor;
    globus_l_xio_gridftp_handle_t *     handle;

    GlobusXIOName(globus_l_xio_gridftp_open_cb);

    GlobusXIOGridftpDebugEnter();

    requestor = (globus_i_xio_gridftp_requestor_t *) user_arg;
    globus_xio_operation_disable_cancel(requestor->op);
    handle = requestor->handle;	
    if (error != GLOBUS_SUCCESS && (error != GLOBUS_NULL &&
	globus_error_ftp_error_get_code(error) != 550))
    {
	char * error_str;
        globus_result_t result;
        error_str = globus_error_print_friendly(error);
        result = GlobusXIOGridftpOpenError(error_str); 	
        globus_xio_driver_finished_open(handle, requestor->op, result);
    }
    else
    {
        globus_mutex_lock(&handle->mutex);
	handle->state = GLOBUS_XIO_GRIDFTP_OPEN;
        globus_mutex_unlock(&handle->mutex);
	globus_xio_driver_finished_open(
		handle, requestor->op, GLOBUS_SUCCESS);
    }		
    globus_memory_push_node(&handle->requestor_memory, (void*)requestor);
    GlobusXIOGridftpDebugExit();
}


static
globus_result_t
globus_l_xio_gridftp_open(
    const globus_xio_contact_t *        contact_info,
    void *				driver_link,
    void *                              driver_attr,
    globus_xio_operation_t              op)
{
    globus_l_xio_gridftp_handle_t *     handle = GLOBUS_NULL;
    const globus_l_xio_gridftp_attr_t * attr;
    globus_i_xio_gridftp_requestor_t *  requestor;
    globus_result_t                     result;
    globus_result_t                     res;

    GlobusXIOName(globus_l_xio_gridftp_open);

    GlobusXIOGridftpDebugEnter();

    globus_assert(driver_link == GLOBUS_NULL);
    attr = (globus_l_xio_gridftp_attr_t *)
        (driver_attr ? driver_attr : &globus_l_xio_gridftp_attr_default);

    handle = globus_l_xio_gridftp_handle_create(attr, contact_info);
    if (handle == GLOBUS_NULL)
    {
        result = GlobusXIOGridftpHandleError();
        goto error_handle;
    }

    if (contact_info->subject)
    {
	/* 
	 * if user and/or pass present in contact_info, it would have been
	 * put into the url (client lib allows user and pass be to present
	 * in the url but not subject)
	 */
	result = globus_ftp_client_operationattr_set_authorization(
	    &handle->attr->ftp_operation_attr,
	    GSS_C_NO_CREDENTIAL,
	    GLOBUS_NULL,  		/* user */
	    GLOBUS_NULL,  		/* pass */
	    GLOBUS_NULL,  		/* acct */
	    contact_info->subject);
	if (result != GLOBUS_SUCCESS)
	{
	    goto error_auth;
	}
    }
    requestor = (globus_i_xio_gridftp_requestor_t *)
			globus_memory_pop_node(&handle->requestor_memory);
    requestor->op = op;
    requestor->handle = handle;
    if (globus_xio_operation_enable_cancel(
	op, globus_l_xio_gridftp_cancel_cb, requestor))
    {
	result = GlobusXIOErrorCanceled();
	goto error_operation_canceled;
    }
    globus_mutex_lock(&handle->mutex);	
    result = globus_ftp_client_size(
	&handle->attr->ftp_handle, 
	handle->url, 
	&handle->attr->ftp_operation_attr, 
	&handle->size,
	globus_l_xio_gridftp_open_cb, 
	requestor);
    globus_mutex_unlock(&handle->mutex);	

    if (result != GLOBUS_SUCCESS)	
    {
	goto error_connect;
    }

    GlobusXIOGridftpDebugExit();
    return GLOBUS_SUCCESS;

error_connect:
error_auth:
    globus_xio_operation_disable_cancel(op);
    res = globus_l_xio_gridftp_handle_destroy(handle);	
    /* what if the cancel_cb is still active (can happen in threaded build)? 
     * check with joe ???
     */
    globus_assert (res == GLOBUS_SUCCESS);	
error_handle:
error_operation_canceled:
    GlobusXIOGridftpDebugExitWithError();
    return result;
}


static
globus_bool_t
globus_l_xio_gridftp_change_state(
    globus_l_xio_gridftp_handle_t *     handle)
{
    globus_bool_t			close = GLOBUS_FALSE;
    GlobusXIOName(globus_l_xio_gridftp_change_state);

    GlobusXIOGridftpDebugEnter();

    /* This function is called with handle->mutex locked */
    switch (handle->state)
    {
	case GLOBUS_XIO_GRIDFTP_IO_PENDING:		
	    if (handle->outstanding_io_count == 0)
	    {
		handle->state = GLOBUS_XIO_GRIDFTP_IO_DONE;
	    }
	    /* fall through */
	case GLOBUS_XIO_GRIDFTP_IO_DONE:
	    if (handle->xfer_done == GLOBUS_TRUE)
	    {
		handle->state = GLOBUS_XIO_GRIDFTP_OPEN;
	    }	    
	    break;
	case GLOBUS_XIO_GRIDFTP_ABORT_PENDING:
	    if ((handle->outstanding_io_count == 0) && 
		(handle->xfer_done == GLOBUS_TRUE))
	    {
	        handle->state = GLOBUS_XIO_GRIDFTP_OPEN;
	    }
	    break;
	case GLOBUS_XIO_GRIDFTP_ABORT_PENDING_IO_PENDING:
	{
	    globus_i_xio_gridftp_requestor_t * requestor;
	    if ((handle->outstanding_io_count == 0) && 
		(handle->xfer_done == GLOBUS_TRUE))
	    {
		handle->state = GLOBUS_XIO_GRIDFTP_OPEN;
		while (globus_fifo_empty(&handle->pending_ops_q))
		{
		    requestor = (globus_i_xio_gridftp_requestor_t*)
				      globus_fifo_dequeue(
					    &handle->pending_ops_q);
		    if (handle->pending_ops_direction == GLOBUS_TRUE)
		    {
			globus_l_xio_gridftp_read(
			    handle,
			    requestor->iovec,
			    1, /* as this driver just handles the first buffer
				* in the iovec irrespective of iovec_count */
			    requestor->op);
		    }
		    else
		    {
			globus_l_xio_gridftp_write(
			    handle,
			    requestor->iovec,
			    1, /* as this driver just handles the first buffer
				* in the iovec irrespective of iovec_count */
			    requestor->op);
		    } 
    		    globus_memory_push_node(
			&handle->requestor_memory, (void*)requestor);
		}
	    }		
	    break;
	}
	case GLOBUS_XIO_GRIDFTP_ABORT_PENDING_CLOSING:
	    if ((handle->outstanding_io_count == 0) && 
		(handle->xfer_done == GLOBUS_TRUE))
	    {
		handle->state = GLOBUS_XIO_GRIDFTP_NONE;
		close = GLOBUS_TRUE;
	    }	
	    break;
	default:
	    /* if it gets here, something is wrong */
	    globus_assert(0);
    }	

    GlobusXIOGridftpDebugExit();
    return close;
}


static
void
globus_l_xio_gridftp_xfer_cb(
    void *				user_arg,
    globus_ftp_client_handle_t * 	ftp_handle,
    globus_object_t *			error)
{
    globus_l_xio_gridftp_handle_t *     handle;
    globus_bool_t			close;	
    globus_xio_operation_t		finish_op = GLOBUS_NULL;
    globus_bool_t			direction;
    globus_size_t			len;	
    globus_result_t			result = GLOBUS_SUCCESS;	

    GlobusXIOName(globus_l_xio_gridftp_xfer_cb);

    GlobusXIOGridftpDebugEnter();

    handle = (globus_l_xio_gridftp_handle_t *) user_arg;
    globus_mutex_lock(&handle->mutex);
    handle->xfer_done = GLOBUS_TRUE;
    close = globus_l_xio_gridftp_change_state(handle);
    if (close == GLOBUS_TRUE)
    {
	globus_i_xio_gridftp_requestor_t * requestor;
	globus_result_t			   result;
	requestor = (globus_i_xio_gridftp_requestor_t*)
			globus_fifo_dequeue(&handle->pending_ops_q);
	globus_xio_driver_finished_close(requestor->op, GLOBUS_SUCCESS);
	globus_memory_push_node(&handle->requestor_memory, (void*)requestor);
	globus_mutex_unlock(&handle->mutex);
	result = globus_l_xio_gridftp_handle_destroy(handle);
	globus_assert(result == GLOBUS_SUCCESS);
    }		
    else if (handle->partial_xfer_op && 
	handle->state == GLOBUS_XIO_GRIDFTP_OPEN)
    {
	finish_op = handle->partial_xfer_op;
	handle->partial_xfer_op = GLOBUS_NULL;
	direction = handle->outstanding_ops_direction;
	len = handle->partial_xfer_len;
	globus_mutex_unlock(&handle->mutex);
    }

    if (finish_op)
    {
	if (error != GLOBUS_SUCCESS)	
	{
	    char * error_str;
	    error_str = globus_error_print_friendly(error);
	    result = GlobusXIOGridftpIOError(error_str); 
	}
	if (direction == GLOBUS_TRUE)
	{ 
	    globus_xio_driver_finished_read(finish_op, result, len);
	}
	else
	{
	    globus_xio_driver_finished_write(finish_op, result, len);
	}
    }
    GlobusXIOGridftpDebugExit();

}


static
globus_result_t
globus_l_xio_gridftp_get(
    globus_l_xio_gridftp_handle_t *	handle)
{
    globus_result_t			result;
    GlobusXIOName(globus_l_xio_gridftp_get);
 
    GlobusXIOGridftpDebugEnter();

    handle->xfer_done = GLOBUS_FALSE;
    if (handle->offset > 0 || handle->partial_xfer_op)
    {
	result = globus_ftp_client_partial_get(
	    &handle->attr->ftp_handle,
	    handle->url,
	    &handle->attr->ftp_operation_attr,
	    GLOBUS_NULL, 	/* restart marker */
	    handle->offset,
	    handle->end_offset, /* represents end offset is end of file */	
	    globus_l_xio_gridftp_xfer_cb,
	    handle);
    }
    else
    {
	result = globus_ftp_client_get(
	    &handle->attr->ftp_handle,
	    handle->url,
	    &handle->attr->ftp_operation_attr,
	    GLOBUS_NULL,  	/* restart_marker */
	    globus_l_xio_gridftp_xfer_cb,
	    handle);
    } 	

    GlobusXIOGridftpDebugExit();
    return result;	

}



static
void
globus_l_xio_gridftp_io_cb(
    void * 				user_arg,
    globus_ftp_client_handle_t *	ftp_handle, 
    globus_object_t *			error, 
    globus_byte_t *			buffer,
    globus_size_t 			length,
    globus_off_t 			offset,
    globus_bool_t 			eof)
{
    globus_result_t 			result = GLOBUS_SUCCESS;
    globus_i_xio_gridftp_requestor_t *	requestor;
    globus_l_xio_gridftp_handle_t *	handle;
    globus_bool_t			close;
    globus_bool_t			finish = GLOBUS_TRUE;
    globus_bool_t			direction;

    GlobusXIOName(globus_l_xio_gridftp_io_cb);

    GlobusXIOGridftpDebugEnter();

    requestor = (globus_i_xio_gridftp_requestor_t *) user_arg;
    handle = requestor->handle;	
    globus_xio_operation_disable_cancel(requestor->op);
    globus_mutex_lock(&handle->mutex);
    handle->outstanding_io_count--;
    close = globus_l_xio_gridftp_change_state(handle);
    /* xio wouldn't call close while there is an outstanding operation */
    globus_assert(close == GLOBUS_FALSE);

    if (error == GLOBUS_SUCCESS)
    {
	if (offset + length > handle->offset)
	{
	    /* 
             * It should get here only for read operations. For write 
	     * operations, i set 'handle->offset = offset + length' in write
	     */ 
	    globus_assert(handle->outstanding_ops_direction == GLOBUS_TRUE);	
	    handle->offset = offset + length;
	}
	result = globus_xio_driver_data_descriptor_cntl(
	    		requestor->op, 
	    		NULL, 
	    		GLOBUS_XIO_DD_SET_OFFSET, 
            		offset);
	if (result == GLOBUS_SUCCESS)
	{
	    if (eof)
	    {
		/* 
		 * It should get here only for read operations. If eof is 
		 * written, write_eof_cb is used instead of this cb
		 */ 
		globus_assert(
			handle->outstanding_ops_direction == GLOBUS_TRUE);	
		globus_xio_driver_set_eof_received(requestor->op);
		result = GlobusXIOErrorEOF();
	    }
	}
    }
    else
    {
	char * error_str;
	error_str = globus_error_print_friendly(error);
	result = GlobusXIOGridftpIOError(error_str); 
    }
    direction = handle->outstanding_ops_direction;
    if (handle->partial_xfer_op)
    {	
 	if (handle->state == GLOBUS_XIO_GRIDFTP_OPEN)
	{
	    handle->partial_xfer_op = GLOBUS_NULL;
	}
	else
	{
	    handle->partial_xfer_len = length;
	    finish = GLOBUS_FALSE;	
	}
    }
    globus_mutex_unlock(&handle->mutex);
	
    /* 
     * no need to worry about the iovec count - handle just the first 
     * buffer in the iovec and xio takes care of the rest - if needed
     * xio would post another read after adjusting the iovec
     */
    if (finish)
    {
	if (direction == GLOBUS_TRUE)
	{ 
	    globus_xio_driver_finished_read(requestor->op, result, length);
	}
	else
	{
	    globus_xio_driver_finished_write(requestor->op, result, length);
	}
    }	
    globus_memory_push_node(&handle->requestor_memory, (void*)requestor);
    GlobusXIOGridftpDebugExit();
    return;

}


static
void
globus_l_xio_gridftp_write_eof_cb(
    void *                              user_arg,
    globus_ftp_client_handle_t *        ftp_handle,
    globus_object_t *                   error,
    globus_byte_t *                     buffer,
    globus_size_t                       length,
    globus_off_t                        offset,
    globus_bool_t                       eof)
{
    GlobusXIOName(globus_l_xio_gridftp_write_eof_cb);

    GlobusXIOGridftpDebugEnter();	

    GlobusXIOGridftpDebugExit();	
}



static
void
globus_l_xio_gridftp_cancel_cb(
    globus_xio_operation_t              op,
    void *                              user_arg)
{
    globus_i_xio_gridftp_requestor_t *	requestor;
    globus_l_xio_gridftp_handle_t *	handle;
    GlobusXIOName(globus_l_xio_gridftp_cancel_cb);

    GlobusXIOGridftpDebugEnter();	
    requestor = (globus_i_xio_gridftp_requestor_t *) user_arg;	
    handle = requestor->handle;

    /* 
     * no need to call finished read or write here, it is done in the 
     * read/write cb 
     */

    globus_mutex_lock(&handle->mutex);
    switch (handle->state)
    {
	case GLOBUS_XIO_GRIDFTP_NONE:
	    globus_ftp_client_abort(&handle->attr->ftp_handle);
	    break;
	case GLOBUS_XIO_GRIDFTP_IO_PENDING:
	    handle->state = GLOBUS_XIO_GRIDFTP_ABORT_PENDING;
	    globus_i_xio_gridftp_abort_io(handle);
	    break;	
        case GLOBUS_XIO_GRIDFTP_ABORT_PENDING:
	    break;
        case GLOBUS_XIO_GRIDFTP_ABORT_PENDING_IO_PENDING:
	    requestor = globus_fifo_remove(&handle->pending_ops_q, requestor);
	    if (requestor != GLOBUS_NULL)
	    {
		if (handle->pending_ops_direction == GLOBUS_TRUE)
		{
		    globus_xio_driver_finished_read(requestor->op,  
			GlobusXIOErrorCanceled(), 0);
		}
		else
		{
		    globus_xio_driver_finished_write(requestor->op,  
			GlobusXIOErrorCanceled(), 0);
		}
	    }
	    globus_memory_push_node(
		&handle->requestor_memory, (void*)requestor);
	    if (globus_fifo_empty(&handle->pending_ops_q))
	    {
		handle->state = GLOBUS_XIO_GRIDFTP_ABORT_PENDING;
	    }	
	    break;
	default:
	    /* if it gets here, something is wrong */
	    globus_assert(0);	
    }  
    globus_mutex_unlock(&handle->mutex);
 
    GlobusXIOGridftpDebugExit();	
}


static
globus_result_t
globus_l_xio_gridftp_read(
    void *                              driver_specific_handle,
    const globus_xio_iovec_t*           iovec,
    int                                 iovec_count,
    globus_xio_operation_t              op)
{
    globus_l_xio_gridftp_handle_t *     handle;
    globus_i_xio_gridftp_requestor_t *	requestor;
    globus_result_t                     result;

    GlobusXIOName(globus_l_xio_gridftp_read);

    GlobusXIOGridftpDebugEnter();
    handle = (globus_l_xio_gridftp_handle_t *) driver_specific_handle;
    requestor = (globus_i_xio_gridftp_requestor_t *)
                globus_memory_pop_node(&handle->requestor_memory);
    requestor->op = op;
    requestor->handle = handle;
    requestor->iovec = (globus_xio_iovec_t*)iovec;

    if (globus_xio_operation_enable_cancel(
	op, globus_l_xio_gridftp_cancel_cb, requestor))
    {
	result = GlobusXIOErrorCanceled();
	goto error_operation_canceled;
    }	
    globus_mutex_lock(&handle->mutex);
    if (handle->attr->partial_xfer && handle->state != GLOBUS_XIO_GRIDFTP_OPEN)
    {
	result = GlobusXIOGridftpOutstandingPartialXferError();
	goto error_outstanding_partial_xfer;
    }
    if (globus_xio_driver_eof_received(op))
    {
	result = GlobusXIOErrorEOF();
	goto error_eof_received;
    }

    switch (handle->state)
    {
	case GLOBUS_XIO_GRIDFTP_OPEN:		
	    handle->outstanding_ops_direction = GLOBUS_TRUE;  
	    if (handle->attr->partial_xfer)
	    {
		handle->partial_xfer_op = op;
		handle->end_offset = handle->offset + iovec[0].iov_len;
	    }
	    result = globus_l_xio_gridftp_get(handle);
	    if (result != GLOBUS_SUCCESS)
	    {   
		goto error_get;
	    }   
	case GLOBUS_XIO_GRIDFTP_IO_DONE:
	    handle->state = GLOBUS_XIO_GRIDFTP_IO_PENDING;
	case GLOBUS_XIO_GRIDFTP_IO_PENDING:
	{
	    /* simultaneous read and write not allowed */	
	    if (handle->outstanding_ops_direction == GLOBUS_FALSE)
	    {
		result = GlobusXIOGridftpOutstandingWriteError();
		goto error_outstanding_write;
	    }		
	    result = globus_ftp_client_register_read(
		&handle->attr->ftp_handle,
		iovec[0].iov_base,
		iovec[0].iov_len,
		globus_l_xio_gridftp_io_cb,
		requestor);
	    if (result != GLOBUS_SUCCESS)
	    {
		goto error_register_read;
	    }
	    ++handle->outstanding_io_count;			
	    break;
	}
	case GLOBUS_XIO_GRIDFTP_ABORT_PENDING:
	    handle->pending_ops_direction = GLOBUS_TRUE;	
	    handle->state = GLOBUS_XIO_GRIDFTP_ABORT_PENDING_IO_PENDING;
	case GLOBUS_XIO_GRIDFTP_ABORT_PENDING_IO_PENDING:
	{
	    /* simultaneous read and write not allowed */	
	    if (handle->pending_ops_direction == GLOBUS_FALSE)
	    {
		result = GlobusXIOGridftpPendingWriteError();
		goto error_pending_write;
	    }		
	    globus_fifo_enqueue(&handle->pending_ops_q, requestor);
	    break;
	}
	default:
	    /* if it gets here, something is wrong */
	    globus_assert(0);
    }
    globus_mutex_unlock(&handle->mutex);

    GlobusXIOGridftpDebugExit();
    return GLOBUS_SUCCESS;

error_pending_write:
error_register_read:
error_outstanding_write:
error_get:
error_eof_received:
error_outstanding_partial_xfer:
    globus_mutex_unlock(&handle->mutex);
    globus_xio_operation_disable_cancel(requestor->op);
    globus_memory_push_node(&handle->requestor_memory, (void*)requestor);
error_operation_canceled:
    GlobusXIOGridftpDebugExitWithError();
    return result;
	
}



static
globus_result_t
globus_l_xio_gridftp_put(
    globus_l_xio_gridftp_handle_t *         handle)
{
    globus_result_t			    result;
    GlobusXIOName(globus_l_xio_gridftp_put);

    GlobusXIOGridftpDebugEnter();

    handle->xfer_done = GLOBUS_FALSE;
    if (handle->offset > 0 || handle->partial_xfer_op)
    {
        result = globus_ftp_client_partial_put(
            &handle->attr->ftp_handle,
            handle->url,
            &handle->attr->ftp_operation_attr,
	    GLOBUS_NULL,
            handle->offset,
	    handle->end_offset,
            globus_l_xio_gridftp_xfer_cb,
            handle);
    }
    else
    {
        result = globus_ftp_client_put(
            &handle->attr->ftp_handle,
            handle->url,
            &handle->attr->ftp_operation_attr,
	    GLOBUS_NULL,
            globus_l_xio_gridftp_xfer_cb,
            handle);
    }

    GlobusXIOGridftpDebugExit();
    return result;	

}




static
globus_result_t
globus_l_xio_gridftp_write(
    void *                              driver_specific_handle,
    const globus_xio_iovec_t *          iovec,
    int                                 iovec_count,
    globus_xio_operation_t              op)
{
    globus_l_xio_gridftp_handle_t *     handle;
    globus_i_xio_gridftp_requestor_t *  requestor;
    globus_result_t                     result;

    GlobusXIOName(globus_l_xio_gridftp_write);
    GlobusXIOGridftpDebugEnter();
            
    handle = (globus_l_xio_gridftp_handle_t *) driver_specific_handle;
    requestor = (globus_i_xio_gridftp_requestor_t *)
                globus_memory_pop_node(&handle->requestor_memory);
    requestor->op = op;
    requestor->handle = handle;
    requestor->iovec = (globus_xio_iovec_t*)iovec;

    if (globus_xio_operation_enable_cancel(
        op, globus_l_xio_gridftp_cancel_cb, requestor))
    {
        result = GlobusXIOErrorCanceled();
        goto error_operation_canceled;
    }
    globus_mutex_lock(&handle->mutex);
    if (handle->attr->partial_xfer && handle->state != GLOBUS_XIO_GRIDFTP_OPEN)
    {
	result = GlobusXIOGridftpOutstandingPartialXferError();
	goto error_outstanding_partial_xfer;
    }
    switch (handle->state)
    {   
        case GLOBUS_XIO_GRIDFTP_OPEN:
            handle->outstanding_ops_direction = GLOBUS_FALSE;
	    if (handle->attr->partial_xfer)
	    {
		handle->partial_xfer_op = op;
		handle->end_offset = handle->offset + iovec[0].iov_len;
	    }
            result = globus_l_xio_gridftp_put(handle);
            if (result != GLOBUS_SUCCESS)
            {   
                goto error_put;
            }
        case GLOBUS_XIO_GRIDFTP_IO_DONE:
	    handle->state = GLOBUS_XIO_GRIDFTP_IO_PENDING;
        case GLOBUS_XIO_GRIDFTP_IO_PENDING:
        {
	    globus_off_t offset;
	    /* simultaneous read and write not allowed */	
            if (handle->outstanding_ops_direction == GLOBUS_TRUE)
            {
                result = GlobusXIOGridftpOutstandingReadError();
                goto error_outstanding_read;
            }
	    result = globus_xio_driver_data_descriptor_cntl(
                        op,
                        NULL,
                        GLOBUS_XIO_DD_GET_OFFSET,
                        &offset);
	    /* 
	     * If offset is not specified, dd_cntl will return offset = -1.
	     * In that case offset is set to handle->offset(whose intial value
	     * is zero). Basically the file will be overwritten from the start
	     * if offset is not specified.
             */	
	    if (result != GLOBUS_SUCCESS || offset == -1)
	    {
		offset = handle->offset;
	    }
            result = globus_ftp_client_register_write(
                &handle->attr->ftp_handle,     
                iovec[0].iov_base,
                iovec[0].iov_len,
		offset,
		GLOBUS_FALSE, /* eof */
                globus_l_xio_gridftp_io_cb,
                requestor);
            if (result == GLOBUS_SUCCESS)
            {
		++handle->outstanding_io_count;			
		handle->offset = offset + iovec[0].iov_len;
            }
	    else
	    {                   
                goto error_register_write;
	    }
            break;
        }
        case GLOBUS_XIO_GRIDFTP_ABORT_PENDING:
            handle->pending_ops_direction = GLOBUS_FALSE;
            handle->state = GLOBUS_XIO_GRIDFTP_ABORT_PENDING_IO_PENDING;
        case GLOBUS_XIO_GRIDFTP_ABORT_PENDING_IO_PENDING:
        {
	    /* simultaneous read and write not allowed */	
            if (handle->pending_ops_direction == GLOBUS_TRUE)
            {
                result = GlobusXIOGridftpPendingReadError();
                goto error_pending_read;
            }
            globus_fifo_enqueue(&handle->pending_ops_q, requestor);
            break;
        }
	default:
	    /* if it gets here, something is wrong */
	    globus_assert(0);
    }
    globus_mutex_unlock(&handle->mutex);

    GlobusXIOGridftpDebugExit();
    return GLOBUS_SUCCESS;

error_pending_read:
error_register_write:
error_outstanding_read:
error_put:
error_outstanding_partial_xfer:
    globus_mutex_unlock(&handle->mutex);
    globus_xio_operation_disable_cancel(requestor->op);
    globus_memory_push_node(&handle->requestor_memory, (void*)requestor);
error_operation_canceled:
    GlobusXIOGridftpDebugExitWithError();
    return result;
	
}


static
void
globus_i_xio_gridftp_abort_io(
    globus_l_xio_gridftp_handle_t *	handle)
{
    globus_byte_t			buffer;
    GlobusXIOName(globus_l_xio_gridftp_close);

    GlobusXIOGridftpDebugEnter();
    if (handle->outstanding_ops_direction == GLOBUS_TRUE)
    {
	globus_ftp_client_abort(&handle->attr->ftp_handle);
    }
    else
    {
	globus_ftp_client_register_write(
	    &handle->attr->ftp_handle,
	    &buffer,
	    0,
	    handle->offset,
	    GLOBUS_TRUE, /* eof */
	    globus_l_xio_gridftp_write_eof_cb,
	    NULL);
    }
    GlobusXIOGridftpDebugExit();
}


static
globus_result_t
globus_l_xio_gridftp_close(
    void *                              driver_specific_handle,
    void *                              attr,
    globus_xio_operation_t              op)
{
    globus_l_xio_gridftp_handle_t *     handle;
    globus_result_t			result;
    globus_bool_t			destroy = GLOBUS_FALSE;
    GlobusXIOName(globus_l_xio_gridftp_close);

    GlobusXIOGridftpDebugEnter();
    handle = (globus_l_xio_gridftp_handle_t *) driver_specific_handle;
    globus_mutex_lock(&handle->mutex);
    switch (handle->state)
    {
	case GLOBUS_XIO_GRIDFTP_OPEN:
	    handle->state = GLOBUS_XIO_GRIDFTP_NONE;
	    globus_xio_driver_finished_close(op, GLOBUS_SUCCESS);
	    destroy = GLOBUS_TRUE;
	    break;
	case GLOBUS_XIO_GRIDFTP_IO_DONE:
	    globus_i_xio_gridftp_abort_io(handle);
	case GLOBUS_XIO_GRIDFTP_ABORT_PENDING:
	{
	    globus_i_xio_gridftp_requestor_t * requestor;
	    requestor = (globus_i_xio_gridftp_requestor_t *)
			globus_memory_pop_node(&handle->requestor_memory);
	    requestor->op = op;
	    requestor->handle = handle;
	    globus_fifo_enqueue(&handle->pending_ops_q, requestor);
	    handle->state = GLOBUS_XIO_GRIDFTP_ABORT_PENDING_CLOSING;	
	    break;
	}
	default:
	    /* 
	     * I should get close only when i'm in one of the above states
	     * otherwise something is wrong (xio wouldn't give me a close
	     * when there is an user operation pending)
 	     */
	    globus_assert(0);
    }
    globus_mutex_unlock(&handle->mutex);
    if (destroy == GLOBUS_TRUE)
    {	
	result = globus_l_xio_gridftp_handle_destroy(handle);
	globus_assert(result == GLOBUS_SUCCESS);
    }
    GlobusXIOGridftpDebugExit();
    return GLOBUS_SUCCESS;	
}


static
globus_result_t
globus_l_xio_gridftp_cntl(
    void  *                             driver_specific_handle,
    int                                 cmd,
    va_list                             ap)
{
    globus_l_xio_gridftp_handle_t *     handle;
    globus_result_t                     result;

    GlobusXIOName(globus_l_xio_gridftp_cntl);

    GlobusXIOGridftpDebugEnter();

    handle = (globus_l_xio_gridftp_handle_t *) driver_specific_handle;
    
    switch(cmd)
    {
	case GLOBUS_XIO_GRIDFTP_SEEK:
	{
	    /* seek is always from the start of the file */
	    globus_off_t seek_offset;   
	    seek_offset = va_arg(ap, globus_off_t);
	    if (handle->offset != seek_offset)
	    {	
		switch (handle->state)
		{
		    case GLOBUS_XIO_GRIDFTP_IO_DONE:
			if (handle->partial_xfer_op)
			{
			    result = GlobusXIOGridftpSeekError(); 
			    goto error;	
			}
			/* put this in a function */
	    		globus_i_xio_gridftp_abort_io(handle);
			handle->state = GLOBUS_XIO_GRIDFTP_ABORT_PENDING;
		    case GLOBUS_XIO_GRIDFTP_OPEN:
		    case GLOBUS_XIO_GRIDFTP_ABORT_PENDING:
		    case GLOBUS_XIO_GRIDFTP_ABORT_PENDING_IO_PENDING:
			handle->offset = seek_offset;
			break;
		    default:
			/* seek not allowed in state's other than above */
			result = GlobusXIOGridftpSeekError(); 
			break;
		}
	    }	
	    break;
	}
	default:
	    result = GlobusXIOErrorInvalidCommand(cmd);
	    goto error;	    
    }	

    GlobusXIOGridftpDebugExit();
    return GLOBUS_SUCCESS;

error:
    GlobusXIOGridftpDebugExitWithError();
    return result;
}


static
globus_result_t
globus_l_xio_gridftp_attr_init(
    void **                             out_attr)
{
    globus_l_xio_gridftp_attr_t *       attr;
    globus_result_t                     result;
    GlobusXIOName(globus_l_xio_gridftp_attr_init);

    GlobusXIOGridftpDebugEnter();

    /*
     *  create a gridftp attr structure and intialize its values
     */
    attr = (globus_l_xio_gridftp_attr_t *) 
		globus_malloc(sizeof(globus_l_xio_gridftp_attr_t));
    if(!attr)
    {
        result = GlobusXIOErrorMemory("attr");
        goto error;
    }

    memcpy(attr, &globus_l_xio_gridftp_attr_default, 
		sizeof(globus_l_xio_gridftp_attr_t));
    result = globus_ftp_client_operationattr_init(
	&attr->ftp_operation_attr);
    if (result != GLOBUS_SUCCESS)
    {
        result = GlobusXIOErrorWrapFailed(
            "globus_ftp_client_operationattr_init", result);
        goto error;
    }

    *out_attr = attr;

    GlobusXIOGridftpDebugExit();
    return GLOBUS_SUCCESS;

error:
    GlobusXIOGridftpDebugExitWithError();
    return result;
}


static
globus_result_t
globus_i_xio_gridftp_set_authorization(
    globus_ftp_client_operationattr_t *	attr,
    va_list				ap)
{	
    globus_result_t			result;	
    char *				user;
    char *	 			password;
    char *				account;
    char *				subject;

    GlobusXIOName(globus_i_xio_gridftp_set_authorization);

    GlobusXIOGridftpDebugEnter();

    user = va_arg(ap, char *);
    password = va_arg(ap, char *);
    account = va_arg(ap, char *);
    subject = va_arg(ap, char *);
    result = globus_ftp_client_operationattr_set_authorization(
	attr,
	GSS_C_NO_CREDENTIAL,
	user,
	password,
	account, 
	subject);
     
    GlobusXIOGridftpDebugExit();
    return result;

}


static
globus_result_t
globus_i_xio_gridftp_set_parallelism(
    globus_ftp_client_operationattr_t *	attr,
    int					num_streams)
{
    globus_result_t			result;
    globus_ftp_control_parallelism_t 	parallelism;

    GlobusXIOName(globus_i_xio_gridftp_set_parallelism);

    GlobusXIOGridftpDebugEnter();	

    /*
     * typedef enum globus_ftp_control_parallelism_mode_e
     * {
     *     GLOBUS_FTP_CONTROL_PARALLELISM_NONE,
     *     GLOBUS_FTP_CONTROL_PARALLELISM_FIXED
     * } globus_ftp_control_parallelism_mode_t;
     *
     * typedef struct globus_i_ftp_parallelism_base_s
     * {
     *     globus_ftp_control_parallelism_mode_t       mode;
     *     globus_size_t                               size;
     * } globus_i_ftp_parallelism_base_t;
     *
     * typedef struct globus_ftp_parallelism_fixed_s
     * {
     *     globus_ftp_control_parallelism_mode_t       mode;
     *     globus_size_t                               size;
     * } globus_ftp_parallelism_fixed_t;
     *
     * typedef union globus_ftp_control_parallelism_u
     * {
     *     globus_ftp_control_parallelism_mode_t    mode;
     *     globus_i_ftp_parallelism_base_t          base;
     *     globus_ftp_parallelism_fixed_t           fixed;
     * } globus_ftp_control_parallelism_t; 
     */
    
    /* 
     * The following is been used in globus_url_copy to set the number of
     * streams
     */	
	
    result = globus_ftp_client_operationattr_set_mode(
	attr,
	GLOBUS_FTP_CONTROL_MODE_EXTENDED_BLOCK);
    if (result != GLOBUS_SUCCESS)
    {
	goto error;
    }		

    parallelism.mode = GLOBUS_FTP_CONTROL_PARALLELISM_FIXED;
    parallelism.fixed.size = num_streams;
    result = globus_ftp_client_operationattr_set_parallelism(
	attr,
	&parallelism);
    if (result != GLOBUS_SUCCESS)
    {
	goto error;
    }

    GlobusXIOGridftpDebugExit();
    return GLOBUS_SUCCESS;

error:	
    GlobusXIOGridftpDebugExitWithError();
    return result;
}	


static
globus_result_t
globus_l_xio_gridftp_attr_cntl(
    void *                              driver_attr,
    int                                 cmd,
    va_list                             ap)
{
    globus_result_t			result;
    globus_l_xio_gridftp_attr_t *       attr;
    GlobusXIOName(globus_l_xio_gridftp_attr_cntl);

    GlobusXIOGridftpDebugEnter();

    attr = (globus_l_xio_gridftp_attr_t *) driver_attr;

    switch(cmd)
    {

	case GLOBUS_XIO_GRIDFTP_SET_HANDLE:
	    attr->ftp_handle = va_arg(ap, globus_ftp_client_handle_t);
	    /* 
	     * If user sets the ftp_handle, i do not use attr->ftp_handle_attr,
	     * it will remain NULL. (Note: The user has to call the attr_cntl 
	     * before doing xio_open)
	     */	 
	    break;

	case GLOBUS_XIO_GRIDFTP_GET_HANDLE:
	{
	    globus_ftp_client_handle_t * ftp_handle;
	    ftp_handle = va_arg(ap, globus_ftp_client_handle_t*);
	    *ftp_handle = attr->ftp_handle;
	    break;
	}
	/* Each read/write maps to a single partial xfer */ 
	case GLOBUS_XIO_GRIDFTP_SET_PARTIAL_TRANSFER:
	    attr->partial_xfer = GLOBUS_TRUE;
	    result = globus_ftp_client_operationattr_set_read_all(
			&attr->ftp_operation_attr, 
			GLOBUS_TRUE, 
			GLOBUS_NULL,
			GLOBUS_NULL);
	    if (result != GLOBUS_SUCCESS)
	    {
 	        result = GlobusXIOErrorWrapFailed(
		    "globus_ftp_client_operationattr_set_read_all", result);
	        goto error;	
	    }
	    break;
	case GLOBUS_XIO_GRIDFTP_GET_PARTIAL_TRANSFER:
	{
	    globus_bool_t * partial_xfer;
	    partial_xfer = va_arg(ap, globus_bool_t*);	
	    *partial_xfer = attr->partial_xfer;
	    break;
	}    
	case GLOBUS_XIO_GRIDFTP_SET_NUM_STREAMS:
	{
	    int num_streams;
	    num_streams = va_arg(ap, int);
	    result = globus_i_xio_gridftp_set_parallelism(
		&attr->ftp_operation_attr, num_streams);
	    if (result != GLOBUS_SUCCESS)
	    {
 	        result = GlobusXIOErrorWrapFailed(
		    "globus_i_xio_gridftp_set_parallelism", result);
	        goto error;	
	    }
	    break;
	}

	case GLOBUS_XIO_GRIDFTP_GET_NUM_STREAMS:
	{
	    globus_ftp_control_parallelism_t parallelism;
	    int * num_streams = va_arg(ap, int*);
	    result = globus_ftp_client_operationattr_get_parallelism(
		&attr->ftp_operation_attr,
		&parallelism);
	    if (result != GLOBUS_SUCCESS)
	    {
 	        result = GlobusXIOErrorWrapFailed(
		    "globus_ftp_client_operationattr_get_parallelism", result);
	        goto error;	
	    }
	    *num_streams = parallelism.fixed.size;
	    break;
	}

	case GLOBUS_XIO_GRIDFTP_SET_TCP_BUFFER:
	{
	    /*
	     * typedef union globus_ftp_control_tcpbuffer_t
	     * {
	     *    globus_ftp_control_tcpbuffer_mode_t         mode;
	     *    globus_ftp_control_tcpbuffer_default_t      default_tcpbuffer;
	     *    globus_ftp_control_tcpbuffer_fixed_t        fixed;
	     *    globus_ftp_control_tcpbuffer_automatic_t    automatic;
	     * } globus_ftp_control_tcpbuffer_t;
	     */
	    globus_ftp_control_tcpbuffer_t tcp_buffer;
            tcp_buffer.mode = GLOBUS_FTP_CONTROL_TCPBUFFER_FIXED;
            tcp_buffer.fixed.size = va_arg(ap, int);
            result = globus_ftp_client_operationattr_set_tcp_buffer(
                &attr->ftp_operation_attr,
                &tcp_buffer);
	    if (result != GLOBUS_SUCCESS)
	    {
 	        result = GlobusXIOErrorWrapFailed(
		    "globus_ftp_client_operationattr_set_tcp_buffer", result);
	        goto error;	
	    }
	    break;
	}

	case GLOBUS_XIO_GRIDFTP_GET_TCP_BUFFER:
	{
	    globus_ftp_control_tcpbuffer_t tcp_buffer;
	    int * buf_size = va_arg(ap, int*);
	    result = globus_ftp_client_operationattr_get_tcp_buffer(
		&attr->ftp_operation_attr,
		&tcp_buffer);
	    if (result != GLOBUS_SUCCESS)
	    {
 	        result = GlobusXIOErrorWrapFailed(
		    "globus_ftp_client_operationattr_get_tcp_buffer", result);
	        goto error;	
	    }
	    *buf_size = tcp_buffer.fixed.size;
	    break;
	}
	/* 
	 * Type, Mode etc are defined as enums in globus_xio_gridftp_driver.h
	 */
	case GLOBUS_XIO_GRIDFTP_SET_TYPE:
	    result = globus_ftp_client_operationattr_set_type(
		&attr->ftp_operation_attr,
		va_arg(ap, int));
	    if (result != GLOBUS_SUCCESS)
	    {
 	        result = GlobusXIOErrorWrapFailed(
		    "globus_ftp_client_operationattr_set_type", result);
	        goto error;	
	    }
	    break;

	case GLOBUS_XIO_GRIDFTP_GET_TYPE:
	{
	    globus_ftp_control_type_t ftp_type;
	    int * type;
	    type = va_arg(ap, int*);
	    result = globus_ftp_client_operationattr_get_type(
		&attr->ftp_operation_attr,
		&ftp_type);
	    if (result != GLOBUS_SUCCESS)
	    {
 	        result = GlobusXIOErrorWrapFailed(
		    "globus_ftp_client_operationattr_get_type", result);
	        goto error;	
	    }
	    *type = ftp_type;
	    break;
	}
	case GLOBUS_XIO_GRIDFTP_SET_MODE:
	    result = globus_ftp_client_operationattr_set_mode(
		&attr->ftp_operation_attr,
		va_arg(ap, int));
	    if (result != GLOBUS_SUCCESS)
	    {
 	        result = GlobusXIOErrorWrapFailed(
		    "globus_ftp_client_operationattr_set_mode", result);
	        goto error;	
	    }
	    break;

	case GLOBUS_XIO_GRIDFTP_GET_MODE:
	{
	    globus_ftp_control_type_t ftp_mode;
	    int * mode;
	    mode = va_arg(ap, int*);
	    result = globus_ftp_client_operationattr_get_mode(
		&attr->ftp_operation_attr,
		&ftp_mode);
	    if (result != GLOBUS_SUCCESS)
	    {
 	        result = GlobusXIOErrorWrapFailed(
		    "globus_ftp_client_operationattr_get_mode", result);
	        goto error;	
	    }
	    *mode = ftp_mode;
	    break;
	}
	case GLOBUS_XIO_GRIDFTP_SET_AUTH:
	    result = globus_i_xio_gridftp_set_authorization(
		&attr->ftp_operation_attr, 
		ap);
	    if (result != GLOBUS_SUCCESS)
	    {
 	        result = GlobusXIOErrorWrapFailed(
		    "globus_i_xio_gridftp_set_authorization", result);
	        goto error;	
	    }
	    break;

	case GLOBUS_XIO_GRIDFTP_GET_AUTH:
	{
	    gss_cred_id_t credential;
	    result = globus_ftp_client_operationattr_get_authorization(
		&attr->ftp_operation_attr,
		&credential,
		va_arg(ap, char**),
		va_arg(ap, char**),
		va_arg(ap, char**),
		va_arg(ap, char**));
	    if (result != GLOBUS_SUCCESS)
	    {
 	        result = GlobusXIOErrorWrapFailed(
		    "globus_ftp_client_operationattr_get_authorization", 
		    result);
	        goto error;	
	    }
	    break;
	}

	case GLOBUS_XIO_GRIDFTP_SET_DCAU:
	{
	    globus_ftp_control_dcau_t dcau;
	    dcau.mode = va_arg(ap, int);	
	    result = globus_ftp_client_operationattr_set_dcau(
		&attr->ftp_operation_attr,
		&dcau);
	    if (result != GLOBUS_SUCCESS)
	    {
 	        result = GlobusXIOErrorWrapFailed(
		    "globus_ftp_client_operationattr_set_dcau", result);
	        goto error;	
	    }
	    break;
	}

	case GLOBUS_XIO_GRIDFTP_GET_DCAU:
	{
	    globus_ftp_control_dcau_t dcau;
	    int *dcau_mode = va_arg(ap, int*);
	    result = globus_ftp_client_operationattr_get_dcau(
		&attr->ftp_operation_attr,
		&dcau);
	    if (result != GLOBUS_SUCCESS)
	    {
 	        result = GlobusXIOErrorWrapFailed(
		    "globus_ftp_client_operationattr_get_dcau", result);
	        goto error;	
	    }
	    *dcau_mode = dcau.mode;
	    break;
	}

	case GLOBUS_XIO_GRIDFTP_SET_DATA_PROTECTION:
	    result = globus_ftp_client_operationattr_set_data_protection(
		&attr->ftp_operation_attr,
		va_arg(ap, int));
	    if (result != GLOBUS_SUCCESS)
	    {
 	        result = GlobusXIOErrorWrapFailed(
		    "globus_ftp_client_operationattr_set_data_protection", 
		    result);
	        goto error;	
	    }
	    break;

    	case GLOBUS_XIO_GRIDFTP_GET_DATA_PROTECTION:
	{
	    globus_ftp_control_protection_t ftp_protection;
	    int * protection;
	    protection = va_arg(ap, int*);
	    result = globus_ftp_client_operationattr_get_data_protection(
		&attr->ftp_operation_attr,
		&ftp_protection);
	    if (result != GLOBUS_SUCCESS)
	    {
 	        result = GlobusXIOErrorWrapFailed(
		    "globus_ftp_client_operationattr_get_data_protection", 
		    result);
	        goto error;	
	    }
	    *protection = ftp_protection;
	    break;
	}
    	case GLOBUS_XIO_GRIDFTP_SET_CONTROL_PROTECTION:
	    result = globus_ftp_client_operationattr_set_control_protection(
		&attr->ftp_operation_attr,
		va_arg(ap, int));
	    if (result != GLOBUS_SUCCESS)
	    {
 	        result = GlobusXIOErrorWrapFailed(
		    "globus_ftp_client_operationattr_set_control_protection", 
		    result);
	        goto error;	
	    }
	    break;

    	case GLOBUS_XIO_GRIDFTP_GET_CONTROL_PROTECTION:
	{
	    globus_ftp_control_protection_t ftp_protection;
	    int * protection;
	    protection = va_arg(ap, int*);
	    result = globus_ftp_client_operationattr_get_control_protection(
		&attr->ftp_operation_attr,
		&ftp_protection);
	    if (result != GLOBUS_SUCCESS)
	    {
 	        result = GlobusXIOErrorWrapFailed(
		    "globus_ftp_client_operationattr_get_control_protection", 
		    result);
	        goto error;	
	    }
	    *protection = ftp_protection;
	    break;
	}
	default:
	   result = GlobusXIOErrorInvalidCommand(cmd);
	   goto error;
    }	

    GlobusXIOGridftpDebugExit();
    return GLOBUS_SUCCESS;

error:	
    GlobusXIOGridftpDebugExitWithError();
    return result;
}


static
globus_result_t
globus_l_xio_gridftp_attr_copy(
    void **                             dst,
    void *                              src)
{
    globus_l_xio_gridftp_attr_t *       src_attr;
    globus_l_xio_gridftp_attr_t *       dst_attr;
    globus_result_t                     result;
    GlobusXIOName(globus_l_xio_gridftp_attr_copy);

    GlobusXIOGridftpDebugEnter();

    dst_attr = (globus_l_xio_gridftp_attr_t *) 
		globus_malloc(sizeof(globus_l_xio_gridftp_attr_t));
    if(!dst_attr)
    {
        result = GlobusXIOErrorMemory("attr");
        goto error_dst_attr;
    }
    src_attr = (globus_l_xio_gridftp_attr_t *) src;
    memcpy(dst_attr, src_attr, sizeof(globus_l_xio_gridftp_attr_t)); 

    result = globus_ftp_client_operationattr_copy(
	&dst_attr->ftp_operation_attr, &src_attr->ftp_operation_attr);
    if (result != GLOBUS_SUCCESS)
    {
        result = GlobusXIOErrorWrapFailed(
            "globus_ftp_client_operationattr_copy", result);
	goto error_operationattr_copy;
    }


    /*
     * if there is any ptr in the attr structure do attr->xptr =
     * globus_libc_strdup(attr->xptr) and do if (!attr->xptr) { result =
     * GlobusXIOErrorMemory("xptr"); goto error_xptr; }
     */

    *dst = dst_attr;

    GlobusXIOGridftpDebugExit();
    return GLOBUS_SUCCESS;

error_operationattr_copy:
    globus_free(dst_attr);
error_dst_attr:
    GlobusXIOGridftpDebugExitWithError();
    return result;
}


static
globus_result_t
globus_l_xio_gridftp_attr_destroy(
    void *                              driver_attr)
{
    globus_l_xio_gridftp_attr_t *       attr;
    GlobusXIOName(globus_l_xio_gridftp_attr_destroy);

    GlobusXIOGridftpDebugEnter();

    attr = (globus_l_xio_gridftp_attr_t *) driver_attr;
    globus_free(attr);

    GlobusXIOGridftpDebugExit();
    return GLOBUS_SUCCESS;
}


static
globus_result_t
globus_l_xio_gridftp_init(
    globus_xio_driver_t *               out_driver)
{
    globus_xio_driver_t                 driver;
    globus_result_t                     result;
    GlobusXIOName(globus_l_xio_gridftp_init);

    GlobusXIOGridftpDebugEnter();

    /* I dont support any driver options, so I'll ignore the ap */

    result = globus_xio_driver_init(&driver, "gridftp", GLOBUS_NULL);
    if(result != GLOBUS_SUCCESS)
    {
        result = GlobusXIOErrorWrapFailed(
            "globus_l_xio_driver_init", result);
        goto error_init;
    }

    /*
     * As this driver makes use of ftp client library, this driver is a 
     * transport driver
     */

    globus_xio_driver_set_transport(
        driver,
        globus_l_xio_gridftp_open,
        globus_l_xio_gridftp_close,
        globus_l_xio_gridftp_read,
        globus_l_xio_gridftp_write,
        globus_l_xio_gridftp_cntl);

    globus_xio_driver_set_attr(
        driver,
        globus_l_xio_gridftp_attr_init,
        globus_l_xio_gridftp_attr_copy,
        globus_l_xio_gridftp_attr_cntl,
        globus_l_xio_gridftp_attr_destroy);

    *out_driver = driver;

    GlobusXIOGridftpDebugExit();
    return GLOBUS_SUCCESS;

error_init:
    GlobusXIOGridftpDebugExitWithError();
    return result;
}

static
void
globus_l_xio_gridftp_destroy(
    globus_xio_driver_t                 driver)
{
    globus_xio_driver_destroy(driver);
}


GlobusXIODefineDriver(
    gridftp,
    globus_l_xio_gridftp_init,
    globus_l_xio_gridftp_destroy);

