#include "globus_xio_pass_macro.h"
#include "globus_i_xio.h"

#define GLOBUS_XIO_HANDLE_DEFAULT_OPERATION_COUNT   2
/*
 *  read_op_list adn write_op_list
 *  ------------------------------
 *  list of operations whos callbacks have not yet *returned*.
 *  
 *  cancel walks this list and sets all operations to canceled, 
 *  calling the driver callbck if neccessary.  This goes for ops
 *  that are finsihed and currently in the users callback.  However
 *  this causes no problem because the cancled flag will not be looked
 *  at again, and the no driver will be registered for such csllbacks.
 */
/********************************************************************
 *                   data structure macros
 *******************************************************************/
#define GlobusXIOHandleDestroy(h)                                           \
{                                                                           \
    globus_i_xio_handle_t *                         _h;                     \
                                                                            \
    _h = (h);                                                               \
    globus_assert(_h->ref == 0);                                            \
    globus_mutex_destroy(_h->mutex);                                        \
    globus_memory_destroy(_h->op_memory);                                   \
    globus_free(_h);                                                        \
    h = NULL;                                                               \
}

#define GlobusXIOHandleCreate(h, t)                                         \
{                                                                           \
    globus_i_xio_target_t *                         _t;                     \
    globus_i_xio_handle_t *                         _h;                     \
    globus_i_xio_context_t *                        _c;                     \
                                                                            \
    _t = (t);                                                               \
    _c = (c);                                                               \
                                                                            \
    /* allocate and intialize the handle structure */                       \
    _h = (struct globus_i_xio_handle_s *) globus_malloc(                    \
                    sizeof(globus_i_xio_handle_t));                         \
    if(_h != NULL)                                                          \
    {                                                                       \
        memset(_h, '\0', sizeof(globus_i_xio_handle_t));                    \
        globus_mutex_init(&_h->mutex, NULL);                                \
        /*                                                                  \
         *  initialize memory for the operation structure                   \
         *  The operation is a stretchy array.  The size of the             \
         *  operation structure plus the size of the entry array            \
         */                                                                 \
        globus_memory_init(                                                 \
            &_h->op_memory,                                                 \
            sizeof(globus_i_xio_op_t) +                                     \
                (sizeof(globus_i_xio_op_entry_t) *                          \
                    (_t->stack_size - 1)),                                  \
            GLOBUS_XIO_HANDLE_DEFAULT_OPERATION_COUNT);                     \
        _h->stack_size = _t->stack_size;                                    \
    }                                                                       \
    h = _h;                                                                 \
}

#define GlobusXIOContextDestroy(c)                                          \
do                                                                          \
{                                                                           \
    globus_i_xio_context_t                              _c;                 \
                                                                            \
    _c = (c);                                                               \
    globus_mutex_destroy(_c->mutex);                                        \
    globus_free(_c);                                                        \
} while(0)

#define GlobusXIOContextCreate(c, t)                                        \
do                                                                          \
{                                                                           \
    globus_i_xio_context_t                              _c;                 \
    globus_i_xio_target_t *                             _t;                 \
    int                                                 _size;              \
                                                                            \
    _t = (t);                                                               \
                                                                            \
    _size = sizeof(globus_i_xio_context_t) +                                \
        (sizeof(globus_i_xio_context_entry_t) * (_t->stack_size - 1));      \
                                                                            \
    _c = (globus_i_xio_context_t *) globus_malloc(_size);                   \
    if(_c != NULL)                                                          \
    {                                                                       \
        memset(_c, '\0', size);                                             \
        globus_mutex_init(&_c->mutex, NULL);                                \
        _c->stack_size = _t->stack_size;                                    \
    }                                                                       \
    c = _c;                                                                 \
} while(0)

#define GlobusXIOOperationCreate(op, type, h, s)                            \
do                                                                          \
{                                                                           \
    globus_i_xio_op_t *                                 _op;                \
    globus_i_xio_handle_t *                             _h;                 \
                                                                            \
    _h = (h);                                                               \
    /* create operation for the open */                                     \
    _h->ref++;                                                              \
    _op = (globus_i_xio_operation_t * )                                     \
            globus_memory_pop_node(&_h->op_memory);                         \
    memset(_op, '\0', sizeof(globus_i_xio_op_t));                           \
                                                                            \
    _op->op_type = (type);                                                  \
    _op->state = GLOBUS_XIO_OP_STATE_OPERATING;                             \
/*  _op->data_cb = NULL;    */                                              \
/*  _op->iovec_cb = NULL;   */                                              \
/*  _op->cb = NULL;                   */                                    \
/*  _op->progress = GLOBUS_TRUE;      */                                    \
/*  _op->timeout_cb = NULL;           */                                    \
/*  _op->ref = 0;                     */                                    \
    _op->xio_handle = _h;                                                   \
    _op->space = (s);                                                       \
/*  _op->cached_res = GLOBUS_SUCCESS; */                                    \
    _op->stack_size = _h->stack_size;                                       \
    _op->context = _h->context;                                             \
    _op->data_desc = _h->data_desc;                                         \
/*  _op->ndx = 0;                    */                                     \
    globus_callback_space_reference(_op->space);                            \
    op = _op;                                                               \
} while(0)

#define GlobusXIOOperationDestroy(op)                                       \
do                                                                          \
{                                                                           \
    globus_i_xio_operation_t *                      _op;                    \
                                                                            \
    _op = (op);                                                             \
    globus_assert(_op->ref == 0);                                           \
    globus_callback_space_destroy(_op->space);                              \
    globus_memory_push_node(&_op->xio_handle->op_memory, _op);              \
} while(0)

#define GlobusIXIOHandleDec(free, h)                                    \
{                                                                       \
    globus_i_xio_handle_t *                         _h;                 \
                                                                        \
    _h = (h);                                                           \
    _h->ref--;                                                          \
    if(_h->ref == 0)                                                    \
    {                                                                   \
        /* if the handle ref gets down to zero we must be in one        \
         * of the followninf staes.  The statement is that the handle   \
         * only goes away when it is closed or a open fails             \
         */                                                             \
        globus_assert(                                                  \
            xio_handle->state == GLOBUS_XIO_HANDLE_STATE_OPEN_FAILED || \
            xio_handle->state == GLOBUS_XIO_HANDLE_STATE_CLOSED);       \
        free = GLOBUS_TRUE;                                             \
    }                                                                   \
    else                                                                \
    {                                                                   \
        free = GLOBUS_FALSE;                                            \
    }                                                                   \
}
/********************************************************************
 *                      Internal functions 
 *******************************************************************/

/*
 *  kickout for the close operation.
 */
void
globus_l_xio_start_close_failure_kickout(
    void *                                      user_arg)
{
    globus_i_xio_operation_t *                  op;

    op = (globus_i_xio_operation_t *) user_arg;

    globus_i_xio_open_close_callback(
        op,
        op->cached_res,
        op->user_arg);
}

/*
 *  This is called when either an open or a close completes.
 */
