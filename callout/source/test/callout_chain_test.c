
#include "globus_callout.h"
#include "globus_common.h"
#include "globus_preload.h"

#include <stdlib.h>
#include <stdio.h>

int main()
{
    globus_callout_handle_t     callout_handle;
    globus_result_t             result;

    LTDL_SET_PRELOADED_SYMBOLS();
    printf("1..4\n");

    globus_module_activate(GLOBUS_COMMON_MODULE);
    globus_module_activate(GLOBUS_CALLOUT_MODULE);
    
    result = globus_callout_handle_init(&callout_handle);

    if(result != GLOBUS_SUCCESS)
    {
        goto error_exit;
    }

    result = globus_callout_register(callout_handle, 
                                     "TEST_CHAIN",
                                     "libchaina_test",
                                     "chaina_test_callout");
    if(result != GLOBUS_SUCCESS)
    {
        goto error_exit;
    }

    result = globus_callout_register(callout_handle, 
                                     "TEST_CHAIN",
                                     "libchainb_test",
                                     "chainb_test_callout");
    if(result != GLOBUS_SUCCESS)
    {
        goto error_exit;
    }

    result = globus_callout_register(callout_handle, 
                                     "TEST_CHAIN",
                                     "libchainc_test",
                                     "chainc_test_callout");
    if(result != GLOBUS_SUCCESS)
    {
        goto error_exit;
    }
    
    result = globus_callout_call_type(callout_handle,
                                      "TEST_CHAIN",
                                      "foo",
                                      "bar");

    if(result != GLOBUS_SUCCESS)
    {
        goto error_exit;
    }
    
    result = globus_callout_handle_destroy(callout_handle);


    if(result != GLOBUS_SUCCESS)
    {
        goto error_exit;
    }

    globus_module_deactivate_all();
    printf("ok 4 - callout_chain_test complete\n");
    
    return 0;

 error_exit:

    fprintf(stderr,"# ERROR: %s",
            globus_error_print_chain(globus_error_get(result)));
    
    globus_module_deactivate_all();
    printf("not ok 4 - callout_chain_test complete\n");

    return 1;
}
