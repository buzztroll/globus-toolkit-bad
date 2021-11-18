#include <stdio.h>
#include <stdbool.h>
#include "gssapi.h"

int
main(int argc, char *argv[])
{
    OM_uint32                           unwrap_major_status;
    OM_uint32                           unwrap_minor_status;
    gss_buffer_desc                     output_buffer = {.length=0};
    int                                 failed = 0;

    printf("1..1\n");

    unwrap_major_status = gss_unwrap(
        &unwrap_minor_status,
        GSS_C_NO_CONTEXT,
        &(gss_buffer_desc) {.length = 0},
        &output_buffer,
        NULL,
        NULL);

    if (unwrap_major_status == GSS_S_COMPLETE)
    {
        printf("not ok 1 - GSS_C_NO_CONTEXT\n");
        failed++;
    }
    else
    {
        printf("ok 1 - GSS_C_NO_CONTEXT\n");
    }

    return failed;
}
