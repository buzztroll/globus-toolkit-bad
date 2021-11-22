
#include "globus_xio.h"
#include "globus_xio_tcp_driver.h"

#define LINE_LEN 1024

void
test_res(
    globus_result_t                         res)
{
    if(res == GLOBUS_SUCCESS)
    {
        return;
    }

    fprintf(stderr, "ERROR: %s\n", globus_object_printable_to_string(
        globus_error_get(res)));

    globus_assert(0);
}

void
help()
{
    fprintf(stdout, "globus_xio_client [options] <contact string>\n");
    fprintf(stdout, "-----------------\n");
    fprintf(stdout, "options:\n");
    fprintf(stdout, "-D <drivers> : add this driver to the stack\n");
}

int
main(
    int                                     argc,
    char **                                 argv)
{
    globus_xio_driver_t                     driver;
    globus_xio_stack_t                      stack;
    globus_xio_handle_t                     xio_handle;
    char *                                  cs;
    globus_result_t                         res;
    char                                    line[LINE_LEN];
    int                                     ctr;
    globus_bool_t                           done = GLOBUS_FALSE;
    globus_size_t                           nbytes;

    if(argc < 2)
    {
        help();
        return 1;
    }

    globus_module_activate(GLOBUS_XIO_MODULE);
    globus_xio_stack_init(&stack, NULL);
    for(ctr = 1; ctr < argc - 1; ctr++)
    {
        if(strcmp(argv[ctr], "-h") == 0)
        {
            help();
            return 0;
        }
        else if(strcmp(argv[ctr], "-D") == 0 && ctr + 1 < argc - 1)
        {
            ctr++;
            globus_xio_driver_load(argv[ctr], &driver);
            globus_xio_stack_push_driver(stack, driver);
        }
    }

    cs = argv[argc - 1];
    res = globus_xio_handle_create(&xio_handle, stack);
    test_res(res);

    globus_xio_open(xio_handle, cs, NULL);

    while(!done)
    {
        if(fgets(line, LINE_LEN, stdin) == NULL)
        {
            done = GLOBUS_TRUE;
        }
        else
        {
            int                         len;
            
            len = strlen(line);
            res = globus_xio_write(
                xio_handle, line, len, len, &nbytes, NULL);
            test_res(res);
        }
    }

    globus_xio_close(xio_handle, NULL);

    globus_module_activate(GLOBUS_XIO_MODULE);

    return 0;
}
