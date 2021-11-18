
#include "globus_xio.h"

void
test_res(
    globus_result_t                         res)
{
    if(res == GLOBUS_SUCCESS)
    {
        return;
    }

    fprintf(stderr, "ERROR: %s\n", globus_error_print_friendly(
        globus_error_peek(res)));

    globus_assert(0);
}

void
help()
{
    fprintf(stdout, "globus-xio-simple-client [options] <contact string> <message>\n");
    fprintf(stdout, "-------------------------------------------------------------\n");
    fprintf(stdout, "options:\n");
    fprintf(stdout, "-D <drivers> : add this driver to the stack\n");
    exit(0);
}


int
main(
    int                             argc,
    char *                          argv[])
{
    globus_result_t                 res;
    globus_xio_driver_t             driver;
    globus_xio_stack_t              stack;
    globus_xio_handle_t             handle;
    globus_size_t                   nbytes;
    char *                          contact_string = NULL;
    char *                          message;
    int                             ctr = 1;
    int                             len;

    globus_module_activate(GLOBUS_XIO_MODULE);
    
    res = globus_xio_stack_init(&stack, NULL);
    test_res(res);
    globus_xio_stack_init(&stack, NULL);
    for(ctr = 1; ctr < argc-2; ctr++)
    {
        if(strcmp(argv[ctr], "-h") == 0)
        {
            help();
        }
        else if(strcmp(argv[ctr], "-D") == 0 && ctr + 1 < argc)
        {
            ctr++;
            globus_xio_driver_load(argv[ctr], &driver);
            globus_xio_stack_push_driver(stack, driver);
        }
    }
    contact_string = argv[ctr];
    ctr++;
    message = argv[ctr];
    len = strlen(message);

    res = globus_xio_handle_create(&handle, stack);
    test_res(res);

    res = globus_xio_open(handle, contact_string, NULL);
    test_res(res);

    res = globus_xio_write(
        handle, (globus_byte_t *)message, len, len, &nbytes, NULL);
    test_res(res);

    printf("Wrote %d bytes\n", nbytes);
    
    res = globus_xio_close(handle, NULL);
    test_res(res);

    globus_module_deactivate(GLOBUS_XIO_MODULE);

    return 0;
}
