
/**
 * @file context/end_listen.c
 * @brief globus_net_manager_context_end_listen()
 */

#include "globus_net_manager_context.h"

/**
 * @brief Call Context End Listen
 * @ingroup globus_net_manager_context
 * @details
 * Call the chain of end_listen() functions in a context with the given
 * task_id, transport, and attributes. If any manager in the context returns
 * an error, the subsequent managers will not be called. 
 *
 * @param [in] context
 *     The context to call functions in.
 * @param [in] task_id
 *     The task_id associated with this network operation.
 * @param [in] transport
 *     The transport driver associated with this network operation.
 * @param [in] local_contact
 *     The string representation of the local address of the
 *     socket.
 * @param [in] attr_array
 *     The set of attributes to pass to the managers.
 *
 * @return
 *     On success, this function returns GLOBUS_SUCCESS. If any manager
 *     returns an error, the end_listen calls stop and the error is returned
 *     to the caller.
 */
globus_result_t
globus_net_manager_context_end_listen(
    globus_net_manager_context_t        context,
    const char                         *task_id,
    const char                         *transport,
    const char                         *local_contact,
    const globus_net_manager_attr_t    *attr_array)
{
    globus_i_net_manager_context_t *    ctx = context;
    globus_list_t *                     list;
    globus_result_t                     result = GLOBUS_SUCCESS;
    globus_i_net_manager_context_entry_t * ent;
    
    if(!ctx || !task_id || !transport || !attr_array || !local_contact)
    {
        result = GlobusNetManagerErrorParameter("No parameter may be NULL.");
        goto error_bad_args;
    }
    
    for(list = ctx->managers; 
        !globus_list_empty(list) && result == GLOBUS_SUCCESS; 
        list = globus_list_rest(list))
    {            
        ent = globus_list_first(list);
        
        if(ent->manager->end_listen)
        {
            result = ent->manager->end_listen(
                ent->manager,
                ent->attrs,
                task_id,
                transport,
                local_contact,
                attr_array);
            if(result != GLOBUS_SUCCESS)
            {
                result = GlobusNetManagerErrorManager(
                    result, ent->manager->name, "end_listen");
            }
        }
    }

    return result;

error_bad_args:
    return result;
}
