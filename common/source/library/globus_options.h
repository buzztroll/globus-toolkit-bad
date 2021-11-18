
/**
 * @file globus_options.h
 * @brief Command-Line Options Parser
 */

#ifndef GLOBUS_OPTIONS_H
#define GLOBUS_OPTIONS_H 1

#include "globus_common_include.h"


#ifdef __cplusplus
extern "C" {
#endif

enum 
{
    GLOBUS_OPTIONS_HELP = 1,
    GLOBUS_OPTIONS_NOT_ENOUGH_ARGS,
    GLOBUS_OPTIONS_INVALID_PARAMETER,
    GLOBUS_OPTIONS_UNKNOWN
};

typedef struct globus_l_options_handle_s * globus_options_handle_t;

typedef
globus_result_t
(*globus_options_callback_t)(
    globus_options_handle_t             opts_handle,
    char *                              cmd,
    char **                             parm,
    void *                              arg,
    int *                               out_parms_used);

typedef
globus_result_t
(*globus_options_unknown_callback_t)(
    globus_options_handle_t             opts_handle,
    void *                              unknown_arg,
    int                                 argc,
    char **                             argv);

typedef struct globus_options_entry_s
{
    char *                              opt_name; /* long and filename */
    char *                              short_opt;
    char *                              env;
    char *                              parms_desc;
    char *                              description;
    int                                 arg_count;
    globus_options_callback_t           func;
} globus_options_entry_t;

#define  GLOBUS_OPTIONS_END {NULL, NULL, NULL, NULL, NULL, 0, NULL}

globus_result_t
globus_options_init(
    globus_options_handle_t *           out_handle,
    globus_options_unknown_callback_t   unknown_func,
    void *                              unknown_arg);

globus_result_t
globus_options_add_table(
    globus_options_handle_t             handle,
    globus_options_entry_t *            table,
    void *                              user_arg);

globus_result_t
globus_options_destroy(
    globus_options_handle_t             handle);

globus_result_t
globus_options_command_line_process(
    globus_options_handle_t             handle,
    int                                 argc,
    char **                             argv);

globus_result_t
globus_options_env_process(
    globus_options_handle_t             handle);

globus_result_t
globus_options_file_process(
    globus_options_handle_t             handle,
    char *                              filename);

globus_result_t
globus_options_xinetd_file_process(
    globus_options_handle_t             handle,
    char *                              filename,
    char *                              service_name);

void
globus_options_help(
    globus_options_handle_t             handle);

#ifdef __cplusplus
}
#endif

#endif /* GLOBUS_OPTIONS_H */
