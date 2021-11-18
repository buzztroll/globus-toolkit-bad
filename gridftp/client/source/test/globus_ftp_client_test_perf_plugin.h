
/**
 * perf plugin
 *
 * Allow the performance marker data be retrieved to a callback.
 */

#ifndef GLOBUS_INCLUDE_FTP_CLIENT_TEST_PERF_PLUGIN_H
#define GLOBUS_INCLUDE_FTP_CLIENT_TEST_PERF_PLUGIN_H

#include "globus_ftp_client.h"
#include "globus_ftp_client_plugin.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Module descriptor
 */
#define GLOBUS_FTP_CLIENT_TEST_PERF_PLUGIN_MODULE (&globus_i_ftp_client_test_perf_plugin_module)

extern
globus_module_descriptor_t globus_i_ftp_client_test_perf_plugin_module;

globus_result_t
globus_ftp_client_test_perf_plugin_init(
    globus_ftp_client_plugin_t *			plugin);

globus_result_t
globus_ftp_client_test_perf_plugin_destroy(
    globus_ftp_client_plugin_t *			plugin);

#ifdef __cplusplus
}
#endif

#endif /* GLOBUS_INCLUDE_FTP_CLIENT_TEST_PERF_PLUGIN_H */