void
globus_i_xio_open_close_callback(
    globus_i_xio_op_t *                         xio_op,
    globus_result_t                             result,
    void *                                      user_arg)
{
    globus_bool_t                               destroy_handle = GLOBUS_FALSE;
    globus_i_xio_handle_t *                     xio_handle;
    globus_bool_t                               fire_callback;

    xio_handle = xio_op->handle;

    globus_mutex_lock(&xio_handle->mutex);
    {   
        /* insist the proper state */
        assert(xio_handle->state == GLOBUS_XIO_HANDLE_STATE_OPENING || 
                xio_handle->state == GLOBUS_XIO_HANDLE_STATE_CLOSING);

        /*
         *  if an error occured we will set both to FAILED.  This introduces
         *  the possibility of a FAILED in a state machine that was not
         *  opening or closing, but this is ok.
         */
        if(result != GLOBUS_SUCCESS)
        {
            xio_handle->state = GLOBUS_XIO_HANDLE_STATE_CLOSED;
        }
        else
        {
            case(xio_handle->state)
            {
                case GLOBUS_XIO_HANDLE_STATE_OPENING:
                    xio_handle->state = GLOBUS_XIO_HANDLE_STATE_OPEN;
                    break;

                case GLOBUS_XIO_HANDLE_STATE_CLOSING:
                    xio_handle->state = GLOBUS_XIO_HANDLE_STATE_CLOSED;
                    break;

                case GLOBUS_XIO_HANDLE_STATE_DELIVERED_AND_CLOSING:
                    xio_handle->state = GLOBUS_XIO_HANDLE_STATE_CLOSED;
                    break;

                default:
                    assert(0);
                    break;
            }
        }

        xio_op->state = GLOBUS_XIO_OP_STATE_FINISH_WAITING;
        /* set to finished for the sake of the timeout */
        if(xio_op->state == GLOBUS_XIO_OP_STATE_TIMEOUT_PENDING)
        {
            fire_operation = GLOBUS_FALSE;
        }
        else
        {
            fire_operation = GLOBUS_TRUE;
            if(xio_op->timeout_cb != NULL)
            {
                /* 
                 * unregister the cancel
                 */
                /* if the unregister fails we will get the callback */
                if(globus_i_xio_timer_unregister_timeout(xio_op))
                {
                    /* at this point we know timeout won't happen */
                    xio_op->ref--;
                    /* since we have no yet deced for the callbacl this
                       cannot be zero */
                    globus_assert(xio_op->ref > 0);
                }
        }

        /* remove the operation from the list */

        xio_op->cached_res = result;
        /* 
         *  when at the top don't worry about the cancel
         *  just act as though we missed it
         */
    }
    globus_mutex_unlock(&xio_handle->mutex);

    if(fire_callback)
    {
        /*
         *  if in a space or within the register call stack
         *  we must register a one shot
         */
        if(xio_op->space != GLOBUS_CALLBACK_GLOBAL_SPACE || 
           xio_op->in_register)
        {
            /* register a oneshot callback */
            globus_callback_space_register_oneshot(
                NULL,
                NULL,
                globus_l_xio_open_close_callback_kickout,
                (void *)xio_op,
                xio_op->space);
        } 
        /* in all other cases we can just call callback */
        else
        {
            globus_l_xio_open_close_callback_kickout((void *)xio_op);
        }
    }
}

/*
 *   called by the callback code.
 *   registerd by finished op when the final (user) callback
 *   is in a callback space, or if it is under the registraton
 *   call within the same callstack
 */
void
globus_l_xio_open_close_callback_kickout(
    void *                                      user_arg)
{
    globus_i_xio_op_t *                         xio_op;
    globus_i_xio_handle_t *                     xio_handle;

    xio_op = (globus_i_xio_operation_t *) user_arg;
    xio_handle = op->xio_handle;

    /* call the users callback */
    if(xio_op->cb != NULL)
    {
        xio_op->cb(xio_handle, op->cached_res, op->user_arg);
    }

    globus_mutex_lock(&xio_handle->mutex);
    {
         /* insist the state machine is in the following 2 states */
        globus_assert(xio_handle->state == GLOBUS_XIO_HANDLE_STATE_CLOSED ||
            xio_handle->state == GLOBUS_XIO_HANDLE_STATE_OPEN);
        globus_assert(xio_op->state == GLOBUS_XIO_OP_STATE_FINISH_WAITING);

        /* decrement operations since open or close has finished */
        xio_handle->outstanding_operations--;

        /* this is likely useless, but may help in debugging */
        xio_op->state = GLOBUS_XIO_OP_STATE_FINISHED;
        /* 
         * if in CLOSED state then we are finished with this handle.
         */
        if(xio_handle->state == GLOBUS_XIO_HANDLE_STATE_CLOSED)
        {
            GlobusIXIOHandleDec(destroy_handle, xio_handle);
            /* destroy handle cannot possibly be true yet 
                the handle stll has the operation reference */
            globus_assert(!destroy_handle);

            /* we can remove both open and close since this branch
               only enters if open failed or close happened.  in either
               case both are removed */
            xio_op->close_op = NULL;
            xio_op->open_op = NULL;
        }
        else if(xio_handle->state == GLOBUS_XIO_HANDLE_STATE_OPENED)    
        {
            /* this state is the only other choice, remove open op */
            xio_op->open_op = NULL;
        }

        /* decrement reference for the operation */
        xio_op->ref--;
        if(xio_op->ref == 0)
        {
            GlobusXIOOperationDestroy(xio_op);
            GlobusIXIOHandleDec(destroy_handle, xio_handle);
        }

    }
    globus_mutex_unlock(&xio_handle->mutex);

    if(destroy_handle)
    {
        GlobusXIOHandleDestroy(xio_handle);
    }
}

/*
 *  operation callback for readv and writev operations
 *  we don't care what the result is, just so it bubbles up to the user
 */
void
globus_i_xio_write_callback(
    globus_xio_driver_operation_t               op,
    globus_result_t                             result,
    globus_size_t                               nbytes,
    void *                                      user_arg)
{
    globus_bool_t                               destroy_handle = GLOBUS_FALSE;
    globus_i_xio_handle_t *                     xio_handle;
    globus_bool_t                               fire_operation = GLOBUS_TRUE;

    xio_handle = op->xio_handle;

    globus_mutex_lock(&xio_handle->mutex);
    {
        /* 
         *  this can happen in any state except for OPENING and CLOSED
         */
        assert(xio_handle->state != GLOBUS_XIO_HANDLE_STATE_OPENING &&
            xio_handle->state != GLOBUS_XIO_HANDLE_STATE_CLOSED);

        xio_op->state = GLOBUS_XIO_OP_STATE_FINISH_WAITING;
        /* set to finished for the sake of the timeout */
        if(xio_op->state == GLOBUS_XIO_OP_STATE_TIMEOUT_PENDING)
        {
            fire_operation = GLOBUS_FALSE;
        }
        else
        {
            fire_operation = GLOBUS_TRUE;
            if(op->timeout_cb != NULL)
            {
                /* 
                 * unregister the cancel
                 */
                op->timeout_set = GLOBUS_FALSE;

                /* if the unregister fails we will get the callback */
                if(globus_i_xio_timer_unregister_timeout(op))
                {
                    /* at this point we know timeout won't happen */
                    op->ref--;
                }
            }
        }

        op->cached_res = result;
        op->nbytes = nbytes;
    }   
    globus_mutex_unlock(&xio_handle->mutex);

    /*
     *  if in a space or within the register call stack
     *  we must register a one shot
     */
    if(fire_operation)
    {
        if(op->space != GLOBUS_CALLBACK_GLOBAL_SPACE ||
           op->in_register)
        {
            /* register a oneshot callback */
            globus_callback_space_register_oneshot(
                NULL,
                NULL,
                globus_l_xio_write_callback_kickout,
                (void *)op,
                op->space);
        } 
        /* in all other cases we can just call callback */
        else
        {
            globus_l_xio_write_callback_kickout((void *)op);
        }
    }
}

/*
 *  called unlocked either by the callback code or in the finsihed op
 *  state.
 */
void
globus_l_xio_write_callback_kickout(
    void *                                      user_arg)
{
    globus_i_xio_operation_t *                  op;
    globus_i_xio_handle_t *                     xio_handle;

    op = (globus_i_xio_operation_t *) user_arg;
    xio_handle = op->xio_handle;

    /* call the users callback */
    if(op->data_cb != NULL)
    {
        op->data_cb(
            xio_handle, 
            op->cached_res, 
            op->mem_iovec.iov_base,
            op->mem_iovec.iov_len,
            op->nbytes,
            op->user_arg);
    }
    else if(op->iovec_cb != NULL)
    {
        op->iovec_cb(
            xio_handle, 
            op->cached_res, 
            op->iovec,
            op->iovec_count,
            op->nbytes,
            op->user_arg);
    }

    globus_mutex_lock(&xio_handle->mutex);
    {
        /*
         *  This is legit in all states except for OPENING and CLOSED
         */
        assert(xio_handle->state != GLOBUS_XIO_HANDLE_STATE_CLOSED &&
            xio_handle->state != GLOBUS_XIO_HANDLE_STATE_OPENING);
        /* decrement reference for the operation */
        op->ref--;
        if(op->ref == 0)
        {
            GlobusXIOOperationDestroy(op);
            /* remove refrence for operation */
            GlobusIXIOHandleDec(destroy_handle, xio_handle);
            /* destroy handle cannot possibly be true yet 
                the handle stll has its own reference */
            assert(!destroy_handle);
        }

        /* remove from the write list, since user was notified they can
           not longer cancel it */
        globus_list_remove(&xio_handle->write_op_list,
            globus_list_search(xio_handle->write_op_list, op));
        /* remove the operation refernce */
        xio_handle->outstanding_operations--;

        /* 
         *  if there are no more outstanding operations and in the
         *  CLOSING state then allow the close operation to continue.
         */
        if(xio_handle->state == GLOBUS_XIO_HANDLE_STATE_CLOSING &&
           xio_handle->state ==
                GLOBUS_XIO_HANDLE_STATE_EOF_DELIVERED_AND_CLOSING &&
           xio_handle->outstanding_operations == 0)
        {
            /* insist that we have a close operation and that both op
               lists are empty. */
            assert(xio_handle->close_op != NULL);

            globus_l_xio_start_close(xio_handle->close_op);
        }
    }
    globus_mutex_unlock(&xio_handle->mutex);
}

/*
 *  operation callback for readv and writev operations
 *  we don't care what the result is, just so it bubbles up to the user
 */
void
globus_i_xio_read_callback(
    globus_xio_driver_operation_t               op,
    globus_result_t                             result,
    globus_size_t                               nbytes,
    void *                                      user_arg)
{
    globus_bool_t                               fire_callback = GLOBUS_TRUE;
    globus_bool_t                               destroy_handle = GLOBUS_FALSE;
    globus_i_xio_handle_t *                     xio_handle;

    xio_handle = op->xio_handle;

    globus_mutex_lock(&xio_handle->mutex);
    {
        /* 
         *  this callback should only happen when a a handle is open
         *  or the handle is waiting to close.  it should not happen
         *  once EOF has been delivered.
         */
        assert(xio_handle->read_state != GLOBUS_XIO_HANDLE_STATE_OPENING &&
            xio_handle->read_state != GLOBUS_XIO_HANDLE_STATE_CLOSED &&
            xio_handle->read_state != GLOBUS_XIO_HANDLE_STATE_EOF_DELIVERED &&
            xio_handle->read_state != 
                GLOBUS_XIO_HANDLE_STATE_EOF_DELIVERED_AND_CLOSING);

       xio_op->state = GLOBUS_XIO_OP_STATE_FINISH_WAITING;
        /* set to finished for the sake of the timeout */
        if(xio_op->state == GLOBUS_XIO_OP_STATE_TIMEOUT_PENDING)
        {
            fire_operation = GLOBUS_FALSE;
        }
        else
        {
            fire_operation = GLOBUS_TRUE;
            if(op->timeout_cb != NULL)
            {
                /* 
                 * unregister the cancel
                 */
                op->timeout_set = GLOBUS_FALSE;
    
                /* if the unregister fails we will get the callback */
                if(globus_i_xio_timer_unregister_timeout(op))
                {
                    /* at this point we know timeout won't happen */
                    op->ref--;
                    globus_assert(op->ref > 0);
                }
            }
        }

        /*
         *  if we have encountered a read eof we delay the callback
         *  until all outstanding callbacks have been delivered.
         *  The timeout on the first EOF will be removed.
         *
         *  all operations from driver that come back with a EOF
         *  will be added to the eof list and removed from the
         *  read op list.  once the read op list is empty we will
         *  purge the eof list.
         */
        if(GlobusXIOIsEOF(result))
        {
            /* set next state */
            switch(xio_handle->state)
            {
                case GLOBUS_XIO_HANDLE_STATE_OPEN:
                    xio_handle->state = GLOBUS_XIO_HANDLE_STATE_EOF_RECEIVED;
                    break;

                case GLOBUS_XIO_HANDLE_STATE_CLOSING:
                    xio_handle->state = 
                        GLOBUS_XIO_HANDLE_STATE_EOF_RECEIVED_AND_CLOSING;
                    break;

                /* posbile that his already happened */
                case GLOBUS_XIO_HANDLE_STATE_EOF_RECEIVED_AND_CLOSING:
                case GLOBUS_XIO_HANDLE_STATE_EOF_RECEIVED:
                    break;

                default:
                    assert(GLOBUS_FALSE);
                    break;
            }

            op->op_type = GLOBUS_XIO_OPERATION_TYPE_EOF;

            /* remove from the main list and add to EOF list */
            globus_list_remove(&xio_handle->read_op_list,
                globus_list_search(xio_handle->read_op_list, op));

            /* if we have outstanding operations we may not fire thihs one */
            if(fire_callback && !globus_list_empty(xio_handle->read_op_list))
            {
                /* only add it if we are not going to call it.  Other
                    wise it could get fired again in purge */
                globus_list_insert(&xio_handle->read_eof_list, op);
                fire_callback = GLOBUS_FALSE;
            }
        }

        op->cached_res = result;
        op->nbytes = nbytes;
    }   
    globus_mutex_unlock(&xio_handle->mutex);

    /*
     *  if in a space or within the register call stack
     *  we must register a one shot
     */
    if(fire_callback)
    {
        if(op->space != GLOBUS_CALLBACK_GLOBAL_SPACE ||
           op->in_register)
        {
            /* register a oneshot callback */
            globus_callback_space_register_oneshot(
                NULL,
                NULL,
                globus_l_xio_read_callback_kickout,
                (void *)op,
                op->space);
        } 
        /* in all other cases we can just call callback */
        else
        {
            globus_l_xio_read_callback_kickout((void *)op);
        }
    }
}

/*
 *  called unlocked either by the callback code or in the finsihed op
 *  state.
 */
void
globus_l_xio_read_callback_kickout(
    void *                                      user_arg)
{
    globus_i_xio_operation_t *                  op;
    globus_i_xio_handle_t *                     xio_handle;
    globus_bool_t                               purge;

    op = (globus_i_xio_operation_t *) user_arg;
    xio_handle = op->xio_handle;

    /* call the users callback */
    if(op->data_cb != NULL)
    {
        op->data_cb(
            xio_handle, 
            op->cached_res, 
            op->mem_iovec.iov_base,
            op->mem_iovec.iov_len,
            op->nbytes,
            op->user_arg);
    }
    else if(op->iovec_cb != NULL)
    {
        op->iovec_cb(
            xio_handle, 
            op->cached_res, 
            op->iovec,
            op->iovec_count,
            op->nbytes,
            op->user_arg);
    }

    globus_mutex_lock(&xio_handle->mutex);
    {
        /* eof is received so if we haven't already we need to change state */
        if(op->op_type == GLOBUS_XIO_OPERATION_TYPE_EOF)
        {
            purge = GLOBUS_FALSE;
            switch(xio_handle->state)
            {
                case GLOBUS_XIO_HANDLE_STATE_EOF_RECEIVED:
                    purge = GLOBUS_TRUE;
                    xio_handle->state = GLOBUS_XIO_HANDLE_STATE_EOF_DELIVERED;
                    break;

                case GLOBUS_XIO_HANDLE_STATE_EOF_RECEIVED_AND_CLOSING:
                    purge = GLOBUS_TRUE;
                    xio_handle->state = 
                        GLOBUS_XIO_HANDLE_STATE_EOF_DELIVERED_AND_CLOSING;
                    break;

                /* this case happens if we already flipped the state */
                case GLOBUS_XIO_HANDLE_STATE_EOF_DELIVERED_AND_CLOSING:
                case GLOBUS_XIO_HANDLE_STATE_EOF_DELIVERED:
                    break;

                default:
                    assert(0);
                    break;
            }

            /* if we get an operation with EOF type we definitly must
               have an empty read_op_list */
            globus_assert(globus_list_empty(xio_handle->read_op_list));
        }
        /* if not eof remove from the other list  */
        else
        {
            globus_list_remove(&xio_handle->read_op_list,
                globus_list_search(xio_handle->read_op_list, op));
        }
        /*
         *  this funciton can only be called with the handle in
         *  one of the following states.
         */
        globus_assert(xio_handle->state == GLOBUS_XIO_HANDLE_STATE_CLOSING ||
            xio_handle->state == GLOBUS_XIO_HANDLE_STATE_OPEN);
        /* decrement reference for the operation */
        op->ref--;
        if(op->ref == 0)
        {
            GlobusXIOOperationDestroy(op);
            /* remove refrence for operation */
            GlobusIXIOHandleDec(destroy_handle, xio_handle);
            /* destroy handle cannot possibly be true yet 
                the handle stll has its own reference */
            globus_assert(!destroy_handle);
        }

        if(globus_list_empty(xio_handle->read_op_list))
        {
            purge = GLOBUS_TRUE;
        }

        /* get rid of this op reference.  the operations in the purge list
           count in this value so i close will not happen until all of them
        return as well. */
        xio_handle->outstanding_operations--;
        /* 
         *  if the operation list is empty and we are in a 
         *  CLOSING state then allow the close operation to continue.
         */
        if(xio_handle->state == GLOBUS_XIO_HANDLE_STATE_CLOSING &&
           xio_handle->state ==
                GLOBUS_XIO_HANDLE_STATE_EOF_DELIVERED_AND_CLOSING &&
           xio_handle->outstanding_operations == 0)
        {
            /* insist that close op has been registered */
            globus_assert(xio_handle->close_op != NULL);

            globus_l_xio_start_close(xio_handle->close_op);
        }

        if(purge)
        {
            while(!globus_list_empty(xio_handle->read_eof_list))
            {
                tmp_op = (globus_i_xio_operation_t *)
                            globus_list_remove(&xio_handle->read_eof_list,
                                xio_handle->read_eof_list);

                /*
                 * LAZY:
                 *  We could call directly here once the lock is releazed
                 *  but benefit is debatable and this is good enough
                 *  for now.
                 *
                 *  To get around the callback code we would have to 
                 *  remove them all from the list and add then to another
                 *  and then walk that list outside of the lock and call 
                 *  then all, then again this may not be true...
                 */
                globus_callback_space_register_oneshot(
                    NULL,
                    NULL,
                    globus_l_xio_read_callback_kickout,
                    (void *)tmp_op,
                    tmp_op->space);
            }
        }
    }
    globus_mutex_unlock(&xio_handle->mutex);
}

/*
 *  this starts the cancel processes for the given operation.
 * 
 *  the cancel flag is to true.  every pass checks this flag and
 *  if cancel is true the pass fails and the above driver should
 *  do what it needs to do to cancel the operation.  If a driver
 *  is able to cancel it will call GlobusXIODriverCancelEanble()
 *  if at the time this function is called the operations cancel
 *  flag is set to true we register a oneshot for the cancel.
 * 
 *  If a cencel occurs while a driver is registered to receive 
 *  cancel notification then the callback is delivered to it.
 *
 *  The framework has little else to do with cancel.  The operation
 *  will come back up via the normal routes with an error.
 */
globus_result_t
globus_l_xio_operation_cancel(
    globus_i_xio_operation_t *                  op)
{
    globus_bool_t                               tmp_rc;

    /* internal function should never be passed NULL */
    globus_assert(op != NULL);

    if(op->cancel)
    {
        return GLOBUS_SUCCESS;
    }
    /* 
     * if the user oks the cancel then remove the timeout from 
     * the poller
     */
    tmp_rc = globus_i_xio_timer_unregister_timeout(op);
    /* since in callback this will always be true */
    globus_assert(tmp_rc);

    /*
     * set cancel flag
     * if a driver has a registered callback it will be called
     * if it doesn't the next pass or finished will pick it up
     */
    op->canceled = GLOBUS_TRUE;
    if(op->cancel_callback != NULL)
    {
        op->cancel_callback(op);
    }

    return GLOBUS_SUCCESS;
}

globus_bool_t
globus_l_xio_timeout_callback(
    void *                                      user_arg)
{
    globus_i_xio_op_t *                         xio_op;
    globus_i_xio_server_t *                     xio_server;
    globus_bool_t                               rc;
    globus_bool_t                               fire_callback;
    globus_bool_t                               destroy_handle = GLOBUS_FALSE;
    globus_bool_t                               timeout = GLOBUS_FALSE;
    
    xio_op = (globus_i_xio_op_t *) user_arg;
    xio_handle = xio_op->handle;

    globus_mutex_lock(&xio_handle->mutex);
    {
        switch(xio_op->state)
        {
            /* 
             * this case happens when a open operation first pass fails and 
             * are unable to unregister the timeout and when the operation
             * completes but we are unable to unregister the callback.
             */
            case GLOBUS_XIO_OP_STATE_FINISHED:
            case GLOBUS_XIO_OP_STATE_FINISH_WAITING:

                /* decerement the reference for the timeout callback */
                xio_op->ref--;
                if(xio_op->ref == 0)
                {
                    /* remove the reference for the target on the server */
                    xio_handle->ref--;
                    if(xio_handle->ref == 0)
                    {
                        GlobusXIOOperationDestroy(op);
                        GlobusIXIOHandleDec(destroy_handle, xio_handle);
                    }
                }

                /* remove it from the timeout list */
                rc = GLOBUS_TRUE;
                break;

            /* this case happens when we actually want to cancel the operation
                The timeout code should insure that prograess is false if this
                gets called in this state */
            case GLOBUS_XIO_OP_STATE_OPERATING:
                /* it is up to the timeout callback to set this to true */
                rc = GLOBUS_FALSE;
                /* cancel the sucker */
                globus_assert(!xio_op->progress);
                globus_assert(xio_op->timeout_cb != NULL);

                /* if the driver has blocked the timeout don't call it */
                if(!op->block_timeout)
                {
                    timeout = GLOBUS_TRUE;
                    /* put in canceling state to delay the accept callback */
                    xio_op->state = GLOBUS_XIO_OP_STATE_TIMEOUT_PENDING;
                }
                break;

            /* fail on any ohter case */
            default:
                globus_assert(0);
        }
    }
    globus_mutex_unlock(&xio_handle->mutex);

    /* if in cancel state, verfiy with user that they want to cancel */
    if(timeout)
    {
        cancel = xio_op->timeout_cb(xio_handle);
    }
    /* all non time out casses can just return */
    else
    {
        /* wait until outside of lock to free the handle */
        if(destroy_handle)
        {
            globus_free(xio_handle);
        }
        return rc;
    }

    globus_mutex_lock(&xio_server->mutex);
    {
        /* if canceling set the res and we will remove this timer event */
        if(cancel)
        {
            xio_op->cached_res = GlouxXIOErrorCanceled();
            rc = GLOBUS_TRUE;
            xio_op->canceled = GLOBUS_TRUE;
            if(xio_op->cancel_cb)
            {
                xio_op->cancel_cb(xio_op);
            }
        }

        /* if callback has already arriverd set flag to later
            call accept callback and set rc to remove timed event */
        if(xio_op->state == GLOBUS_XIO_OP_STATE_FINISH_WAITING)
        {
            fire_callback = GLOBUS_TRUE;
            rc = GLOBUS_TRUE;
        }
        /* if no accept is waiting, set state back to operating */
        else
        {
            fire_callback = GLOBUS_FALSE;
            xio_op->state = GLOBUS_XIO_OP_STATE_OPERATING;
        }

        /* if we are remvoing the timed event */
        if(rc)
        {
            /* decremenet the target reference count and insist that it is
               not zero yet */
            xio_op->ref--;
            globus_assert(xio_op->ref > 0);
        }

        /* if the accpet was pending we must call it */
        if(fire_callback)
        {
            switch(xio_op->type)
            {
                case GLOBUS_XIO_OPERATION_TYPE_OPEN:
                case GLOBUS_XIO_OPERATION_TYPE_CLOSE:
                    delayed_cb = globus_l_xio_open_close_callback_kickout;
                    break;

                case GLOBUS_XIO_OPERATION_TYPE_READ:
                    delayed_cb = globus_l_xio_read_callback_kickout;
                    break;

                case GLOBUS_XIO_OPERATION_TYPE_WRITE:
                    delayed_cb = globus_l_xio_read_callback_write;
                    break;

                /* the EOF type may not fire until all read_ops have
                    returned from their callbacks */
                case GLOBUS_XIO_OPERATION_TYPE_EOF:
                    if(!globus_list_empty(xio_handle->read_op_list))
                    {
                        fire_callback = GLOBUS_FALSE;
                    }
                    else
                    {
                        globus_list_remove(&xio_handle->read_eof_list,
                            globus_list_search(xio_handle->read_eof_list, 
                                xio_op));
                        delayed_cb = globus_l_xio_read_callback_write;
                    }
                    break;

                default:
                    globus_assert(0);
                    break;

            }
        }
    }
    globus_mutex_unlock(&xio_server->mutex);

    if(fire_callback)
    {
        if(xio_op->space != GLOBUS_CALLBACK_GLOBAL_SPACE ||
           xio_op->in_register)
        {
            /* register a oneshot callback */
            globus_callback_space_register_oneshot(
                NULL,
                NULL,
                delayed_cb,
                (void *)xio_op,
                xio_op->space);
        }
        /* in all other cases we can just call callback */
        else
        {
            delayed_cb((void *)xio_op);
        }
    }

    return rc;
}

/*
 *  internal open function
 *  ----------------------
 *
 *  we enter this function with a intialized handle.
 *  the handle has a reference to context
 *  
 *  an operation is created.  If a timeout is requested then we associate
 *  a timeout with the operation.  The operation is then passed down the
 *  stack.
 *
 *  handle reference count
 *  ----------------------
 *  - The handle comes in with a reference count of 1 (for its own
 *    existance).  
 *  - The operation is created from the handle.and adds a reference to
 *    the handle.
 *
 *  operation reference count
 *  -------------------------
 *  - the operation gets 1 reference count for itself
 *  - if a timeout is associated with the operation then the reference count
 *    is incremented to 2
 */
globus_result_t
globus_l_xio_register_open(
    globus_i_xio_handle_t *                     xio_handle,
    globus_i_xio_context_t *                    xio_context,
    globus_i_xio_target_t *                     xio_target,
    globus_i_xio_op_t *                         xio_op,
    globus_i_xio_attr_t *                       xio_attr)
{
    globus_i_xio_context_t *                    tmp_context;
    globus_bool_t                               destroy_handle;
    int                                         ctr;
    globus_bool_t                               destroy_handle;

    xio_handle->ref = 2; /* itself, operation */
    xio_handle->context = xio_context;
    xio_handle->state = GLOBUS_XIO_HANDLE_STATE_OPENING;
    /* this is set for the cancel */
    xio_handle->open_op = xio_op;
    xio_handle->outstanding_operations = 1; /* open operation */
    /* set timeout values */
    xio_handle->open_timeout = xio_attr->open_timeout;
    GlobusTimeReltimeCopy(xio_handle->open_timeout_period,                \
        xio_attr->open_timeout_period);
    xio_handle->read_timeout = xio_attr->read_timeout;
    GlobusTimeReltimeCopy(xio_handle->read_timeout_period,                \
        xio_attr->read_timeout_period);
    xio_handle->write_timeout = xio_attr->write_timeout;
    GlobusTimeReltimeCopy(xio_handle->write_timeout_period,               \
        xio_attr->write_timeout_period);
    xio_handle->close_timeout = xio_attr->close_timeout;
    GlobusTimeReltimeCopy(xio_handle->close_timeout_period,               \
        xio_attr->close_timeout_period);

    xio_context->ref = 2; /* 1 for handle one for itself */

    xio_op->_op_handle = xio_handle;
    xio_op->ref = 1;

    /* set entries in structures */
    for(ctr = 0; ctr < xio_handle->stack_size; ctr++)
    {
        xio_context->entry[ctr].driver = xio_target->entry[ctr].driver;

        xio_op->entry[ctr].driver = xio_target->entry[ctr].driver;
        xio_op->entry[ctr].target = xio_target->entry[ctr].target;
        GlobusIXIOAttrGetDS(xio_op->entry[ctr].attr,                    \
            xio_attr, xio_target->entry[ctr].driver);
    }

    /* register timeout */
    if(l_handle->open_timeout_cb != NULL)
    {
        /* op the operatin reference count for this */
        xio_op->ref++;
        xio_op->timeout_cb = l_handle->open_timeout_cb;
        globus_i_xio_timer_register_timeout(
            g_globus_l_xio_timeout_timer,
            xio_op,
            &xio_op->progress,
            globus_l_xio_timeout_callback,
            &l_handle->open_timeout_period);
    }

    /* wrap pass with in_register flag for same stack call backs */
    xio_op->in_register = GLOBUS_TRUE;
    GlobusXIODriverPassOpen(res, tmp_context, \
        globus_i_xio_open_close_callback, cb, user_arg);
    xio_op->in_register = GLOBUS_FALSE;

    if(res != GLOBUS_SUCCESS)
    {
        goto err;
    }

    return GLOBUS_SUCCESS;

    /*
     * error handling 
     */
  err:

    if(globus_i_xio_timer_unregister_timeout(xio_op))
    {
        xio_op->ref--;
    }
    op->ref--;
    if(xio_op->ref == 0)
    {
        GlobusXIOOperationDestroy(xio_op);
        /* remove the operations reference to the handle */
        GlobusIXIOHandleDec(destroy_handle, xio_handle);
        /* handle should always have a reference left at this point */
        globus_assert(!destroy_handle);
    }
    GlobusIXIOHandleDec(destroy_handle, xio_handle);
    if(destroy_handle)
    {
        GlobusXIOHandleDestroy(xio_handle);
    }
    GlobusXIOContextDestroy(xio_context);

    return res;
}

/*
 *  this is called locked
 */
globus_result_t
globus_l_xio_register_writev(
    globus_xio_handle_t                         xio_handle,
    globus_i_xio_operation_t *                  op)
{
    globus_result_t                             res;
    globus_bool_t                               destroy_handle;

    /* register timeout */
    if(xio_handle->write_timeout_cb != NULL)
    {
        /* op the operatin reference count for this */
        op->ref++;
        op->timeout_cb = xio_handle->write_timeout_cb;
        globus_i_xio_timer_register_timeout(
            g_globus_l_xio_timeout_timer,
            op,
            &op->progress,
            globus_l_xio_timeout_callback,
            &l_handle->open_timeout_period);
    }
    globus_list_insert(&xio_handle->write_op_list, op);

    /* wrap pass with in_register flag */
    op->in_register = GLOBUS_TRUE;
    GlobusXIODriverPassWrite(res, op, op->iovec, op->iovec_count,     \
        globus_i_xio_write_callback, (void *)NULL);
    op->in_register = GLOBUS_FALSE;
    if(res != GLOBUS_SUCCESS)
    {
        /* TODO: should remove timeout here */
        goto err;
    }

    /* add outstanding op */
    xio_handle->outstanding_operations++;

    return GLOBUS_SUCCESS;

  err:

    /* in case timeout unregister fails */
    op->op_type = GLOBUS_L_XIO_OPERATION_TYPE_FINISHED;
    /* if we had a timeout, we need to unregister it */
    if(xio_handle->write_timeout_cb != NULL)
    {
        /* if unregister works remove its reference count */
        if(globus_i_xio_timer_unregister_timeout(op))
        {
            op->ref--;
            globus_assert(op->ref > 0);
        }
    }
    /* clean up the operation */
    op->ref--;
    if(op->ref == 0)
    {
        GlobusXIOOperationDestroy(op);
        /* remove the operations reference to the handle */
        GlobusIXIOHandleDec(destroy_handle, xio_handle);
        /* handle should always have a reference left at this point */
        globus_assert(!destroy_handle);
    }

    return res;
}

/*
 *  Internal readv
 */
globus_result_t
globus_l_xio_register_readv(
    globus_xio_handle_t                         xio_handle,
    globus_i_xio_operation_t *                  op)
{
    globus_result_t                             res;
    globus_bool_t                               destroy_handle;

    if(xio_handle->state == GLOBUS_XIO_HANDLE_STATE_EOF_RECEIVED)
    {
        /* since we are already at eof give op null values */
        op->nbytes = 0;
        op->cached_res = GlobusXIOErrorReadEOF();
        globus_list_insert(&xio_handle->eof_op_list, op);
    }
    else
    {
        /* register timeout */
        if(xio_handle->read_timeout_cb != NULL)
        {
            /* op the operatin reference count for this */
            op->ref++;
            op->timeout_cb = xio_handle->read_timeout_cb;
            globus_i_xio_timer_register_timeout(
                g_globus_l_xio_timeout_timer,
                op,
                &op->progress,
                globus_l_xio_timeout_callback,
                &l_handle->open_timeout_period);
        }

        GlobusXIODriverPassRead(res, op, iovec, iovec_count,         \
            globus_i_xio_read_callback, (void *)op);
        if(res != GLOBUS_SUCCESS)
        {
            goto err;
        }
    }
    /* add outstanding op in both of above cases */
    xio_handle->outstanding_operations++;

    return GLOBUS_SUCCESS;

  err:

    /* clean up the operation */
    if(xio_handle->read_timeout_cb != NULL)
    {
        if(globus_i_xio_timer_unregister_timeout(op))
        {
            ref--;
            globus_assert(op->ref > 0);
        }
    }
    op->ref--;
    if(op->ref == 0)
    {
        GlobusXIOOperationDestroy(op);
        /* remove the operations reference to the handle */
        GlobusIXIOHandleDec(destroy_handle, xio_handle);
        /* handle should always have a reference left at this point */
        globus_assert(!destroy_handle);
    }

    return res;
}

/*
 *  start the close operation.
 *
 *  This can be called from:
 *
 *  register_close()
 *    - if there are no outstanding operations at time of call
 *
 *  read/write callback
 *    - if in a closing state and the last outstanding operation finishes.
 */
globus_result_t
globus_l_xio_start_close(
    globus_i_xio_operation_t *                  op)
{
    globus_i_xio_handle_t *                     xio_handle;
    globus_result_t                             res;

    xio_handle = op->xio_handle;

    /* the close must not be started until both lists are empty */
    globus_assert(globus_list_empty(xio_handle->read_list) &&
           globus_list_empty(xio_handle->write_list));

    /* register the timeout */
    if(xio_handle->close_timeout_cb != NULL)
    {
        /* op the operatin reference count for this */
        op->ref++;
        op->timeout_cb = xio_handle->close_timeout_cb;
        globus_i_xio_timer_register_timeout(
                g_globus_l_xio_timeout_timer,
                op,
                &op->progress,
                globus_l_xio_timeout_callback,
                &xio_handle->close_timeout_period);
    }

    op->in_register = GLOBUS_TRUE;
    GlobusXIODriverPassClose(res, op, globus_i_xio_open_close_callback, \
        (void *)op);
    op->in_register = GLOBUS_FALSE;
    if(res != GLOBUS_SUCCESS)
    {
        /* LAZY:
         * if failure occurres register a callback with the error and return
         * If this is a performance issue we could return a result_t and
         * let the calling function decide if it needs to register a one
         * shot or just call callback directly or even just return the 
         * result_t.  However this is much easier and only happens at 
         * close and cancel (I have rationalized this to myself, but i 
         * i still feel dirty about it, thus the 8 lines of doc)
         */
        op->cached_res = res;
        res = globus_callback_register_oneshot(
                NULL,
                NULL,
                globus_l_xio_start_close_failure_kickout,
                (void *)op,
                op->space);

        /* what else can we do, at least this way we know about it */
        globus_assert(res == GLOBUS_SUCCESS);
    }

    return GLOBUS_SUCCESS;

}

globus_result_t
globus_l_xio_register_close(
    globus_i_xio_handle_t *                     xio_handle,
    globus_i_xio_attr_t *                       attr,
    globus_xio_callback_t                       cb,
    void *                                      user_arg)
{
    globus_result_t                             res = GLOBUS_SUCCESS;
    globus_i_xio_operation_t *                  op;
    globus_i_xio_operation_t *                  tmp_op;

    GlobusXIOOperationCreate(op, GLOBUS_XIO_OPERATION_TYPE_CLOSE, \
        xio_handle, attr->space);
    if(op == NULL)
    {
        return GlobusXIOErrorMemoryAlloc("globus_xio_register_close");
    }

    /* set up op */
    for(ctr = 0; ctr < xio_handle->stack_size; ctr++)
    {
        GlobusIXIOAttrGetDS(xio_op->entry[ctr].attr,     \
            attr, xio_handle->context->entry[ctr].driver);
    }

    switch(xio_handle->state)
    {
        case GLOBUS_XIO_HANDLE_STATE_OPEN:
            xio_handle->state = GLOBUS_XIO_HANDLE_STATE_CLOSING;
            break;

        case GLOBUS_XIO_HANDLE_STATE_EOF_RECEIVED:
            xio_handle->state = 
                GLOBUS_XIO_HANDLE_STATE_EOF_RECEIVED_AND_CLOSING;
            break
        
        case GLOBUS_XIO_HANDLE_STATE_EOF_DELIVERED:
            xio_handle->state = 
                GLOBUS_XIO_HANDLE_STATE_EOF_DELIVERED_AND_CLOSING;
            break;
    }
    /* this is set for the cancel */
    xio_handle->close_op = op;

    /* 
     *  if the user requests a cancel kill all open ops
     *  if they didn't the close will not happen until all ops finish 
     */
    if(attr->close_cancel)
    {
        /* all canceling is done with cancel op locked */
        globus_mutex_lock(&xio_handle->cancel_mutex);

        for(list = xio_handle->op_list;
            !globus_list_empty(list);
            list = globus_list_rest(list))
        {
            tmp_op = (globus_i_xio_operation_t *) globus_list_first(list);
            globus_l_xio_operation_cancel(tmp_op);
        }

        globus_mutex_unlock(&xio_handle->cancel_mutex);
    }

    /* register timeout */
    if(xio_handle->close_timeout_cb != NULL)
    {
        /* op the operatin reference count for this */
        op->ref++;
        op->timeout_cb = xio_handle->close_timeout_cb;
        res = globus_i_xio_timer_register_timeout(
                g_globus_l_xio_timeout_timer,
                op,
                &op->progress,
                globus_l_xio_timeout_callback,
                &xio_handle->close_timeout_period);
        if(res != GLOBUS_SUCCESS)
        {
            return res;
        }
    }

    xio_handle->close_op = op;
    /* if no outstanding operations just go ahead with cancel */
    if(xio_handle->outstanding_operations == 0)
    {
        res = globus_l_xio_start_close(op);
    }

    return res;
}

/*
 *  cancel the operations
 */
globus_result_t
globus_l_xio_handle_cancel_operations(
    globus_i_xio_handle_t *                     xio_handle,
    globus_i_xio_attr_t *                       attr)
{
    globus_list_t *                             list;
    globus_i_xio_operation_t *                  tmp_op;
    globus_result_t                             res = GLOBUS_SUCCESS;

    globus_mutex_lock(&xio_handle->cancel_mutex);
    {
        if(attr->cancel_open)
        {
            if(xio_handle->open_op == NULL)
            {
                res = GlobusXIOOperationNotFound(
                        "globus_l_xio_handle_cancel_operations");
            }
            else
            {
                globus_l_xio_operation_cancel(xio_handle->open_op);
            }
        }
        if(attr->cancel_close)
        {
            if(xio_handle->close_op == NULL)
            {
                res = GlobusXIOOperationNotFound(
                        "globus_l_xio_handle_cancel_operations");
            }
            else
            {
                globus_l_xio_operation_cancel(xio_handle->close_op);
            }
        }
        if(attr->cancel_read)
        {
            if(!globus_list_empty(xio_handle->read_op_list) &&
                !globus_list_empty(xio_handle->read_eof_list))
            {
                res = GlobusXIOOperationNotFound(
                        "globus_l_xio_handle_cancel_operations");
            }
            else
            {
                /* remove all outstanding read ops */
                for(list = xio_handle->read_op_list;
                    !globus_list_empty(list);
                    list = globus_list_rest(list))
                {
                    tmp_op = (globus_i_xio_operation_t *) 
                                globus_list_first(list);
                    globus_l_xio_operation_cancel(tmp_op);
                }

                /* cancel all already returned eof ops
                    we could probably skip this */
                for(list = xio_handle->read_eof_list;
                    !globus_list_empty(list);
                    list = globus_list_rest(list))
                {
                    tmp_op = (globus_i_xio_operation_t *) 
                                globus_list_first(list);
                    globus_l_xio_operation_cancel(tmp_op);
                }
            }
        }
        if(attr->cancel_write)
        {
            if(!globus_list_empty(xio_handle->write_op_list))
            {
                res = GlobusXIOOperationNotFound(
                        "globus_l_xio_handle_cancel_operations");
            }
            else
            {
                for(list = xio_handle->write_op_list;
                    !globus_list_empty(list);
                    list = globus_list_rest(list))
                {
                    tmp_op = (globus_i_xio_operation_t *)  
                                globus_list_first(list);
                    globus_l_xio_operation_cancel(tmp_op);
                }
            }
        }
    }
    globus_mutex_unlock(&xio_handle->cancel_mutex);

    /* cancel doesn't fail sonce it is a best effort method */
    return res;
}
/********************************************************************
 *                        API functions 
 *                        -------------
 *******************************************************************/
/*
 *  User Open
 *  ---------
 *  Check the parameters and state then pass to internal open function.
 */
globus_result_t
globus_xio_register_open(
    globus_xio_handle_t *                       handle,
    globus_xio_attr_t                           attr,
    globus_xio_target_t                         target,
    globus_xio_callback_t                       cb,
    void *                                      user_arg)
{
    globus_i_xio_handle_t *                     xio_handle = NULL;
    globus_i_xio_target_t *                     xio_target;
    globus_i_xio_context_t *                    xio_context = NULL;
    globus_result_t                             res = GLOBUS_SUCCESS;

    if(handle == NULL)
    {
        res = GlobusXIOErrorBadParameter("globus_xio_register_open");
        goto err;
    }
    if(target == NULL)
    {
        res = GlobusXIOErrorBadParameter("globus_xio_register_open");
        goto err;
    }

    *handle = NULL; /* initialze to be nice touser */
    xio_target = (globus_i_xio_target_t *) target;

    /* this is gaurenteed to be greater than zero */
    globus_assert(xio_target->stack_size > 0);

    /* allocate and initialize context */
    GlobusXIOContextCreate(xio_context, xio_target);
    if(xio_context == NULL)
    {
        res = GlobusXIOErrorMemoryAlloc("globus_xio_register_open");
        goto err;
    }

    /* allocate and intialize the handle structure */
    GlobusXIOHandleCreate(xio_handle, xio_target);
    if(xio_handle == NULL)
    {
        res = GlobusXIOErrorMemoryAlloc("globus_xio_register_open");
        goto err;
    }

    GlobusXIOOperationCreate(xio_op, GLOBUS_XIO_OPERATION_TYPE_OPEN, \
        xio_handle, attr->space);
    if(op == NULL)
    {
        res = GlobusXIOErrorMemoryAlloc("globus_xio_register_open");
        goto err;
    }

    xio_op._op_cb = cb;
    xio_op.user_arg = user_arg;

    res = globus_l_xio_register_open(
            xio_handle,
            xio_context,
            xio_target,
            xio_op,
            attr);
    if(res != GLOBUS_SUCCESS)
    {
        /* if function fails it will deal with clean up of the structures */
        xio_op = NULL;
        xio_handle = NULL;
        xio_context = NULL;
        goto err;
    }

    return GLOBUS_SUCCESS;

  err:
    if(xio_op != NULL)
    {
        GlobusXIOOperationDestroy(xio_op);
    }
    if(xio_handle != NULL)
    {
        GlobusXIOHandleDestroy(xio_handle);
    }
    if(xio_context != NULL)
    {
        GlobusXIOContextDestroy(xio_context);
    }

    return res;
}

/*
 *  User Read
 *  ---------
 *  Check the parameters and state
 *  If everything is ok create and setup the operation structure
 *  fake the iovec structure with the dummy iovec in the operation struct
 *  Then pass to the internal readv function
 */
globus_result_t
globus_xio_register_read(
    globus_xio_handle_t                         handle,
    globus_byte_t *                             buffer,
    globus_size_t                               buffer_length,
    globus_xio_data_descriptor_t                data_desc,
    globus_xio_callback_t                       cb,
    void *                                      user_arg)
{
    globus_xio_operation_t *                    op;
    globus_i_xio_handle_t *                     xio_handle;

    /* error echecking */
    if(handle == NULL)
    {
        return GlobusXIOErrorBadParameter("globus_xio_register_read");
    }
    if(buffer == NULL)
    {
        return GlobusXIOErrorBadParameter("globus_xio_register_read");
    }
    if(buffer_length <= 0)
    {
        return GlobusXIOErrorBadParameter("globus_xio_register_read");
    }

    xio_handle = handle;

    globus_mutex_lock(xio_handle->mutex);
    {
        if(xio_handle->write_state != GLOBUS_XIO_HANDLE_STATE_OPEN &&
           xio_handle->write_state != GLOBUS_XIO_HANDLE_STATE_EOF_RECEIVED)
        {
            res = GlobusXIOErrorHandleNotOpen("globus_xio_register_read");
        }
        else
        {
            GlobusXIOOperationCreate(op, GLOBUS_XIO_OPERATION_TYPE_READ, \
                xio_handle, dd->space);
            if(op == NULL)
            {
                res = GlobusXIOErrorMemoryAlloc("globus_xio_register_read");
            }
            else
            {
                /* set up the operation */
                op->data_cb = cb;
                op->user_arg = user_arg;
                op->mem_iovec.iov_base = buffer;
                op->mem_iovec.iov_len = buffer_length;
                op->iovec_count = 1;
                op->iovec = &op->mem_iovec;

                res = globus_l_xio_register_readv(
                        xio_handle,
                        op);
            }
        }
    }
    globus_mutex_unlock(xio_handle->mutex);

    return res;
}

/*
 *  User Readv
 *  ----------
 *  Check the parameters and state
 *  If everything is ok create and setup the operation structure
 *  Then pass to the internal readv function
 */
globus_result_t
globus_xio_register_readv(
    globus_xio_handle_t                         handle,
    globus_iovec_t *                            iovec,
    int                                         iovec_count,
    globus_xio_data_descriptor_t                data_desc,
    globus_xio_callback_t                       cb,
    void *                                      user_arg)
{
    globus_xio_operation_t *                    op;
    globus_i_xio_handle_t *                     xio_handle;

    /* error echecking */
    if(handle == NULL)
    {
        return GlobusXIOErrorBadParameter("globus_xio_register_readv");
    }
    if(buffer == NULL)
    {
        return GlobusXIOErrorBadParameter("globus_xio_register_readv");
    }
    if(buffer_length <= 0)
    {
        return GlobusXIOErrorBadParameter("globus_xio_register_readv");
    }

    xio_handle = handle;

    globus_mutex_lock(xio_handle->mutex);
    {
        if(xio_handle->write_state != GLOBUS_XIO_HANDLE_STATE_OPEN &&
           xio_handle->write_state != GLOBUS_XIO_HANDLE_STATE_EOF_RECEIVED)
        {
            res = GlobusXIOErrorHandleNotOpen("globus_xio_register_readv");
        }
        else
        {
            GlobusXIOOperationCreate(op, GLOBUS_XIO_OPERATION_TYPE_READ, \
                xio_handle, dd->space);
            if(op == NULL)
            {
                res = GlobusXIOErrorMemoryAlloc("globus_xio_register_readv");
            }
            else
            {
                /* set up the operation */
                op->iovec_cb = cb;
                op->user_arg = user_arg;
                op->iovec = iovec;
                op->iovec_count = iovec_count;

                res = globus_l_xio_register_readv(
                        xio_handle,
                        op);
            }
        }
    }
    globus_mutex_unlock(xio_handle->mutex);

    return res;
}   

/*
 *  User Write
 *  ----------
 *  Check the parameters and state
 *  If everything is ok create and setup the operation structure
 *  fake the iocev structure with the dummy iovec in the operation struct
 *  Then pass to the internal writev function
 */
globus_result_t
globus_xio_register_write(
    globus_xio_handle_t                         handle,
    globus_byte_t *                             buffer,
    globus_size_t                               buffer_length,
    globus_xio_data_descriptor_t                data_desc,
    globus_xio_data_callback_t                  cb,
    void *                                      user_arg)
{
    globus_xio_operation_t *                    op;

    /* error echecking */
    if(handle == NULL)
    {
        return GlobusXIOErrorBadParameter("globus_xio_register_write");
    }
    if(buffer == NULL)
    {
        return GlobusXIOErrorBadParameter("globus_xio_register_write");
    }
    if(buffer_length <= 0)
    {
        return GlobusXIOErrorBadParameter("globus_xio_register_write");
    }

    xio_handle = handle;

    globus_mutex_lock(xio_handle->mutex);
    {
        if(xio_handle->write_state != GLOBUS_XIO_HANDLE_STATE_OPEN &&
           xio_handle->write_state != GLOBUS_XIO_HANDLE_STATE_EOF_RECEIVED &&
           xio_handle->write_state != GLOBUS_XIO_HANDLE_STATE_DELIVERED)
        {
            res = GlobusXIOErrorHandleNotOpen("globus_xio_register_write");
        }
        else
        {
            GlobusXIOOperationCreate(op, GLOBUS_XIO_OPERATION_TYPE_WRITE, \
                xio_handle, dd->space);
            if(op == NULL)
            {
                res = GlobusXIOErrorMemoryAlloc("globus_xio_register_write");
            }
            else
            {
                /* set up the operation */
                op->data_cb = cb;
                op->user_arg = user_arg;
                op->mem_iovec.iov_base = buffer;
                op->mem_iovec.iov_len = buffer_length;
                op->iovec_count = 1;
                op->iovec = &op->mem_iovec;

                res = globus_l_xio_register_writev(
                        xio_handle,
                        op);
            }
        }
    }
    globus_mutex_unlock(xio_handle->mutex);

    return res;
}

/*
 *  User Writev
 *  -----------
 *  Check the parameters and state
 *  If everything is ok create and setup the operation structure
 *  Then pass to the internal writev function
 */
globus_result_t
globus_xio_register_writev(
    globus_xio_handle_t                         handle,
    globus_iovec_t *                            iovec,
    int                                         iovec_count,
    globus_xio_data_descriptor_t                data_desc,
    globus_xio_iovec_data_callback_t            cb,
    void *                                      user_arg)
{
    globus_i_xio_handle_t *                     xio_handle;
    globus_result_t                             res;
    globus_xio_operation_t *                    op;

    /* error echecking */
    if(handle == NULL)
    {
        return GlobusXIOErrorBadParameter("globus_xio_register_writev");
    }
    if(iovec == NULL)
    {
        return GlobusXIOErrorBadParameter("globus_xio_register_writev");
    }
    if(iovec_count <= 0)
    {
        return GlobusXIOErrorBadParameter("globus_xio_register_writev");
    }

    xio_handle = handle;

    globus_mutex_lock(xio_handle->mutex);
    {
        if(xio_handle->write_state != GLOBUS_XIO_HANDLE_STATE_OPEN &&
           xio_handle->write_state != GLOBUS_XIO_HANDLE_STATE_EOF_RECEIVED &&
           xio_handle->write_state != GLOBUS_XIO_HANDLE_STATE_DELIVERED)
        {
            res = GlobusXIOErrorHandleNotOpen("globus_xio_register_writev");
        }
        else
        {
            GlobusXIOOperationCreate(op, GLOBUS_XIO_OPERATION_TYPE_WRITE, \
                xio_handle, dd->space);
            if(op == NULL)
            {
                res = GlobusXIOErrorMemoryAlloc("globus_xio_register_writev");
            }
            else
            {
                /* set up the operation */
                op->iovec_cb = cb;
                op->user_arg = user_arg;
                op->iovec = iovec;
                op->iovec_count = iovec_count;

                /* if this fails the sub function will clean the op */
                res = globus_l_xio_register_writev(
                        xio_handle,
                        op);
            }
        }
    }
    globus_mutex_lock(xio_handle->mutex);

    return res;
}


/*
 *  User Close
 *  ----------
 *  Check the parameters and state then pass to internal function.
 */
globus_result_t
globus_xio_register_close(
    globus_xio_handle_t                         handle,
    globus_xio_attr_t                           attr,
    globus_xio_callback_t                       cb,
    void *                                      user_arg)
{
    globus_i_xio_handle_t *                     xio_handle;
    globus_result_t                             res;

    /* error echecking */
    if(handle == NULL)
    {
        return GlobusXIOErrorBadParameter("globus_xio_register_close");
    }

    xio_handle = handle;

    globus_mutex_lock(&xio_handle->mutex);
    {
        if(xio_handle->state != GLOBUS_XIO_HANDLE_STATE_OPEN &&
           xio_handle->state != GLOBUS_XIO_HANDLE_STATE_EOF_RECEIVED &&
           xio_handle->state != GLOBUS_XIO_HANDLE_STATE_EOF_DELIVERED)
        {
            res = GlobusXIOErrorHandleNotOpen("globus_xio_register_close");
        }
        else
        {
            res = globus_l_xio_register_close(
                    xio_handle,
                    attr,
                    cb,
                    user_arg);
        }
    }
    globus_mutex_unlock(&xio_handle->mutex);

    return res;
}

/*
 *  cancel outstanding operations.
 * 
 *  In the furture the attr will control what operations get canceled.
 *  For now all are canceled.
 */
globus_result_t
globus_xio_handle_cancel_operations(
    globus_xio_handle_t                         handle,
    globus_xio_attr_t                           attr)
{
    globus_i_xio_handle_t *                     xio_handle;
    globus_result_t                             res;

    /* error echecking */
    if(handle == NULL)
    {
        return GlobusXIOErrorBadParameter(                      \
                    "globus_xio_handle_cancel_operations");
    }

    xio_handle = handle;

    globus_mutex_lock(&xio_handle->mutex);
    {
        /* if closed there is nothing to cancel */
        if(xio_handle->state == GLOBUS_XIO_HANDLE_STATE_CLOSED)
        {
            res = GlobusXIOErrorBadState("globus_xio_handle_cancel_operations");
        }
        else
        {
            res = globus_l_xio_handle_cancel_operations(
                    xio_handle,
                    attr);
        }
    }
    globus_mutex_unlock(&xio_handle->mutex);

    return res;
}
