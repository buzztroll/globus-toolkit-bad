
#include "globus_common.h"
#include "globus_error_string.h"
#include "globus_test_tap.h"

#define myline 42
int main()
{
    globus_object_t * err;
    char * s;
    char * t;
    static char * myname = "main";

    printf("1..1\n");

    globus_module_activate(GLOBUS_COMMON_MODULE);

#line myline
    err = globus_error_construct_string(GLOBUS_COMMON_MODULE, GLOBUS_ERROR_NO_INFO, "[%s]: Error doing something hard at %s:%d\n", GLOBUS_COMMON_MODULE->module_name, myname, __LINE__);
    s = globus_object_printable_to_string(err);
#line myline
    t = globus_common_create_string( "[%s]: Error doing something hard at %s:%d\n", GLOBUS_COMMON_MODULE->module_name, myname, __LINE__);
    ok(strcmp(s, t) == 0, "globus_common_error_string");
    free(s);
    free(t);

    return TEST_EXIT_CODE;
}
