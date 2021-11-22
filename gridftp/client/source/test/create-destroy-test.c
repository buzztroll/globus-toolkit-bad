
/*
 * Verify that handle destruction works even if no operation was done
 * on the handle.
 */
#include "globus_ftp_client.h"
#include "globus_preload.h"

int main()
{
    globus_ftp_client_handle_t			handle;
    LTDL_SET_PRELOADED_SYMBOLS();
    globus_module_activate(GLOBUS_FTP_CLIENT_MODULE);
    globus_ftp_client_handle_init(&handle, GLOBUS_NULL);
    globus_ftp_client_handle_destroy(&handle);
    globus_module_deactivate_all();

    return 0;
}
