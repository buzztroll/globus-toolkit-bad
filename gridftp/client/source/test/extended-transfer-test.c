
/*
 * extended block mode transfer.
 *
 * makes sure that the ftp client and control libraries will handle the
 * 3rd-party transfer of a file in extended block mode.
 */

#include "globus_ftp_client.h"
#include "globus_ftp_client_test_common.h"

static globus_mutex_t lock;
static globus_cond_t cond;
static globus_bool_t done;
static globus_bool_t error = GLOBUS_FALSE;
#define MYSIZE 40

static
void
done_cb(
	void *					user_arg,
	globus_ftp_client_handle_t *		handle,
	globus_object_t *			err)
{
    char * tmpstr;

    if(err) { tmpstr = globus_object_printable_to_string(err);
	      printf("done with error: %s\n", tmpstr); 
              error = GLOBUS_TRUE;
	      globus_libc_free(tmpstr); }
    globus_mutex_lock(&lock);
    done = GLOBUS_TRUE;
    globus_cond_signal(&cond);
    globus_mutex_unlock(&lock);
       
}

int main(int argc, char **argv)
{
    globus_ftp_client_handle_t			handle;
    globus_ftp_client_operationattr_t		attr;
    globus_result_t				result;
    globus_ftp_client_handleattr_t		handle_attr;
    char *					src;
    char *					dst;
    int						i;
    globus_size_t				parallelism_level = 1;
    globus_ftp_control_parallelism_t		parallelism;

    LTDL_SET_PRELOADED_SYMBOLS();
    globus_module_activate(GLOBUS_FTP_CLIENT_MODULE);
    globus_ftp_client_handleattr_init(&handle_attr);
    globus_ftp_client_operationattr_init(&attr);

    /* Parse local arguments */
    for(i = 1; i < argc; i++)
    {
	if(strcmp(argv[i], "-P") == 0 && i + 1 < argc)
	{
	    parallelism_level = atoi(argv[i+1]);

	    test_remove_arg(&argc, argv, &i, 1);
	}
    }
    test_parse_args(argc,
		    argv,
		    &handle_attr,
		    &attr,
		    &src,
		    &dst);
    if(parallelism_level < 1) 
    {
	parallelism_level = 1;
    }
    parallelism.mode = GLOBUS_FTP_CONTROL_PARALLELISM_FIXED;
    parallelism.fixed.size = parallelism_level;

    globus_mutex_init(&lock, GLOBUS_NULL);
    globus_cond_init(&cond, GLOBUS_NULL);

    globus_ftp_client_operationattr_set_mode(
        &attr,
        GLOBUS_FTP_CONTROL_MODE_EXTENDED_BLOCK);
    globus_ftp_client_operationattr_set_parallelism(&attr,
					            &parallelism);

    globus_ftp_client_handle_init(&handle,  &handle_attr);

    done = GLOBUS_FALSE;
    result = globus_ftp_client_third_party_transfer(&handle,
						    src,
						    &attr,
						    dst,
						    &attr,
						    GLOBUS_NULL,
						    done_cb,
						    0);
    if(result != GLOBUS_SUCCESS)
    {
	globus_object_t * err;
	err = globus_error_get(result);
	fprintf(stderr, "%s", globus_object_printable_to_string(err));
	done = GLOBUS_TRUE;
    }

    globus_mutex_lock(&lock);
    while(!done)
    {
	globus_cond_wait(&cond, &lock);
    }
    globus_mutex_unlock(&lock);

    globus_ftp_client_handle_destroy(&handle);
    globus_module_deactivate_all();
    
    if(test_abort_count && error)
    {
	return 0;
    }
    return error;
}
