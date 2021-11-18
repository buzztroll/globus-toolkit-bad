
#ifndef GLOBUS_I_GFS_CONFIG_H
#define GLOBUS_I_GFS_CONFIG_H

#define globus_i_gfs_config_list    (globus_list_t *) globus_i_gfs_config_get
#define globus_i_gfs_config_string  (char *) globus_i_gfs_config_get
#define globus_i_gfs_config_bool    (globus_bool_t) globus_i_gfs_config_int

int
globus_i_gfs_config_init(
    int                                 argc,
    char **                             argv,
    globus_bool_t                       argv_only);

int
globus_i_gfs_config_init_envs(
    int                                 argc,
    char **                             argv);

void
globus_i_gfs_config_post_init();

void *
globus_i_gfs_config_get(
    const char *                        option_name);

int
globus_i_gfs_config_int(
    const char *                        option_name);

globus_bool_t
globus_i_gfs_config_is_anonymous(
    const char *                        userid);

const char *
globus_i_gfs_config_get_module_name(
    const char *                        client_supplied_name);

globus_bool_t
globus_i_gfs_config_allow_addr(
    const char *                        remote_addr,
    globus_bool_t                       ipc);

globus_result_t
globus_i_gfs_config_hostname_to_address_string(
    char *                              hostname,
    char *                              out_buf,
    int                                 out_buf_len);

void
globus_i_gfs_config_display_usage();

#endif
