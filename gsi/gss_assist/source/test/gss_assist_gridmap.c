
#include "globus_gss_assist.h"

#ifdef WIN32
#include <getopt.h>
#endif

int main(int argc, char * argv[])
{
    char *                              local_user = NULL;
    char *                              mapped_user = NULL;
    char *                              dn = NULL;
    int                                 rc;
    int                                 ch;

    while ((ch = getopt(argc, argv, "g:l:d:")) != -1)
    {
        switch (ch)
        {
        case 'g':
            globus_libc_setenv("GRIDMAP", optarg, 1);
            break;
        case 'l':
            local_user = optarg;
            break;
        case 'd':
            dn = optarg;
            break;
        default:
            fprintf(stderr, "Usage: %s -g GRIDMAP -l LOCAL-ID -d DN\n",
                    strrchr(argv[0], '/') ? strrchr(argv[0], '/')+1 : argv[0]);
            exit(99);
        }
    }

    if (dn == NULL || local_user == NULL)
    {
        fprintf(stderr, "Usage: %s -g GRIDMAP -l LOCAL-ID -d DN\n",
                strrchr(argv[0], '/') ? strrchr(argv[0], '/')+1 : argv[0]);
        exit(99);
    }

    printf("1..3\n");
    
    globus_module_activate(GLOBUS_GSI_GSS_ASSIST_MODULE);
    
    rc = globus_gss_assist_gridmap(dn, &mapped_user);
    printf("%s - globus_gss_assist_gridmap\n",  (rc==0) ? "ok" : "not ok");

    printf("%s - default_user_match\n",
        (strcmp(mapped_user?mapped_user:"", local_user) == 0) ? "ok" : "not ok");

    rc = globus_gss_assist_userok(
            dn, local_user);
    printf("%s - userok\n", (rc == 0) ? "ok" : "not ok");
    globus_module_deactivate_all();

    return rc;
}
