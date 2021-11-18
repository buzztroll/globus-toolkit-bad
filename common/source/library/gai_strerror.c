
#include "globus_common.h"

#ifdef GLOBUS_IMPLEMENT_GAI_STRERROR
const char *gai_strerror(
    int                                 rc)
{
    return "name resolution error";
}
#endif
