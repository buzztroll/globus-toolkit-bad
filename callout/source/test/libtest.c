
#include <stdlib.h>
#include <stdio.h>
#include "globus_common.h"

#ifndef WIN32
globus_result_t
test_callout(va_list ap)
#else
globus_result_t
__declspec(dllexport) test_callout(va_list ap)
#endif
{
    va_list cpy;
    const char *arg1, *arg2;
    
    va_copy(cpy, ap);
    arg1 = va_arg(cpy, char *);
    arg2 = va_arg(cpy, char *);
    va_end(cpy);

    if (strcmp(arg1, "foo") != 0 || strcmp(arg2, "bar") != 0)
    {
        printf("not ok 1 - test_callout\n");
        fprintf(stderr, "# arg1 = %s\n# arg2 = %s\n", arg1, arg2);
        return GLOBUS_FAILURE;
    }
    printf("ok 1 - test_callout\n");
    return GLOBUS_SUCCESS;
}
