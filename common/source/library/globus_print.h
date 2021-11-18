
/**
 * @file globus_print.h
 * @brief Error Message Printing
 */

#if !defined(GLOBUS_PRINT_H)
#define GLOBUS_PRINT_H 1

#include "globus_common_include.h"
#include "globus_module.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 */
extern void globus_fatal(char *msg, ...);
/**
 */
extern void globus_silent_fatal(void);
/**
 */
extern void globus_error(char *msg, ...);
/**
 */
extern void globus_warning(char *msg, ...);
/**
 */
extern void globus_notice(char *msg, ...);
/**
 */
extern void globus_perror(char *msg, ...);
/**
 */
extern void globus_fatal_perror(char *msg, ...);
/**
 */
extern char *globus_assert_sprintf(char *msg, ...);
/**
 */
extern char *globus_get_unique_session_string(void);

void
globus_panic(
    globus_module_descriptor_t *        module,
    globus_result_t                     result,
    const char *                        message,
    ...);
    

#ifdef __cplusplus
}
#endif

#endif /* GLOBUS_PRINT_H */
