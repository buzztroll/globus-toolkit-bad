
#ifndef _GLOBUS_USAGE_H_
#define _GLOBUS_USAGE_H_

#include "globus_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GLOBUS_USAGE_MODULE             &globus_i_usage_stats_module
extern globus_module_descriptor_t       globus_i_usage_stats_module;

typedef struct globus_usage_stats_handle_s * globus_usage_stats_handle_t;

enum
{
    GLOBUS_USAGE_STATS_ERROR_TYPE_OOM,
    GLOBUS_USAGE_STATS_ERROR_TYPE_TOO_BIG,
    GLOBUS_USAGE_STATS_ERROR_TYPE_UNKNOWN_HOSTNAME
};

globus_result_t
globus_usage_stats_handle_init(
    globus_usage_stats_handle_t *       handle,
    uint16_t                            code,
    uint16_t                            version,
    const char *                        targets);

void
globus_usage_stats_handle_destroy(
    globus_usage_stats_handle_t         handle);

globus_result_t
globus_usage_stats_send(
    globus_usage_stats_handle_t         handle,
    int                                 count,
    ...);

globus_result_t
globus_usage_stats_vsend(
    globus_usage_stats_handle_t         handle,
    int                                 count,
    va_list                             ap);

globus_result_t
globus_usage_stats_send_array(
    globus_usage_stats_handle_t         handle,
    int                                 count,
    char **                             key_array,
    char **                             value_array);

#ifdef __cplusplus
}
#endif

#endif
