
/**
 * @file tokens_bsd.h
 * @author Sam Lang, Sam Meder
 */

#include "globus_common.h"
#include "globus_i_gss_assist.h"

int
token_bsd_get(
    void *                              arg, 
    void **                             bufp, 
    size_t *                            sizep);

int
token_bsd_send(
    void *                              arg,  
    void *                              buf, 
    size_t                              size);


int
token_bsd_send_ex(
    void *                              exp,  
    void *                              buf, 
    size_t                              size);
