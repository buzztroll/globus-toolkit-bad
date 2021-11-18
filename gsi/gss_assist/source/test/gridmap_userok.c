
#include "gssapi.h"
#include "globus_gss_assist.h"

#define SIZEOF_ARRAY(a) (sizeof(a) / sizeof(a[0]))

int
main(int argc, char *argv[])
{
    char *dn, *local_username;
    int rc;
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s dn local-username\n", argv[0]);
        exit(1);
    }
    dn = argv[1];
    local_username = argv[2];

    rc = globus_module_activate(GLOBUS_GSI_GSS_ASSIST_MODULE);
    if (rc != 0)
    {
        fprintf(stderr, "Error activating GLOBUS_GSI_GSS_ASSIST_MODULE\n");
        exit(1);
    }

    rc = globus_gss_assist_userok(dn, local_username);
    if (rc != 0)
    {
        fprintf(stderr, "globus_gss_assist_userok failed [%s for %s]\n", local_username, dn);
        exit(1);
    }

    exit(0);
}
/* main() */
