#ifndef GLOBUS_PRELOAD_H
#define GLOBUS_PRELOAD_H 1

#include "ltdl.h"

#ifndef lt_preloaded_symbols    
#if defined(_WIN32) || defined(__CYGWIN__) || defined(_WIN32_WCE)
/* DATA imports from DLLs on WIN32 con't be const, because runtime
   relocations are performed -- see ld's documentation on pseudo-relocs.  */
# define LT_DLSYM_CONST
#elif defined(__osf__)
   /* This system does not cope well with relocations in const data.  */
# define LT_DLSYM_CONST
#else
# define LT_DLSYM_CONST const
#endif
#define lt_preloaded_symbols    lt__PROGRAM__LTX_preloaded_symbols
/* Ensure C linkage.  */
extern LT_DLSYM_CONST lt_dlsymlist lt__PROGRAM__LTX_preloaded_symbols[];

#ifdef LTDL_SET_PRELOADED_SYMBOLS
#undef LTDL_SET_PRELOADED_SYMBOLS
#endif

#define LTDL_SET_PRELOADED_SYMBOLS() \
        lt_dlpreload_default(lt_preloaded_symbols)
#endif

#endif /* GLOBUS_PRELOAD_H */
