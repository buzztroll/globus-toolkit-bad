
/*
 * simple get test. Verify that using url caching does not cause a
 * simple get operation to fail. Transfer the file twice, with url caching
 * enabled.
 */
#include "globus_ftp_client.h"
#include "globus_ftp_client_test_common.h"

static globus_mutex_t lock;
static globus_cond_t cond;
static globus_bool_t done;
static globus_bool_t error = GLOBUS_FALSE;

static
void
done_cb(
	void *					user_arg,
	globus_ftp_client_handle_t *		handle,
	globus_object_t *			err)
{
    char * tmpstr;

    if(err) tmpstr = " an";
    else    tmpstr = "out";

    if(err) { printf("done with%s error\n", tmpstr); 
              error++; }
    globus_mutex_lock(&lock);
    done = GLOBUS_TRUE;
    globus_cond_signal(&cond);
    globus_mutex_unlock(&lock);
       
}

int main(int argc, char **argv)
{
    globus_ftp_client_handle_t			handle;
    globus_ftp_client_operationattr_t		attr;
    globus_byte_t				buffer[1024];
    globus_size_t				buffer_length = sizeof(buffer);
    globus_result_t				result;
    int						i;
    globus_ftp_client_handleattr_t		handle_attr;
    char *					src;
    char *					dst;

    LTDL_SET_PRELOADED_SYMBOLS();
    globus_module_activate(GLOBUS_FTP_CLIENT_MODULE);
    globus_ftp_client_handleattr_init(&handle_attr);
    globus_ftp_client_operationattr_init(&attr);

    test_parse_args(argc,
		    argv,
		    &handle_attr,
		    &attr,
		    &src,
		    &dst);
    globus_mutex_init(&lock, GLOBUS_NULL);
    globus_cond_init(&cond, GLOBUS_NULL);

    globus_ftp_client_handleattr_set_cache_all(&handle_attr,
					       GLOBUS_TRUE);

    globus_ftp_client_handle_init(&handle,  &handle_attr);
    for (i = 0; i < 3; i++)
    {
        done = GLOBUS_FALSE;

        result = globus_ftp_client_third_party_transfer(
                    &handle,
                    src,
                    &attr,
                    dst,
                    &attr,
                    GLOBUS_NULL,
				   done_cb,
				   0);
        if(result != GLOBUS_SUCCESS)
        {
            done = GLOBUS_TRUE;
        }
        globus_mutex_lock(&lock);
        while(!done)
        {
            globus_cond_wait(&cond, &lock);
        }
        globus_mutex_unlock(&lock);

        fprintf(stderr, "####################\n");
    }

    globus_ftp_client_handle_destroy(&handle);
    globus_module_deactivate_all();

    if(test_abort_count && (error==2))
    {
	return 0;
    }
    return error;
}
