#include "globus_gridftp_server_control.h"
#include "globus_i_gridftp_server_control.h"
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/utsname.h>

/*
 *  These commands will only come in one at a time
 */

typedef struct globus_l_gsc_cmd_wrapper_s
{
    globus_i_gsc_op_t *                     op;
    char *                                  strarg;
    char *                                  mod_name;
    char *                                  mod_parms;
    char *                                  path;

    globus_bool_t                           transfer_flag;
    int                                     dc_parsing_alg;
    int                                     max;
    globus_gridftp_server_control_network_protocol_t prt;

    globus_i_gsc_op_type_t                  type;
    int                                     cmd_ndx;

    char **                                 cs;
    int                                     cs_count;
    int                                     reply_code;
} globus_l_gsc_cmd_wrapper_t;

static void
globus_l_gsc_cmd_transfer(
    globus_l_gsc_cmd_wrapper_t *            wrapper);

/*************************************************************************
 *                      simple commands
 *                      ---------------
 ************************************************************************/
static void
globus_l_gsc_cmd_all(
    globus_i_gsc_op_t *                 op,
    const char *                        full_command,
    char **                             cmd_a,
    int                                 argc,
    void *                              user_arg)
{
    /* XXX could make this a switched based on user_arg */
    if(strcmp(cmd_a[0], "STOR") != 0 &&
        strcmp(cmd_a[0], "ESTO") != 0)
    {
        op->server_handle->allocated_bytes = 0;
    }

    /* do logging here */
    globus_gsc_959_finished_command(op, NULL);
}

static void
globus_l_gsc_cmd_stru(
    globus_i_gsc_op_t *                 op,
    const char *                        full_command,
    char **                             cmd_a,
    int                                 argc,
    void *                              user_arg)
{
    char *                              tmp_ptr;

    globus_i_gsc_log(op->server_handle, full_command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_OTHER);
    tmp_ptr = cmd_a[1];
    if((tmp_ptr[0] == 'f' || tmp_ptr[0] == 'F') 
        && tmp_ptr[1] == '\0')
    {
        globus_gsc_959_finished_command(op, _FSMSL("200 STRU F ok.\r\n"));
    }
    else
    {
        globus_gsc_959_finished_command(
            op, _FSMSL("501 Syntax error in parameter.\r\n"));
    }
}

static void
globus_l_gsc_cmd_allo(
    globus_i_gsc_op_t *                 op,
    const char *                        full_command,
    char **                             cmd_a,
    int                                 argc,
    void *                              user_arg)
{
    int                                 sc;
    globus_off_t                        size;

    globus_i_gsc_log(op->server_handle, full_command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_FILE_COMMANDS);

    sc = sscanf(cmd_a[1], "%"GLOBUS_OFF_T_FORMAT, &size);
    if(sc == 1)
    {
        op->server_handle->allocated_bytes = size;
        globus_gsc_959_finished_command(op, _FSMSL("200 NOOP command successful.\r\n"));
    }
    else
    {
        globus_gsc_959_finished_command(
            op, _FSMSL("501 Syntax error in parameters or arguments.\r\n"));
    }
}


/*
 *  simply pings the control channel
 */
static void
globus_l_gsc_cmd_noop(
    globus_i_gsc_op_t *                     op,
    const char *                            full_command,
    char **                                 cmd_a,
    int                                     argc,
    void *                                  user_arg)
{
    globus_i_gsc_log(op->server_handle, full_command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_OTHER);
    globus_gsc_959_finished_command(op, _FSMSL("200 NOOP command successful.\r\n"));
}

static void
globus_l_gsc_cmd_pbsz(
    globus_i_gsc_op_t *                     op,
    const char *                            full_command,
    char **                                 cmd_a,
    int                                     argc,
    void *                                  user_arg)
{
    char *                                  msg;

    globus_i_gsc_log(op->server_handle, full_command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_SECURITY);
    if(strlen(cmd_a[1]) > 10 || 
        (strlen(cmd_a[1]) == 10 && strcmp(cmd_a[1], "4294967296") >= 0))
    {
        msg = globus_common_create_string(_FSMSL("501 Bad value for PBSZ: %s\r\n"),
            cmd_a[1]);
    }
    else
    {
        msg = globus_common_create_string(_FSMSL("200 PBSZ=%s\r\n"), cmd_a[1]);
    }
    globus_gsc_959_finished_command(op, msg);
    globus_free(msg);
}

static void
globus_l_gsc_cmd_dcau(
    globus_i_gsc_op_t *                     op,
    const char *                            full_command,
    char **                                 cmd_a,
    int                                     argc,
    void *                                  user_arg)
{
    char *                                  tmp_ptr;
    char *                                  msg;

    tmp_ptr = cmd_a[1];
    if(tmp_ptr[1] != '\0')
    {
        globus_gsc_959_finished_command(op, _FSMSL("504 Bad DCAU mode.\r\n"));
        return;
    }

    globus_i_gsc_log(op->server_handle, full_command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_SECURITY);

    *tmp_ptr = toupper(*tmp_ptr);
    switch(*tmp_ptr)
    {
        case 'S':
            if(op->server_handle->del_cred == NULL)
            {
                globus_gsc_959_finished_command(
                    op, _FSMSL("504 No delegated credential.\r\n"));
            }
            else if(argc < 3)
            {
                globus_gsc_959_finished_command(
                    op, _FSMSL("501 DCAU S expected subject.\r\n"));
            }
            else
            {
                op->server_handle->dcau = *tmp_ptr;
                if(op->server_handle->dcau_subject != NULL)
                {
                    globus_free(op->server_handle->dcau_subject);
                }
                op->server_handle->dcau_subject = strdup(cmd_a[2]);
                globus_i_guc_command_data_destroy(op->server_handle);
                globus_gsc_959_finished_command(op, _FSMSL("200 DCAU S.\r\n"));
            }
            break;

        case 'A':
            /* if no del cred return error else fall through */
            if(op->server_handle->del_cred == NULL)
            {
                globus_gsc_959_finished_command(
                    op, _FSMSL("504 No delegated credential.\r\n"));
                break;
            }
        case 'N':
            if(argc != 2)
            {
                globus_gsc_959_finished_command(
                    op, _FSMSL("501 Bad Parameter to DCAU.\r\n"));
            }
            else
            {
                msg = globus_common_create_string(_FSMSL("200 DCAU %c.\r\n"), *tmp_ptr);
                op->server_handle->dcau = *tmp_ptr;
                globus_i_guc_command_data_destroy(op->server_handle);
                globus_gsc_959_finished_command(op, msg);
                globus_free(msg);
            }
            break;

        default:
            globus_gsc_959_finished_command(op, _FSMSL("501 Bad DCAU mode.\r\n"));
            break;
    }
}

static void
globus_l_gsc_cmd_trev(
    globus_i_gsc_op_t *                     op,
    const char *                            full_command,
    char **                                 cmd_a,
    int                                     argc,
    void *                                  user_arg)
{
    char *                                  event_name;
    char *                                  info;
    int                                     frequency;
    int                                     sc;

    globus_i_gsc_log(op->server_handle, full_command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_TRANSFER_STATE);
    for(event_name = cmd_a[1]; *event_name != '\0'; event_name++)
    {
        *event_name = toupper(*event_name);
    }
    event_name = cmd_a[1];
    sc = sscanf(cmd_a[2], "%d", &frequency);
    if(sc != 1)
    {
        globus_gsc_959_finished_command(op, _FSMSL("501 Bad paramter mode.\r\n"));
    }
    info = cmd_a[3];

    if(strcmp(event_name, "RESTART") == 0)
    {
        op->server_handle->opts.restart_frequency = frequency;
        globus_gsc_959_finished_command(op, _FSMSL("200 Command Successful.\r\n"));
    }
    else if(strcmp(event_name, "PERF") == 0)
    {
        op->server_handle->opts.perf_frequency = frequency;
        globus_gsc_959_finished_command(op, _FSMSL("200 Command Successful.\r\n"));
    }
    else
    {
        globus_gsc_959_finished_command(op, _FSMSL("502 Unsupported event.\r\n"));
    }
}

static void
globus_l_gsc_cmd_prot(
    globus_i_gsc_op_t *                     op,
    const char *                            full_command,
    char **                                 cmd_a,
    int                                     argc,
    void *                                  user_arg)
{
    char *                                  tmp_ptr;
    char *                                  msg;

    globus_i_gsc_log(op->server_handle, full_command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_SECURITY);

    tmp_ptr = cmd_a[1];
    if(tmp_ptr[1] != '\0')
    {
        msg = globus_common_create_string(
            _FSMSL("536 %s protection level not supported.\r\n"), cmd_a[1]);
        globus_gsc_959_finished_command(op, msg);
        globus_free(msg);
        return;
    }

    *tmp_ptr = toupper(*tmp_ptr);
    switch(*tmp_ptr)
    {
        case 'P':
        case 'S':
            if(op->server_handle->del_cred == NULL)
            {
                msg = globus_common_create_string(
                    _FSMSL("536 %s protection level not supported.\r\n"), cmd_a[1]);
                break;
            }
        case 'C':
            msg = globus_common_create_string(
                _FSMSL("200 Protection level set to %c.\r\n"), *tmp_ptr);
            op->server_handle->prot = *tmp_ptr;
            globus_i_guc_command_data_destroy(op->server_handle);
            break;

        default:
            msg = globus_common_create_string(
                _FSMSL("536 %s protection level not supported.\r\n"), cmd_a[1]);
            break;
    }

    globus_gsc_959_finished_command(op, msg);
    globus_free(msg);
}

static void
globus_l_gsc_cmd_mdtm_cb(
    globus_i_gsc_op_t *                     op,
    globus_gridftp_server_control_response_t response_type,
    char *                                  response_msg,
    char *                                  path,
    globus_gridftp_server_control_stat_t *  stat_info,
    int                                     stat_count,
    uid_t                                   uid,
    void *                                  user_arg)
{
    int                                     code;
    char *                                  tmp_ptr;
    struct tm *                             tm;
    char *                                  msg;
    GlobusGridFTPServerName(globus_l_gsc_cmd_mdtm_cb);

    if(response_type != GLOBUS_GRIDFTP_SERVER_CONTROL_RESPONSE_SUCCESS)
    {
        switch(response_type)
        {
            default:
                code = 500;
                /* TODO: evaulated error type */
                msg = globus_libc_strdup("Command failed");
                break;
        }
    }
    else
    {
        tm = gmtime(&stat_info[0].mtime);
        code = 213;
        msg =  globus_common_create_string(
            "%04d%02d%02d%02d%02d%02d",
            tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, 
            tm->tm_hour, tm->tm_min, tm->tm_sec);
    }

    if(response_msg != NULL)
    {
        tmp_ptr = msg;
        msg = globus_common_create_string("%s : %s", msg, response_msg);
        free(tmp_ptr);
    }
    tmp_ptr = globus_i_gsc_string_to_959(code, msg, NULL);
    globus_gsc_959_finished_command(op, tmp_ptr);
    globus_free(tmp_ptr);
    globus_free(msg);
}

static void
globus_l_gsc_cmd_mdtm(
    globus_i_gsc_op_t *                     op,
    const char *                            full_command,
    char **                                 cmd_a,
    int                                     argc,
    void *                                  user_arg)
{
    globus_result_t                         res;

    globus_i_gsc_log(op->server_handle, full_command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_LIST);

    res = globus_i_gsc_resource_query(
            op,
            cmd_a[1],
            GLOBUS_GRIDFTP_SERVER_CONTROL_RESOURCE_FILE_ONLY,
            globus_l_gsc_cmd_mdtm_cb,
            NULL);
    if(res != GLOBUS_SUCCESS)
    {
        globus_gsc_959_finished_command(op, _FSMSL("500 Command not supported.\r\n"));
    }
}

/*
 *  mode
 */
static void
globus_l_gsc_cmd_mode(
    globus_i_gsc_op_t *                     op,
    const char *                            full_command,
    char **                                 cmd_a,
    int                                     argc,
    void *                                  user_arg)
{
    char *                                  msg;
    char                                    ch;

    globus_i_gsc_log(op->server_handle, full_command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_TRANSFER_STATE);
    ch = (char)toupper((int)cmd_a[1][0]);
    if(strchr(op->server_handle->modes, ch) == NULL)
    {
        msg = globus_common_create_string(
            _FSMSL("501 '%s' unrecognized transfer mode.\r\n"), full_command);
    }
    else
    {
        msg = globus_common_create_string(_FSMSL("200 Mode set to %c.\r\n"), ch);
        op->server_handle->mode = ch;
    }
    if(msg == NULL)
    {
        globus_i_gsc_command_panic(op);
    }
    else
    {
        globus_gsc_959_finished_command(op, msg);
        globus_free(msg);
    }
}

/*
 *  type
 */
static void
globus_l_gsc_cmd_type(
    globus_i_gsc_op_t *                     op,
    const char *                            full_command,
    char **                                 cmd_a,
    int                                     argc,
    void *                                  user_arg)
{
    char                                    ch;
    char *                                  msg;
    GlobusGridFTPServerName(globus_l_gsc_cmd_type);

    globus_i_gsc_log(op->server_handle, full_command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_TRANSFER_STATE);
    ch = (char)toupper((int)cmd_a[1][0]);
    if(strchr(op->server_handle->types, ch) == NULL)
    {
        msg = globus_common_create_string(
            _FSMSL("501 '%s' unrecognized type.\r\n"), full_command);
    }
    else
    {
        msg = globus_common_create_string(_FSMSL("200 Type set to %c.\r\n"), ch);
        op->server_handle->type = ch;
    }
    if(msg == NULL)
    {
        globus_i_gsc_command_panic(op);
    }
    else
    {
        globus_gsc_959_finished_command(op, msg);
        globus_free(msg);
    }
}

/*
 *  lang
 */
static void
globus_l_gsc_cmd_lang(
    globus_i_gsc_op_t *                     op,
    const char *                            full_command,
    char **                                 cmd_a,
    int                                     argc,
    void *                                  user_arg)
{
    char *                                  lang = NULL;

    
    char *                                  msg;

    globus_i_gsc_log(op->server_handle, full_command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_TRANSFER_STATE);
    if (cmd_a[1]==NULL)
    {
	    /*nothing specified after LANG, keep default*/
        msg = globus_common_create_string(_FSMSL("200 lang set to %s.\r\n"), lang);
	op->server_handle->lang = lang;
    }
    else
    {
        lang = strdup(cmd_a[1]);
    }

    if(lang == NULL)
    {
        msg = globus_common_create_string(_FSMSL("200 lang set to %s.\r\n"), "EN");
    }
    else
    {
	/*Check if it is a supported language*/
	/*As internationalization continues, this will be discovered
	 * from configuration--hardcoded for now
	 */
	if (strcmp(lang, "EN")==0)
	{	
           msg = globus_common_create_string(_FSMSL("200 lang set to %s.\r\n"), lang);
           op->server_handle->lang = lang;
	}
	else
	{
           msg = globus_common_create_string(
            _FSMSL("501 '%s' unrecognized language.\r\n"), full_command);
	}
    }
    if(msg == NULL)
    {
        globus_i_gsc_command_panic(op);
    }
    else
    {
        globus_gsc_959_finished_command(op, msg);
        globus_free(msg);
    }
}

/*************************************************************************
 *                      directory functions
 *                      -------------------
 ************************************************************************/
/*
 *  PWD
 */
static void
globus_l_gsc_cmd_pwd(
    globus_i_gsc_op_t *                     op,
    const char *                            full_command,
    char **                                 cmd_a,
    int                                     argc,
    void *                                  user_arg)
{
    char *                                  msg;
    GlobusGridFTPServerName(globus_l_gsc_cmd_pwd);

    globus_i_gsc_log(op->server_handle, full_command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_OTHER);
    msg = globus_common_create_string(
        _FSMSL("257 \"%s\" is current directory.\r\n"), op->server_handle->cwd);
    if(msg == NULL)
    {
        globus_i_gsc_command_panic(op);
    }
    else
    {
        globus_gsc_959_finished_command(op, msg);
        globus_free(msg);
    }
}

/*
 *  CWD
 */
static void
globus_l_gsc_cmd_cwd_cb(
    globus_i_gsc_op_t *                     op,
    globus_gridftp_server_control_response_t response_type,
    char *                                  response_msg,
    char *                                  path,
    globus_gridftp_server_control_stat_t *  stat_info,
    int                                     stat_count,
    uid_t                                   uid,
    void *                                  user_arg)
{
    int                                     code;
    char *                                  l_path;
    char *                                  msg = NULL;
    char *                                  tmp_ptr;
    GlobusGridFTPServerName(globus_l_gsc_cmd_cwd_cb);

    /*
     *  decide what message to send
     */
    if(response_type != GLOBUS_GRIDFTP_SERVER_CONTROL_RESPONSE_SUCCESS ||
        stat_count < 1)
    {
        switch(response_type)
        {
            case GLOBUS_GRIDFTP_SERVER_CONTROL_RESPONSE_PATH_INVALID:
                code = 550;
                msg = globus_common_create_string(
                    _FSMSL("%s: No such file or directory."), path);
                break;

            case GLOBUS_GRIDFTP_SERVER_CONTROL_RESPONSE_ACCESS_DENINED:
                code = 553;
                msg = globus_common_create_string(_FSMSL("Permission denied."));
                break;

            default:
                code = 550;
                msg = globus_common_create_string(
                    _FSMSL("%s: Could not change directory."), path);
                break;
        }
    }
    else if(!S_ISDIR(stat_info->mode))
    {
        code = 550;
        msg = globus_common_create_string(_FSMSL("%s: Not a directory."), path);
    }
    else
    {
        if(!(S_IXOTH & stat_info->mode && S_IROTH & stat_info->mode) &&
            !(stat_info->uid == uid && 
                S_IXUSR & stat_info->mode && S_IRUSR & stat_info->mode))
        {
            code = 550;
            msg = globus_common_create_string(_FSMSL("%s: Permission denied"), path);
        }
        else
        {
            l_path = globus_i_gsc_concat_path(op->server_handle, path);
            if(l_path == NULL)
            {
                code = 550;
                msg = globus_common_create_string(_FSMSL("Could not change directory."),
                    path);
            }
            else
            {
                if(op->server_handle->cwd != NULL)
                {
                    globus_free(op->server_handle->cwd);
                }
                op->server_handle->cwd = globus_libc_strdup(path);
                code = 250;
                msg = globus_libc_strdup(_FSMSL("CWD command successful."));
            }
        }
    }
    if(msg == NULL)
    {
        globus_i_gsc_command_panic(op);
        goto err;
    }
    if(response_msg != NULL)
    {
        tmp_ptr = msg;
        msg = globus_common_create_string("%s : %s", msg, response_msg);
        free(tmp_ptr);
    }
    tmp_ptr = globus_i_gsc_string_to_959(code, msg, NULL);
    globus_gsc_959_finished_command(op, tmp_ptr);
    globus_free(tmp_ptr);
    globus_free(msg);

    return;

  err:

    if(l_path != NULL)
    {
        globus_free(l_path);
    }
    if(msg != NULL)
    {
        globus_free(msg);
    }
}

static void
globus_l_gsc_cmd_cwd(
    globus_i_gsc_op_t *                     op,
    const char *                            full_command,
    char **                                 cmd_a,
    int                                     argc,
    void *                                  user_arg)
{
    globus_result_t                         res;
    int                                     mask = GLOBUS_GRIDFTP_SERVER_CONTROL_RESOURCE_FILE_ONLY;
    char *                                  path = NULL;
    GlobusGridFTPServerName(globus_l_gsc_cmd_cwd);

    globus_i_gsc_log(op->server_handle, full_command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_OTHER);
    if(strcmp(cmd_a[0], "CDUP") == 0 && argc == 1)
    {
        path = globus_libc_strdup("..");
        if(path == NULL)
        {
            globus_i_gsc_command_panic(op);
            goto err;
        }
    }
    else if(argc == 2)
    {
        path = globus_i_gsc_concat_path(op->server_handle, cmd_a[1]);
        if(path == NULL)
        {
            globus_gsc_959_finished_command(op,
                _FSMSL("550 Could not change directory.\r\n"));
            goto err;
        }
    }
    else
    {
        globus_gsc_959_finished_command(op,
            _FSMSL("550 Could not change directory.\r\n"));
        goto err;
    }

    res = globus_i_gsc_resource_query(
            op,
            path,
            mask,
            globus_l_gsc_cmd_cwd_cb,
            NULL);
    if(res != GLOBUS_SUCCESS)
    {
        goto err;
    }
    globus_free(path);

    return;

  err:
    if(path != NULL)
    {
        globus_free(path);
    }
}

/*
 *  STAT
 */
static void
globus_l_gsc_cmd_stat_cb(
    globus_i_gsc_op_t *                     op,
    globus_gridftp_server_control_response_t response_type,
    char *                                  response_msg,
    char *                                  path,
    globus_gridftp_server_control_stat_t *  stat_info,
    int                                     stat_count,
    uid_t                                   uid,
    void *                                  user_arg)
{
    int                                     code;
    char *                                  msg;
    char *                                  tmp_ptr;
    GlobusGridFTPServerName(globus_l_gsc_cmd_stat_cb);

    if(response_type != GLOBUS_GRIDFTP_SERVER_CONTROL_RESPONSE_SUCCESS)
    {
        switch(response_type)
        {
            case GLOBUS_GRIDFTP_SERVER_CONTROL_RESPONSE_PATH_INVALID:
                code = 550;
                msg = globus_common_create_string(
                    _FSMSL("No such file or directory."));
                break;

            case GLOBUS_GRIDFTP_SERVER_CONTROL_RESPONSE_ACCESS_DENINED:
                code = 553;
                msg = globus_common_create_string(
                    _FSMSL("Permission denied."));
                break;

            default:
                code = 500;
                msg = globus_libc_strdup(_FSMSL("Command failed"));
                break;
        }
    }
    else
    {
        if((int)user_arg == 0)
        {
            code = 213;
            tmp_ptr = globus_i_gsc_list_single_line(stat_info);
        }
        else
        {
            code = 250;
            /* for mlst we want the requested path, not just the filename */
            snprintf(stat_info->name, MAXPATHLEN, "%s", path);
            tmp_ptr = globus_i_gsc_mlsx_line_single(
                op->server_handle->opts.mlsx_fact_str, uid, stat_info);
        }
        msg =  globus_common_create_string(
            _FSMSL("status of %s\n %s\n"),
            op->path, tmp_ptr);
        globus_free(tmp_ptr);
    }

    if(response_msg != NULL)
    {
        tmp_ptr = msg;
        msg = globus_common_create_string("%s : %s", msg, response_msg);
        free(tmp_ptr);
    }
    /* set a blank preline -- mlst output already has the initial space */
    tmp_ptr = globus_i_gsc_string_to_959(code, msg, "");
    globus_gsc_959_finished_command(op, tmp_ptr);
    globus_free(tmp_ptr);
    globus_free(msg);
}

static void
globus_l_gsc_cmd_stat(
    globus_i_gsc_op_t *                     op,
    const char *                            full_command,
    char **                                 cmd_a,
    int                                     argc,
    void *                                  user_arg)
{
    /* these are really just place holders in the list */
    int                                     mask = GLOBUS_GRIDFTP_SERVER_CONTROL_RESOURCE_FILE_ONLY;
    char *                                  msg = NULL;
    char *                                  path;
    globus_result_t                         res;
    GlobusGridFTPServerName(globus_l_gsc_cmd_stat);

    globus_i_gsc_log(op->server_handle, full_command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_OTHER);
    if(argc == 1 && user_arg == 0)
    {
        msg = globus_common_create_string(
                _FSMSL("212 GridFTP server status.\r\n"));
        if(msg == NULL)
        {
            globus_i_gsc_command_panic(op);
            goto err;
        }
        globus_gsc_959_finished_command(op, msg);
        globus_free(msg);
    }
    else
    {
        if(argc != 2)
        {
            path = op->server_handle->cwd;
        }
        else
        {
            path = cmd_a[1];
        }
        res = globus_i_gsc_resource_query(
                op,
                path,
                mask,
                globus_l_gsc_cmd_stat_cb,
                user_arg);
        if(res != GLOBUS_SUCCESS)
        {
            globus_gsc_959_finished_command(
                op, _FSMSL("500 Command not supported.\r\n"));
        }
    }

    return;

  err:
    return;
}

/*
 *  size and mdtm
 */
static void
globus_l_gsc_cmd_size_cb(
    globus_i_gsc_op_t *                     op,
    globus_gridftp_server_control_response_t response_type,
    char *                                  response_msg,
    char *                                  path,
    globus_gridftp_server_control_stat_t *  stat_info,
    int                                     stat_count,
    uid_t                                   uid,
    void *                                  user_arg)
{
    int                                     code;
    char *                                  tmp_ptr;
    char *                                  msg = NULL;
    GlobusGridFTPServerName(globus_l_gsc_cmd_size_cb);

    if(response_type != GLOBUS_GRIDFTP_SERVER_CONTROL_RESPONSE_SUCCESS ||
        stat_count < 1)
    {
        switch(response_type)
        {
            case GLOBUS_GRIDFTP_SERVER_CONTROL_RESPONSE_PATH_INVALID:
                code = 550;
                msg = globus_common_create_string(
                    _FSMSL("No such file."));
                break;

            case GLOBUS_GRIDFTP_SERVER_CONTROL_RESPONSE_ACCESS_DENINED:
                code = 553;
                msg = globus_common_create_string(
                    _FSMSL("Permission denied."));
                break;

            default:
                code = 550;
                msg = globus_libc_strdup(_FSMSL("Command failed"));
                break;
        }
    }
    else
    {
        code = 213;
        msg = globus_common_create_string("%d", stat_info->size);
    }
    if(msg == NULL)
    {
        globus_i_gsc_command_panic(op);
        goto err;
    }
    if(response_msg != NULL)
    {
        tmp_ptr = msg;
        msg = globus_common_create_string("%s : %s", msg, response_msg);
        free(tmp_ptr);
    }
    tmp_ptr = globus_i_gsc_string_to_959(code, msg, NULL);
    globus_gsc_959_finished_command(op, tmp_ptr);
    globus_free(tmp_ptr);
    globus_free(msg);
    
    return;
    
  err:
    if(msg != NULL)
    {
        globus_free(msg);
    }
}

static void
globus_l_gsc_cmd_size(
    globus_i_gsc_op_t *                     op,
    const char *                            full_command,
    char **                                 cmd_a,
    int                                     argc,
    void *                                  user_arg)
{
    /* these are really just place holders in the list */
    char *                                  path = NULL;
    int                                     mask = GLOBUS_GRIDFTP_SERVER_CONTROL_RESOURCE_FILE_ONLY;
    globus_result_t                         res;
    GlobusGridFTPServerName(globus_l_gsc_cmd_size);

    globus_i_gsc_log(op->server_handle, full_command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_FILE_COMMANDS);
    path = strdup(cmd_a[1]);
    if(path == NULL)
    {
        globus_i_gsc_command_panic(op);
        goto err;
    }
    res = globus_i_gsc_resource_query(
        op,
        path,
        mask,
        globus_l_gsc_cmd_size_cb,
        NULL);
    if(res != GLOBUS_SUCCESS)
    {
        globus_i_gsc_command_panic(op);
        goto err;
    }
    globus_free(path);

    return;

  err:
    if(path != NULL)
    {
        globus_free(path);
    }
}

/*
 *  quit
 */
static void
globus_l_gsc_cmd_quit(
    globus_i_gsc_op_t *                     op,
    const char *                            full_command,
    char **                                 cmd_a,
    int                                     argc,
    void *                                  user_arg)
{
    globus_i_gsc_server_handle_t *          server_handle;

    server_handle = op->server_handle;

    globus_i_gsc_log(op->server_handle, full_command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_SECURITY);

    globus_gsc_959_finished_command(op, _FSMSL("221 Goodbye.\r\n"));

    globus_i_gsc_terminate(server_handle);
}

/*************************************************************************
 *                      authentication commands
 *                      -----------------------
 ************************************************************************/
/*
 *   USER
 */
static void
globus_l_gsc_cmd_user(
    globus_i_gsc_op_t *                     op,
    const char *                            full_command,
    char **                                 cmd_a,
    int                                     argc,
    void *                                  user_arg)
{
    char *                                  msg;
    GlobusGridFTPServerName(globus_l_gsc_cmd_user);

    globus_i_gsc_log(op->server_handle, full_command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_SECURITY);
    if(op->server_handle->username != NULL)
    {
        globus_free(op->server_handle->username);
        op->server_handle->username = NULL;
    }
    op->server_handle->username = globus_libc_strdup(cmd_a[1]);
    msg = globus_common_create_string(
        _FSMSL("331 Password required for %s.\r\n"), op->server_handle->username);
    if(msg == NULL)
    {
        goto err;
    }
    globus_gsc_959_finished_command(op, msg);
    globus_free(msg);
    return;

  err:
    if(op->server_handle->username != NULL)
    {
        globus_free(op->server_handle->username);
    }
    globus_i_gsc_command_panic(op);
}

static void
globus_l_gsc_auth_cb(
    globus_i_gsc_op_t *                     op,
    globus_gridftp_server_control_response_t response_type,
    char *                                  response_msg,
    void *                                  user_arg)
{
    int                                     code;
    char *                                  msg;
    char *                                  tmp_ptr;

    if(response_type == GLOBUS_GRIDFTP_SERVER_CONTROL_RESPONSE_SUCCESS)
    {
        if(op->server_handle->post_auth_banner == NULL)
        {
            code = 230;
            msg = globus_common_create_string(
                _FSMSL("User %s logged in."),
                op->server_handle->username);
        }
        else
        {
            code = 230;
            msg = globus_common_create_string(
                _FSMSL("User %s logged in.\n%s"),
                op->server_handle->username,
                op->server_handle->post_auth_banner);
        }
    }
    else
    {
        code = 530;
        msg = globus_common_create_string(_FSMSL("Login incorrect."));
    }
    globus_i_gsc_log(op->server_handle, op->command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_SECURITY);
    if(response_msg != NULL)
    {
        tmp_ptr = msg;
        msg = globus_common_create_string("%s : %s", msg, response_msg);
        free(tmp_ptr);
    }
    tmp_ptr = globus_i_gsc_string_to_959(code, msg, NULL);
    globus_gsc_959_finished_command(op, tmp_ptr);
    globus_free(tmp_ptr);
    globus_free(msg);
}

/*
 *  pass
 */
static void
globus_l_gsc_cmd_pass(
    globus_i_gsc_op_t *                     op,
    const char *                            full_command,
    char **                                 cmd_a,
    int                                     argc,
    void *                                  user_arg)
{
    char *                                  msg = NULL;
    globus_result_t                         res;
    GlobusGridFTPServerName(globus_l_gsc_cmd_pass);

    /*
     *  if user name has not yet been supplied return error message
     */
    globus_i_gsc_log(op->server_handle, full_command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_SECURITY);
    if(op->server_handle->username == NULL)
    {
        msg = _FSMSL("503 Login with USER first.\r\n");
        if(msg == NULL)
        {
            goto err;
        }
        globus_gsc_959_finished_command(op, msg);
    }
    else
    {
        res = globus_i_gsc_authenticate(
            op,
            op->server_handle->username,
            (argc == 2) ? cmd_a[1] : "",
            globus_l_gsc_auth_cb,
            NULL);
        if(res != GLOBUS_SUCCESS)
        {
            goto err;
        }
    }

    return;

  err:
    globus_i_gsc_command_panic(op);
}

/*
 *  syst
 */
static void
globus_l_gsc_cmd_syst(
    globus_i_gsc_op_t *                     op,
    const char *                            full_command,
    char **                                 cmd_a,
    int                                     argc,
    void *                                  user_arg)
{
    char *                                  msg;
    struct utsname                          uname_info;
    GlobusGridFTPServerName(globus_l_gsc_cmd_syst);

    globus_i_gsc_log(op->server_handle, full_command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_OTHER);
    uname(&uname_info);

    msg = globus_common_create_string(_FSMSL("215 %s.\r\n"), uname_info.sysname);
    if(msg == NULL)
    {
        goto err;
    }
    globus_gsc_959_finished_command(op, msg);
    globus_free(msg);

    return;

  err:
    globus_i_gsc_command_panic(op);
}

static void
globus_l_gsc_cmd_feat(
    globus_i_gsc_op_t *                     op,
    const char *                            full_command,
    char **                                 cmd_a,
    int                                     argc,
    void *                                  user_arg)
{
    char *                                  msg;
    char *                                  tmp_ptr;
    globus_list_t *                         list;

    globus_i_gsc_log(op->server_handle, full_command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_OTHER);
    msg = globus_libc_strdup(_FSMSL("211-Extensions supported\r\n"));
    for(list = op->server_handle->feature_list;
        !globus_list_empty(list);
        list = globus_list_rest(list))
    {
        tmp_ptr = globus_common_create_string("%s %s\r\n", msg,
            (char *)globus_list_first(list));
        globus_free(msg);
        msg = tmp_ptr;
    }
    tmp_ptr = globus_common_create_string(_FSMSL("%s211 End.\r\n"), msg);
    globus_free(msg);

    globus_gsc_959_finished_command(op, tmp_ptr);
    globus_free(tmp_ptr);
}

/*
 *  help
 */
static void
globus_l_gsc_cmd_help(
    globus_i_gsc_op_t *                     op,
    const char *                            full_command,
    char **                                 cmd_a,
    int                                     argc,
    void *                                  user_arg)
{
    int                                     ctr;
    char *                                  msg;
    char *                                  arg;
    GlobusGridFTPServerName(globus_l_gsc_cmd_help);

    globus_i_gsc_log(op->server_handle, full_command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_OTHER);
    /* general help */
    if(argc == 1 || (argc == 2 && strcmp(cmd_a[0], "SITE") == 0))
    {
        arg = NULL;
    }
    else
    {
        if(strcmp(cmd_a[0], "SITE") == 0)
        {
            arg = globus_libc_strdup(cmd_a[2]);
        }
        else
        {
            arg = globus_libc_strdup(cmd_a[1]);
        }
        for(ctr = 0; ctr < strlen(arg); ctr++)
        {
            arg[ctr] = toupper(arg[ctr]);
        }
    }

    msg = globus_i_gsc_get_help(op->server_handle, arg);
    if(arg != NULL)
    {
        globus_free(arg);
    }
    if(msg == NULL)
    {
        goto err;
    }

    globus_gsc_959_finished_command(op, msg);
    globus_free(msg);

    return;

  err:
    globus_i_gsc_command_panic(op);
}

/*
 * opts
 */
static void
globus_l_gsc_cmd_opts(
    globus_i_gsc_op_t *                     op,
    const char *                            full_command,
    char **                                 cmd_a,
    int                                     argc,
    void *                                  user_arg)
{
    globus_bool_t                           done = GLOBUS_FALSE;
    int                                     tmp_i;
    char                                    tmp_s[1024];
    char *                                  msg;
    char *                                  tmp_ptr;
    globus_i_gsc_handle_opts_t *            opts;
    GlobusGridFTPServerName(globus_l_gsc_cmd_opts);

    globus_i_gsc_log(op->server_handle, full_command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_OTHER);
    opts = &op->server_handle->opts;

    for(tmp_ptr = cmd_a[1]; *tmp_ptr != '\0'; tmp_ptr++)
    {
        *tmp_ptr = toupper(*tmp_ptr);
    }
    
    if(argc != 3)
    {
        msg = _FSMSL("500 OPTS failed.\r\n");
    }
    else if(strcmp("RETR", cmd_a[1]) == 0)
    {
        tmp_ptr = cmd_a[2];

        done = GLOBUS_FALSE;
        while(*tmp_ptr != '\0')
        {
            msg = _FSMSL("200 OPTS Command Successful.\r\n");
            if(sscanf(tmp_ptr, "Parallelism=%d,%*d,%*d;", &tmp_i) == 1)
            {
                opts->parallelism = tmp_i;
            }
            else if(sscanf(tmp_ptr, "PacketSize=%d;", &tmp_i) == 1)
            {
                opts->packet_size = tmp_i;
            }
            else if(sscanf(tmp_ptr, "WindowSize=%d;", &tmp_i) == 1)
            {
                opts->send_buf = tmp_i;
            }
            else if(sscanf(tmp_ptr, "StripeLayout=%s;", tmp_s) == 1)
            {
                /* XXX can't scan for strings like that */
            }
            else if(sscanf(tmp_ptr, "BlockSize=%d;", &tmp_i) == 1)
            {
            }
            else
            {
                msg = _FSMSL("500 OPTS failed.\r\n");
                done = GLOBUS_TRUE;
            }
            tmp_ptr = strchr(tmp_ptr, ';');
            if(tmp_ptr == NULL)
            {
                msg = _FSMSL("500 OPTS failed.\r\n");
                done = GLOBUS_TRUE;
            }
            tmp_ptr++;
        }
    }
    else if(strcmp("PASV", cmd_a[1]) == 0 || 
        strcmp("SPAS", cmd_a[1]) == 0 ||
        strcmp("EPSV", cmd_a[1]) == 0)
    {
        msg = _FSMSL("200 OPTS Command Successful.\r\n");
        if(sscanf(cmd_a[2], "AllowDelayed=%d", &tmp_i) == 1)
        {
            opts->delayed_passive = tmp_i;
        }
        else
        {
            msg = _FSMSL("500 OPTS failed.\r\n");
        }
    }
    else if(strcmp("MLST", cmd_a[1]) == 0 || 
        strcmp("MLSD", cmd_a[1]) == 0)
    {
        tmp_ptr = opts->mlsx_fact_str;
        if(strstr(cmd_a[2], "type"))
        {
            *tmp_ptr = GLOBUS_GSC_MLSX_FACT_TYPE;
            tmp_ptr++;
        }
        if(strstr(cmd_a[2], "modify"))
        {
            *tmp_ptr = GLOBUS_GSC_MLSX_FACT_MODIFY;
            tmp_ptr++;
        }
        if(strstr(cmd_a[2], "charset"))
        {
            *tmp_ptr = GLOBUS_GSC_MLSX_FACT_CHARSET;
            tmp_ptr++;
        }
        if(strstr(cmd_a[2], "size"))
        {
            *tmp_ptr = GLOBUS_GSC_MLSX_FACT_SIZE;
            tmp_ptr++;
        }
        if(strstr(cmd_a[2], "perm"))
        {
            *tmp_ptr = GLOBUS_GSC_MLSX_FACT_PERM;
            tmp_ptr++;
        }
        if(strstr(cmd_a[2], "unix.mode"))
        {
            *tmp_ptr = GLOBUS_GSC_MLSX_FACT_UNIXMODE;
            tmp_ptr++;
        }
        if(strstr(cmd_a[2], "unix.slink"))
        {
            *tmp_ptr = GLOBUS_GSC_MLSX_FACT_UNIXSLINK;
            tmp_ptr++;
        }
        if(strstr(cmd_a[2], "unique"))
        {
            *tmp_ptr = GLOBUS_GSC_MLSX_FACT_UNIQUE;
            tmp_ptr++;
        }
        msg = _FSMSL("200 OPTS Command Successful.\r\n");
    }
    else
    {
        msg = _FSMSL("500 OPTS failed.\r\n");
    }

    globus_gsc_959_finished_command(op, msg);
}

/*
 *
 */
static void
globus_l_gsc_cmd_sbuf(
    globus_i_gsc_op_t *                     op,
    const char *                            full_command,
    char **                                 cmd_a,
    int                                     argc,
    void *                                  user_arg)
{
    int                                     sc;
    int                                     tmp_i;
    GlobusGridFTPServerName(globus_l_gsc_cmd_sbuf);

    globus_i_gsc_log(op->server_handle, full_command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_TRANSFER_STATE);
    if(argc != 2)
    {
        globus_gsc_959_finished_command(op, _FSMSL("502 Invalid Parameter.\r\n"));
    }
    else
    {
        sc = sscanf(cmd_a[1], "%d", &tmp_i);
        if(sc != 1)
        {
            globus_gsc_959_finished_command(
                op, _FSMSL("502 Invalid Parameter.\r\n"));
        }
        else
        {
            op->server_handle->opts.send_buf = tmp_i;
            op->server_handle->opts.receive_buf = tmp_i;

            globus_gsc_959_finished_command(
                op, _FSMSL("200 SBUF Command Successful.\r\n"));
        }
    }
}

/*
 *
 */
static void
globus_l_gsc_cmd_site_sbuf(
    globus_i_gsc_op_t *                 op,
    const char *                        full_command,
    char **                             cmd_a,
    int                                 argc,
    void *                              user_arg)
{
    int                                 tmp_i;
    int                                 sc;

    globus_i_gsc_log(op->server_handle, full_command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_TRANSFER_STATE);
    sc = sscanf(cmd_a[2], "%d", &tmp_i);
    if(sc != 1)
    {
        globus_gsc_959_finished_command(op,
                _FSMSL("501 Syntax error in parameters or arguments.\r\n"));
    }
    else
    {
        op->server_handle->opts.send_buf = tmp_i;
        op->server_handle->opts.receive_buf = tmp_i;
        globus_gsc_959_finished_command(op, _FSMSL("200 Site Command Successful.\r\n"));
    }
}

static void
globus_l_gsc_cmd_site_receive_buf(
    globus_i_gsc_op_t *                 op,
    const char *                        full_command,
    char **                             cmd_a,
    int                                 argc,
    void *                              user_arg)
{
    int                                 tmp_i;
    int                                 sc;

    globus_i_gsc_log(op->server_handle, full_command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_TRANSFER_STATE);
    sc = sscanf(cmd_a[2], "%d", &tmp_i);
    if(sc != 1)
    {
        globus_gsc_959_finished_command(op,
                _FSMSL("501 Syntax error in parameters or arguments.\r\n"));
    }
    else
    {
        op->server_handle->opts.receive_buf = tmp_i;
        globus_gsc_959_finished_command(op, _FSMSL("200 Site Command Successful.\r\n"));
    }
}

static void
globus_l_gsc_cmd_site_send_buf(
    globus_i_gsc_op_t *                 op,
    const char *                        full_command,
    char **                             cmd_a,
    int                                 argc,
    void *                              user_arg)
{
    int                                 tmp_i;
    int                                 sc;

    globus_i_gsc_log(op->server_handle, full_command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_TRANSFER_STATE);
    sc = sscanf(cmd_a[2], "%d", &tmp_i);
    if(sc != 1)
    {
        globus_gsc_959_finished_command(op,
                _FSMSL("501 Syntax error in parameters or arguments.\r\n"));
    }
    else
    {
        op->server_handle->opts.send_buf = tmp_i;
        globus_gsc_959_finished_command(op, _FSMSL("200 Site Command Successful.\r\n"));
    }
}

/*
 *
 */
static void
globus_l_gsc_cmd_site_fault(
    globus_i_gsc_op_t *                 op,
    const char *                        full_command,
    char **                             cmd_a,
    int                                 argc,
    void *                              user_arg)
{
    int                                 len;
    int                                 ctr;
  
    globus_i_gsc_log(op->server_handle, full_command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_SITE);

    if(op->server_handle->fault_cmd != NULL)
    {
        globus_free(op->server_handle->fault_cmd);
    }
    op->server_handle->fault_cmd = globus_libc_strdup(cmd_a[2]);

    len = strlen(op->server_handle->fault_cmd);
    for(ctr = 0; ctr < len; ctr++)
    {
        op->server_handle->fault_cmd[ctr] = 
            toupper(op->server_handle->fault_cmd[ctr]);
    }
    
    globus_gsc_959_finished_command(
        op, "200 Site Command Successful.\r\n");

}

static void
globus_l_gsc_cmd_rest(
    globus_i_gsc_op_t *                     op,
    const char *                            full_command,
    char **                                 cmd_a,
    int                                     argc,
    void *                                  user_arg)
{
    globus_range_list_t                     range_list;
    globus_off_t                            start_offset;
    globus_off_t                            end_offset;
    int                                     sc;
    char *                                  tmp_ptr;

    globus_i_gsc_log(op->server_handle, full_command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_TRANSFER_STATE);

    globus_range_list_init(&range_list);
    /* mode s */
    if(strchr(cmd_a[1], '-') == NULL)
    {
        sc = sscanf(cmd_a[1], "%"GLOBUS_OFF_T_FORMAT, &end_offset);
        if(sc != 1)
        {
            globus_gsc_959_finished_command(op, _FSMSL("501 bad parameter.\r\n"));
            globus_range_list_destroy(range_list);
            return;
        }

        globus_range_list_insert(range_list, 0, end_offset);
    }
    /* mode e */
    else
    {
        tmp_ptr = cmd_a[1];
        while(tmp_ptr != NULL)
        {
            sc = sscanf(tmp_ptr, 
                "%"GLOBUS_OFF_T_FORMAT"-%"GLOBUS_OFF_T_FORMAT, 
                &start_offset, &end_offset);
            if(sc != 2)
            {
                globus_gsc_959_finished_command(
                    op, _FSMSL("501 bad paremeter.\r\n"));
                globus_range_list_destroy(range_list);
                return;
            }

            globus_range_list_insert(
                range_list, start_offset, end_offset - start_offset);
            tmp_ptr = strchr(tmp_ptr, ',');
            if(tmp_ptr)
            {
                tmp_ptr++;
            }
        }
    }
    if(op->server_handle->range_list != NULL)
    {
        globus_range_list_destroy(op->server_handle->range_list);
    }
    op->server_handle->range_list = range_list;
    globus_gsc_959_finished_command(op, 
        _FSMSL("350 Restart Marker OK. Send STORE or RETR to initiate transfer.\r\n"));
}

/*************************************************************************
 *                  data connection esstablishement
 *                  -------------------------------
 ************************************************************************/
static void
globus_l_gsc_cmd_pasv_cb(
    globus_i_gsc_op_t *                     op,
    globus_gridftp_server_control_response_t response_type,
    char *                                  response_msg,
    const char **                           cs,
    int                                     addr_count,
    void *                                  user_arg)
{
    int                                     ctr;
    char *                                  tmp_ptr;
    char *                                  host;
    int                                     host_ip[16];
    int                                     ip_count;
    unsigned short                          port;
    int                                     sc;
    char *                                  msg = NULL;
    globus_l_gsc_cmd_wrapper_t *            wrapper = NULL;
    GlobusGridFTPServerName(globus_l_gsc_cmd_pasv_cb);

    wrapper = (globus_l_gsc_cmd_wrapper_t *) user_arg;
    wrapper->op = op;

    if(response_type != GLOBUS_GRIDFTP_SERVER_CONTROL_RESPONSE_SUCCESS)
    {
        /* TODO: evaulated error type */
        globus_gsc_959_finished_command(op, _FSMSL("500 Command failed.\r\n"));
        goto err;
    }
    else if(addr_count > wrapper->max && wrapper->max != -1)
    {
        globus_gsc_959_finished_command(wrapper->op, _FSMSL("500 Command failed.\r\n"));
        goto err;
    }
    else if(wrapper->dc_parsing_alg == 0)
    {
        /* if pasv */
        if(wrapper->cmd_ndx == 1)
        {
            if(globus_libc_contact_string_to_ints(
                cs[0], host_ip, &ip_count, &port) != GLOBUS_SUCCESS)
            {
                globus_gsc_959_finished_command(
                    wrapper->op, "500 Resource error.\r\n");
                goto err;
            }
            if(ip_count > 4)
            {
                globus_gsc_959_finished_command(
                    wrapper->op, "522 Network protocol not supported.\r\n");
                goto err;
            }
            
            msg = globus_common_create_string(
                "%d Entering Passive Mode (%d,%d,%d,%d,%d,%d)\r\n",
                    wrapper->reply_code,
                    host_ip[0],
                    host_ip[1],
                    host_ip[2],
                    host_ip[3],
                    (int) (port / 256),
                    (int) (port % 256));
        }
        else
        {
            /* allow SPAS to work until real striping gets done */
            if(addr_count == -1)
            {
                addr_count = 1;
            }
            msg =  globus_common_create_string(
                "%d-Entering Striped Passive Mode.\r\n", 
                wrapper->reply_code);
            for(ctr = 0; ctr < addr_count; ctr++)
            {
                if(globus_libc_contact_string_to_ints(
                    cs[ctr], host_ip, &ip_count, &port) != GLOBUS_SUCCESS)
                {
                    globus_gsc_959_finished_command(
                        wrapper->op, "500 Resource problem.\r\n");
                    goto err;
                }
                if(ip_count > 4)
                {
                    globus_gsc_959_finished_command(
                       wrapper->op, "522 Network protocol not supported.\r\n");
                    goto err;
                }
            
                tmp_ptr = globus_common_create_string(
                    "%s %d,%d,%d,%d,%d,%d\r\n",
                    msg,
                    host_ip[0],
                    host_ip[1],
                    host_ip[2],
                    host_ip[3],
                    (int) (port / 256),
                    (int) (port % 256));
                if(tmp_ptr == NULL)
                {
                    globus_gsc_959_finished_command(
                        wrapper->op, "500 Resource error.\r\n");
                    goto err;
                }
                globus_free(msg);
                msg = tmp_ptr;
            }
            tmp_ptr = globus_common_create_string("%s%d End\r\n", 
                msg, wrapper->reply_code);
            if(tmp_ptr == NULL)
            {
                globus_gsc_959_finished_command(
                    wrapper->op, "500 Resource error.\r\n");
                goto err;
            }
            globus_free(msg);
            msg = tmp_ptr;
        }
    }
    else if(wrapper->dc_parsing_alg == 1)
    {
        /* if epsv */
        if(wrapper->cmd_ndx == 2)
        {
            char *                      p;
            char *                      h;
            
            host = globus_libc_strdup(cs[0]);
            if(!host)
            {
                globus_gsc_959_finished_command(
                    wrapper->op, "500 Resource error.\r\n");
                goto err;
            }
            
            p = strrchr(host, ':');
            if(!p || p == host)
            {
                globus_free(host);
                globus_gsc_959_finished_command(
                    wrapper->op, "500 Internal Parse error.\r\n");
                goto err;
            }
            
            h = host;
            if(*cs[0] == '[')
            {
                h++;
                *(p - 1) = 0;
            }
            else
            {
                *p = 0;
            }
            p++;
            
            sc = sscanf(p, "%hu", &port);
            if(sc != 1)
            {
                globus_free(host);
                globus_gsc_959_finished_command(
                    wrapper->op, "500 Internal Parse error.\r\n");
                goto err;
            }
            
            msg = globus_common_create_string(
                "%d Entering Passive Mode (|%d|%s|%d|)\r\n",
                    wrapper->reply_code,
                    *cs[0] == '[' ? 2 : 1,
                    h,
                    (int) port);
            globus_free(host);
        }
        else
        {
            /* allow SPAS to work until real striping gets done */
            if(addr_count == -1)
            {
                addr_count = 1;
            }
            msg =  globus_common_create_string(
                "%d-Entering Striped Passive Mode.\r\n", wrapper->reply_code);
            for(ctr = 0; ctr < addr_count; ctr++)
            {
                char *                  p;
                char *                  h;
                
                host = globus_libc_strdup(cs[ctr]);
                if(!host)
                {
                    globus_gsc_959_finished_command(
                        wrapper->op, "500 Resource error.\r\n");
                    goto err;
                }
                
                p = strrchr(host, ':');
                if(!p || p == host)
                {
                    globus_free(host);
                    globus_gsc_959_finished_command(
                        wrapper->op, "500 Internal Parse error.\r\n");
                    goto err;
                }
                
                h = host;
                if(*cs[0] == '[')
                {
                    h++;
                    *(p - 1) = 0;
                }
                else
                {
                    *p = 0;
                }
                p++;
            
                sc = sscanf(p, "%hu", &port);
                if(sc != 1)
                {
                    globus_free(host);
                    globus_gsc_959_finished_command(
                        wrapper->op, "500 Internal Parse error.\r\n");
                    goto err;
                }
                
                tmp_ptr = globus_common_create_string(
                    "%s |%d|%s|%d|\r\n",
                    msg,
                    *cs[ctr] == '[' ? 2 : 1,
                    h,
                    (int) port);
                globus_free(host);
                if(tmp_ptr == NULL)
                {
                    globus_gsc_959_finished_command(
                        wrapper->op, "500 Resource error.\r\n");
                    goto err;
                }
                globus_free(msg);
                msg = tmp_ptr;
            }
            tmp_ptr = globus_common_create_string("%s%d End\r\n", 
                msg, wrapper->reply_code);
            if(tmp_ptr == NULL)
            {
                globus_gsc_959_finished_command(
                    wrapper->op, "500 Resource error.\r\n");
                goto err;
            }
            globus_free(msg);
            msg = tmp_ptr;
        }
    }
    else
    {
        globus_assert(GLOBUS_FALSE);
    }

    /* if we were in delayed passive mode we start transfer now */
    if(wrapper->transfer_flag)
    {
        globus_i_gsc_intermediate_reply(op, msg);
        globus_l_gsc_cmd_transfer(wrapper);
        globus_free(msg);
    }
    else
    {
        globus_gsc_959_finished_command(op, msg);
        globus_free(msg);
        globus_free(wrapper);
    }

    return;

  err:

    if(msg != NULL)
    {
        globus_free(msg);
    }
    globus_free(wrapper);
}

/*
 *  passive
 */
static void
globus_l_gsc_cmd_pasv(
    globus_i_gsc_op_t *                     op,
    const char *                            full_command,
    char **                                 cmd_a,
    int                                     argc,
    void *                                  user_arg)
{
    int                                     sc;
    globus_l_gsc_cmd_wrapper_t *            wrapper = NULL;
    char *                                  msg = NULL;
    globus_bool_t                           reply_flag;
    globus_bool_t                           dp;
    globus_result_t                         res;
    GlobusGridFTPServerName(globus_l_gsc_cmd_pasv);

    wrapper = (globus_l_gsc_cmd_wrapper_t *)
        globus_calloc(1, sizeof(globus_l_gsc_cmd_wrapper_t));

    globus_i_gsc_log(op->server_handle, full_command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_TRANSFER_STATE);
    dp = op->server_handle->opts.delayed_passive;
    reply_flag = op->server_handle->opts.delayed_passive;

    if(strncmp(cmd_a[0], "PASV", 4) == 0)
    {
        wrapper->dc_parsing_alg = 0;
        wrapper->max = 1;
        wrapper->prt = GLOBUS_GRIDFTP_SERVER_CONTROL_PROTOCOL_IPV4;
        msg = _FSMSL("227 Passive delayed.\r\n");
        wrapper->cmd_ndx = 1;
        wrapper->reply_code = 227;
    }
    else if(strncmp(cmd_a[0], "EPSV", 4) == 0)
    {
        wrapper->dc_parsing_alg = 1;
        wrapper->prt = GLOBUS_GRIDFTP_SERVER_CONTROL_PROTOCOL_IPV6;
        wrapper->max = 1;
        msg = _FSMSL("229 Passive delayed.\r\n");
        if(argc == 2)
        {
            if(strstr(cmd_a[1], "ALL") != NULL)
            {
                reply_flag = GLOBUS_TRUE;
                op->server_handle->opts.passive_only = GLOBUS_TRUE;
                msg = _FSMSL("229 EPSV ALL Successful.\r\n");
                dp = op->server_handle->opts.delayed_passive;
            }
            else
            {
                sc = sscanf(cmd_a[1], "%d", (int*)&wrapper->prt);
                if(sc != 1)
                {
                    dp = op->server_handle->opts.delayed_passive;
                    reply_flag = GLOBUS_TRUE;
                    msg = _FSMSL("501 Invalid network command.\r\n");
                }
                else if(wrapper->prt !=
                    GLOBUS_GRIDFTP_SERVER_CONTROL_PROTOCOL_IPV4
                    && wrapper->prt !=
                        GLOBUS_GRIDFTP_SERVER_CONTROL_PROTOCOL_IPV6)
                {
                    dp = op->server_handle->opts.delayed_passive;
                    reply_flag = GLOBUS_TRUE;
                    msg = _FSMSL("501 Invalid protocol.\r\n");
                }
            }
        }
        
        wrapper->reply_code = 229;
        wrapper->cmd_ndx = 2;
    }
    else if(strcmp(cmd_a[0], "SPAS") == 0)
    {
        msg = _FSMSL("229 Passive delayed.\r\n");
        wrapper->max = -1;
        wrapper->cmd_ndx = 3;
        wrapper->reply_code = 229;
        wrapper->prt = GLOBUS_GRIDFTP_SERVER_CONTROL_PROTOCOL_IPV4;
        
        /* optional 2nd argument similar to epsv */
        if(argc == 2)
        {
            wrapper->dc_parsing_alg = 1;
            if(*cmd_a[1] == '2')
            {
                wrapper->prt = GLOBUS_GRIDFTP_SERVER_CONTROL_PROTOCOL_IPV6;
            }
        }
        else
        {
            wrapper->dc_parsing_alg = 0;
        }
    }
    else
    {
        globus_assert(GLOBUS_FALSE);
    }

    /*
     *  if delayed just wait for it
     */
    if(!reply_flag)
    {
        res = globus_i_gsc_passive(
            op,
            wrapper->max,
            wrapper->prt,
            NULL,
            globus_l_gsc_cmd_pasv_cb,
            wrapper);
        if(res != GLOBUS_SUCCESS)
        {
            globus_gsc_959_finished_command(op, _FSMSL("500 command failed.\r\n"));
        }
    }
    else
    {
        op->server_handle->opts.delayed_passive = dp;
        globus_gsc_959_finished_command(op, msg);
        globus_free(wrapper);
    }
}

/*
 *  port
 */
static void
globus_l_gsc_cmd_port_cb(
    globus_i_gsc_op_t *                     op,
    globus_gridftp_server_control_response_t response_type,
    char *                                  response_msg,
    void *                                  user_arg)
{
    int                                     code;
    char *                                  msg;
    char *                                  tmp_ptr;

    if(response_type != GLOBUS_GRIDFTP_SERVER_CONTROL_RESPONSE_SUCCESS)
    {
        /* TODO: evaulated error type */
        code = 500;
        msg = strdup(_FSMSL("PORT Command failed."));
    }
    else
    {
        /* if port is successful we know that we are not delaying the pasv */
        op->server_handle->opts.delayed_passive = GLOBUS_FALSE;
        code = 200;
        msg = strdup(_FSMSL("PORT Command successful."));
    }
    if(response_msg != NULL)
    {
        tmp_ptr = msg;
        msg = globus_common_create_string("%s : %s", msg, response_msg);
        free(tmp_ptr);
    }
    tmp_ptr = globus_i_gsc_string_to_959(code, msg, NULL);
    globus_gsc_959_finished_command(op, tmp_ptr);
    globus_free(tmp_ptr);
    globus_free(msg);
}

static void
globus_l_gsc_cmd_port(
    globus_i_gsc_op_t *                     op,
    const char *                            full_command,
    char **                                 cmd_a,
    int                                     argc,
    void *                                  user_arg)
{
    unsigned                                host_ip[4];
    unsigned                                hi;
    unsigned                                low;
    unsigned                                port;
    int                                     sc;
    int                                     i;
    int                                     stripe_count;
    char                                    delim;
    globus_l_gsc_cmd_wrapper_t *            wrapper = NULL;
    char *                                  msg = NULL;
    char *                                  p;
    char **                                 contact_strings = NULL;
    globus_result_t                         res;
    GlobusGridFTPServerName(globus_l_gsc_cmd_port);

    wrapper = (globus_l_gsc_cmd_wrapper_t *) globus_calloc(
        1, sizeof(globus_l_gsc_cmd_wrapper_t));
    if(wrapper == NULL)
    {
        goto err;
    }
    wrapper->op = op;

    globus_i_gsc_log(op->server_handle, full_command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_TRANSFER_STATE);
    if(strcmp(cmd_a[0], "PORT") == 0)
    {
        wrapper->max = 1;
        stripe_count = 1;
    }
    else if(strcmp(cmd_a[0], "SPOR") == 0)
    {
        wrapper->max = -1;
        
        stripe_count = 1; /* for the first */
        p = cmd_a[1];
        while((p = strchr(p, ' ')))
        {
            stripe_count++;
            p++;
        }
    }
    else if(strcmp(cmd_a[0], "EPRT") == 0)
    {
        wrapper->max = 1;
        stripe_count = 1;
    }
    else
    {
        globus_assert(GLOBUS_FALSE);
    }
    
    contact_strings = globus_libc_calloc(stripe_count, sizeof(char *));
    if(contact_strings == NULL)
    {
        goto err;
    }
            
    /* move to the first command argument */
    p = cmd_a[1];
    while(isspace(*p)) p++;
    if(isdigit(delim = *p))
    {
        delim = 0;
    }
    
    for(i = 0; i < stripe_count && *p; i++)
    {
        if(delim)
        {
            /* |prt|ip|port| */
            while(*p && *p != delim) p++;
            if(*p)
            {
                p++;
                if(*p != delim)
                {
                    /* get prt portion */
                    if(sscanf(p, "%d", &sc) < 1 || (sc != 1 && sc != 2))
                    {
                        msg = 
                          "522 Network protocol not supported, use (1,2).\r\n";
                        break;
                    }
                    
                    /* may need to handle mixed prt in striped mode */
                    if(sc == 1)
                    {
                        wrapper->prt =
                            GLOBUS_GRIDFTP_SERVER_CONTROL_PROTOCOL_IPV4;
                    }
                    else
                    {
                        wrapper->prt =
                            GLOBUS_GRIDFTP_SERVER_CONTROL_PROTOCOL_IPV6;
                    }
                    
                    while(*p && *p != delim) p++;
                }
                else
                {
                    msg = "501 Malformed argument.\r\n";
                    break;
                }
            }
            
            if(*p)
            {
                int                     j = 0;
                char *                  s;
                char *                  c;
                char                    buf[100];
                
                p++;
                c = strchr(p, ':');
                s = strchr(p, delim);
                
                if(*p != delim)
                {
                    /* get ip portion */
                    if(c && c < s)
                    {
                        buf[j++] = '[';
                    }
                
                    while(j < sizeof(buf) - 1 && p < s)
                    {
                        buf[j++] = *(p++);
                    }
                    
                    /* need room for ], :, 5 digits, and nul */
                    if(*p == delim && j + 7 < sizeof(buf))
                    {
                        /* get port portion */
                        p++;
                        if(*buf == '[')
                        {
                            buf[j++] = ']';
                        }
                        
                        buf[j++] = ':';
                        while(j < sizeof(buf) - 1 &&
                            isdigit(*p) && *p != delim)
                        {
                            buf[j++] = *(p++);
                        }
                        if(*p == delim)
                        {
                            p++;
                        }
                        else
                        {
                            msg = "501 Malformed argument.\r\n";
                            break;
                        }
                        
                        buf[j] = 0;
                        
                        contact_strings[i] = globus_libc_strdup(buf);
                    }
                    else
                    {
                        msg = "501 Malformed argument.\r\n";
                        break;
                    }
                }
                else
                {
                    msg = "501 Malformed argument.\r\n";
                    break;
                }
            }
        }
        else
        {
            int                         consumed;
            
            wrapper->prt = GLOBUS_GRIDFTP_SERVER_CONTROL_PROTOCOL_IPV4;
            
            while(*p && !isdigit(*p)) p++;
    
            sc = sscanf(p,
                        "%u,%u,%u,%u,%u,%u%n",
                        &host_ip[0],
                        &host_ip[1],
                        &host_ip[2],
                        &host_ip[3],
                        &hi,
                        &low,
                        &consumed);
            port = hi * 256 + low;
            /* if string improperly parsed */
            if(sc < 6)
            {
                msg = _FSMSL("501 Illegal PORT command.\r\n");
                break;
            }
            /* if received port is not valid */
            else if(host_ip[0] > 255 ||
                    host_ip[1] > 255 ||
                    host_ip[2] > 255 ||
                    host_ip[3] > 255 ||
                    port > 65535)
            {
                msg = _FSMSL("501 Illegal PORT command.\r\n");
                break;
            }
            /* all is well with the client string */
            else
            {
                /* create the stripe count string */
                contact_strings[i] = globus_common_create_string(
                    "%u.%u.%u.%u:%d",
                    host_ip[0], host_ip[1], host_ip[2], host_ip[3], port);
                p += consumed;
            }
        }
        
        if(!contact_strings[i])
        {
            msg = _FSMSL("501 Resource error.\r\n");
            break;
        }
    }

    if(i < stripe_count && msg == NULL)
    {
        msg = _FSMSL("501 Illegal PORT command.\r\n");
    }
    if(msg != NULL)
    {
        globus_gsc_959_finished_command(op, msg);
        globus_free(wrapper);
        globus_free(contact_strings);
    }
    else
    {
        wrapper->cs = contact_strings;
        wrapper->cs_count = stripe_count;
        res = globus_i_gsc_port(
                op,
                (const char **)contact_strings,
                stripe_count,
                wrapper->prt,
                globus_l_gsc_cmd_port_cb,
                wrapper);
        if(res != GLOBUS_SUCCESS)
        {
            goto err;
        }
    }

    return;

  err:
    if(contact_strings != NULL)
    {
        globus_free(contact_strings);
    }
    if(wrapper != NULL)
    {
        globus_free(wrapper);
    }
    globus_i_gsc_command_panic(op);
}

/*************************************************************************
 *                          transfer functions
 *                          ------------------
 ************************************************************************/

static void 
globus_l_gsc_data_cb(
    globus_i_gsc_op_t *                     op,
    globus_gridftp_server_control_response_t response_type,
    char *                                  response_msg,
    void *                                  user_arg)
{
    int                                     code;
    char *                                  msg;
    char *                                  tmp_ptr;
    globus_l_gsc_cmd_wrapper_t *            wrapper;

    wrapper = (globus_l_gsc_cmd_wrapper_t *) user_arg;

    if(response_type != GLOBUS_GRIDFTP_SERVER_CONTROL_RESPONSE_SUCCESS)
    {
        /* TODO: evaulated error type */
        code = 500;
        msg = strdup(_FSMSL("Command failed."));
    }
    else
    {
        code = 226;
        msg = strdup(_FSMSL("Transfer Complete."));
    }
    if(response_msg != NULL)
    {
        tmp_ptr = msg;
        msg = globus_common_create_string("%s : %s", msg, response_msg);
        free(tmp_ptr);
    }
    tmp_ptr = globus_i_gsc_string_to_959(code, msg, NULL);
    globus_gsc_959_finished_command(wrapper->op, tmp_ptr);
    globus_free(tmp_ptr);
    globus_free(msg);

    if(wrapper->mod_name)
    {
        globus_free(wrapper->mod_name);
    }
    if(wrapper->mod_parms)
    {
        globus_free(wrapper->mod_parms);
    }
    globus_free(wrapper->path);
    globus_free(wrapper);
}

static void
globus_l_gsc_cmd_transfer(
    globus_l_gsc_cmd_wrapper_t *            wrapper)
{
    globus_result_t                         res;

    switch(wrapper->type)
    {
        case GLOBUS_L_GSC_OP_TYPE_SEND:
            res = globus_i_gsc_send(
                wrapper->op,
                wrapper->path,
                wrapper->mod_name,
                wrapper->mod_parms,
                globus_l_gsc_data_cb,
                wrapper);
            break;

        case GLOBUS_L_GSC_OP_TYPE_RECV:
            res = globus_i_gsc_recv(
                wrapper->op,
                wrapper->path,
                wrapper->mod_name,
                wrapper->mod_parms,
                globus_l_gsc_data_cb,
                wrapper);
            break;

        case GLOBUS_L_GSC_OP_TYPE_NLST:
        case GLOBUS_L_GSC_OP_TYPE_LIST:
        case GLOBUS_L_GSC_OP_TYPE_MLSD:
            res = globus_i_gsc_list(
                wrapper->op,
                wrapper->path,
                GLOBUS_GRIDFTP_SERVER_CONTROL_RESOURCE_USER_DEFINED,
                wrapper->type,
                globus_l_gsc_data_cb,
                wrapper);
            break;
	    
        default:
            globus_assert(GLOBUS_FALSE);
            break;
    }

    if(res != GLOBUS_SUCCESS)
    {
        globus_gsc_959_finished_command(wrapper->op, _FSMSL("500 Command failed\r\n"));
    }
}

/*
 *  stor
 */
static void
globus_l_gsc_cmd_stor_retr(
    globus_i_gsc_op_t *                     op,
    const char *                            full_command,
    char **                                 cmd_a,
    int                                     argc,
    void *                                  user_arg)
{
    int                                     sc;
    globus_result_t                         res;
    char *                                  path = NULL;
    char *                                  mod_name = NULL;
    char *                                  mod_parm = NULL;
    char *                                  tmp_ptr = NULL;
    globus_l_gsc_cmd_wrapper_t *            wrapper = NULL;
    globus_off_t                            tmp_o;
    GlobusGridFTPServerName(globus_l_gsc_cmd_stor);

    wrapper = (globus_l_gsc_cmd_wrapper_t *) globus_malloc(
        sizeof(globus_l_gsc_cmd_wrapper_t));
    if(wrapper == NULL)
    {
        globus_i_gsc_command_panic(op);
        return;
    }
    wrapper->op = op;

    globus_i_gsc_log(op->server_handle, full_command,
        GLOBUS_GRIDFTP_SERVER_CONTROL_LOG_TRANSFER);
    if(strcmp(cmd_a[0], "STOR") == 0 ||  strcmp(cmd_a[0], "ESTO") == 0)
    {
        wrapper->type = GLOBUS_L_GSC_OP_TYPE_RECV;
    }
    else if(strcmp(cmd_a[0], "RETR") == 0 ||  strcmp(cmd_a[0], "ERET") == 0)
    {
        wrapper->type = GLOBUS_L_GSC_OP_TYPE_SEND;
    }
    else if(strcmp(cmd_a[0], "LIST") == 0)
    {
        wrapper->type = GLOBUS_L_GSC_OP_TYPE_LIST;
    }
    else if(strcmp(cmd_a[0], "NLST") == 0)
    {
        wrapper->type = GLOBUS_L_GSC_OP_TYPE_NLST;
    }
    else if(strcmp(cmd_a[0], "MLSD") == 0)
    {
        wrapper->type = GLOBUS_L_GSC_OP_TYPE_MLSD;
    }
    else
    {
        globus_assert(0 && "func shouldn't be called for this command");
    }

    if(strcmp(cmd_a[0], "STOR") == 0 ||
            strcmp(cmd_a[0], "RETR") == 0)
    {
        if(argc != 2)
        {
            globus_free(wrapper);
            globus_gsc_959_finished_command(op, _FSMSL("500 command failed.\r\n"));
            return;
        }
        path = globus_libc_strdup(cmd_a[1]);
        mod_name = NULL;
        mod_parm = NULL;
    }
    else if(strcmp(cmd_a[0], "ESTO") == 0 ||
        strcmp(cmd_a[0], "ERET") == 0)
    {
        if(argc != 3)
        {
            globus_free(wrapper);
            globus_gsc_959_finished_command(op, _FSMSL("500 command failed.\r\n"));
            return;
        }
        if(strcmp(cmd_a[1], "P") == 0 && strcmp(cmd_a[0], "ERET") == 0)
        {
            sc = sscanf(cmd_a[2], 
                "%"GLOBUS_OFF_T_FORMAT" %"GLOBUS_OFF_T_FORMAT, 
                &tmp_o, &tmp_o);
            if(sc != 2)
            {
                globus_free(wrapper);
                globus_gsc_959_finished_command(
                    op, _FSMSL("500 command failed.\r\n"));
                return;
            }
            mod_parm = globus_libc_strdup(cmd_a[2]);
            tmp_ptr = mod_parm;
            while(isdigit(*tmp_ptr)) tmp_ptr++;
            while(isspace(*tmp_ptr)) tmp_ptr++;
            while(isdigit(*tmp_ptr)) tmp_ptr++;
            /* up until here the scanf gauentess safety */
            while(isspace(*tmp_ptr) && *tmp_ptr != '\0') tmp_ptr++;
            if(*tmp_ptr == '\0')
            {
                globus_free(mod_parm);
                globus_free(wrapper);
                globus_gsc_959_finished_command(op, _FSMSL("501 bad parameter.\r\n"));
                return;
            }
            *(tmp_ptr-1) = '\0';

            path = globus_libc_strdup(tmp_ptr);
            mod_name = globus_libc_strdup(cmd_a[1]);
        }
        else if(strcmp(cmd_a[1], "A") == 0 && strcmp(cmd_a[0], "ESTO") == 0)
        {
            sc = sscanf(cmd_a[2], "%"GLOBUS_OFF_T_FORMAT, &tmp_o);
            if(sc != 1)
            {
                globus_free(wrapper);
                globus_gsc_959_finished_command(op, _FSMSL("501 bad parameter.\r\n"));
                return;
            }
            mod_parm = globus_libc_strdup(cmd_a[2]);
            tmp_ptr = mod_parm;
            while(isdigit(*tmp_ptr)) tmp_ptr++;
            /* up until here the scanf gauentess safety */
            while(isspace(*tmp_ptr) && *tmp_ptr != '\0') tmp_ptr++;
            if(*tmp_ptr == '\0')
            {
                globus_free(mod_parm);
                globus_free(wrapper);
                globus_gsc_959_finished_command(op, _FSMSL("501 bad parameter.\r\n"));
                return;
            }
            *(tmp_ptr-1) = '\0';

            path = globus_libc_strdup(tmp_ptr);
            mod_name = globus_libc_strdup(cmd_a[1]);
        }
        else
        {
            mod_name = globus_libc_strdup(cmd_a[1]);
            if(mod_name == NULL)
            {
                globus_free(wrapper);
                globus_i_gsc_command_panic(op);
                return;
            }

            tmp_ptr = strstr(mod_name, "=\"");
            if(tmp_ptr == NULL)
            {
                globus_free(mod_name);
                globus_free(wrapper);
                globus_gsc_959_finished_command(op, _FSMSL("500 command failed.\r\n"));
                return;
            }

            *tmp_ptr = '\0';
            tmp_ptr += 2;
            mod_parm = globus_libc_strdup(tmp_ptr);
            tmp_ptr = strchr(mod_parm, '\"');
            *tmp_ptr = '\0';

            path = globus_libc_strdup(cmd_a[2]);
        }
    }
    /* all list stuff is here */
    else
    {
        if(cmd_a[1] == NULL)
        {
            path = strdup(op->server_handle->cwd);
        }
        else
        {
            path = strdup(cmd_a[1]);
        }
    }

    wrapper->mod_name = mod_name;
    wrapper->mod_parms = mod_parm;
    wrapper->path = path;
    wrapper->reply_code = 129;
    /* if in delayed passive tell library to go passive */
    if(op->server_handle->opts.delayed_passive)
    {
        res = globus_i_gsc_passive(
            wrapper->op,
            wrapper->max,
            wrapper->prt,
            wrapper->path,
            globus_l_gsc_cmd_pasv_cb,
            wrapper);
        if(res != GLOBUS_SUCCESS)
        {
            globus_free(wrapper);
            globus_gsc_959_finished_command(op, _FSMSL("500 command failed.\r\n"));
        }
    }
    else
    {
        globus_l_gsc_cmd_transfer(wrapper);
    }

    return;
}

/*************************************************************************
 *                          helpers
 *                          -------
 ************************************************************************/

void
globus_i_gsc_add_commands(
    globus_i_gsc_server_handle_t *          server_handle)
{
    globus_gsc_959_command_add(
        server_handle,
        NULL,
        globus_l_gsc_cmd_all,
        GLOBUS_GSC_COMMAND_PRE_AUTH |
            GLOBUS_GSC_COMMAND_POST_AUTH,
        1,
        1,
        "ALLO <sp> <size>",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "ALLO", 
        globus_l_gsc_cmd_allo,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        2,
        2,
        "ALLO <sp> <size>",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "CWD", 
        globus_l_gsc_cmd_cwd,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        2,
        2,
        "CWD <sp> pathname",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "CDUP", 
        globus_l_gsc_cmd_cwd,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        1,
        1,
        "CDUP (up one directory)",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "DCAU", 
        globus_l_gsc_cmd_dcau,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        2,
        3,
        "DCAU <S,N,A>",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "EPSV", 
        globus_l_gsc_cmd_pasv,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        1,
        2,
        "EPSV [<sp> ALL]",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "ERET", 
        globus_l_gsc_cmd_stor_retr,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        3,
        3,
        "ERET <sp> mod_name=\"mod_parms\" <sp> pathname",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "ESTO", 
        globus_l_gsc_cmd_stor_retr,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        3,
        3,
        "ESTO <sp> mod_name=\"mod_parms\" <sp> pathname",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "FEAT", 
        globus_l_gsc_cmd_feat,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        1,
        1,
        "FEAT",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "HELP", 
        globus_l_gsc_cmd_help,
        GLOBUS_GSC_COMMAND_PRE_AUTH | 
            GLOBUS_GSC_COMMAND_POST_AUTH,
        1,
        2,
        "HELP [<sp> command]",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "LIST", 
        globus_l_gsc_cmd_stor_retr,
        GLOBUS_GSC_COMMAND_PRE_AUTH | 
            GLOBUS_GSC_COMMAND_POST_AUTH,
        1,
        2,
        "LIST [<sp> <filename>]",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "MDTM", 
        globus_l_gsc_cmd_mdtm,
        GLOBUS_GSC_COMMAND_PRE_AUTH | 
            GLOBUS_GSC_COMMAND_POST_AUTH,
        2,
        2,
        "MDTM <sp> <filename>",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "MODE", 
        globus_l_gsc_cmd_mode,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        2,
        2,
        "MODE <sp> mode-code",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "NLST", 
        globus_l_gsc_cmd_stor_retr,
        GLOBUS_GSC_COMMAND_PRE_AUTH | 
            GLOBUS_GSC_COMMAND_POST_AUTH,
        1,
        2,
        "NLST [<sp> <filename>]",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "MLSD",
        globus_l_gsc_cmd_stor_retr,
        GLOBUS_GSC_COMMAND_PRE_AUTH | 
            GLOBUS_GSC_COMMAND_POST_AUTH,
        1,
        2,
        "MLSD [<sp> <filename>]",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "MLST",
        globus_l_gsc_cmd_stat,
        GLOBUS_GSC_COMMAND_PRE_AUTH | 
            GLOBUS_GSC_COMMAND_POST_AUTH,
        1,
        2,
        "MLST [<sp> <filename>]",
        (void *)1);

    globus_gsc_959_command_add(
        server_handle,
        "NOOP", 
        globus_l_gsc_cmd_noop,
        GLOBUS_GSC_COMMAND_PRE_AUTH | 
            GLOBUS_GSC_COMMAND_POST_AUTH,
        1,
        1,
        "NOOP (no operation)",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "OPTS", 
        globus_l_gsc_cmd_opts,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        2,
        3,
        "OPTS <sp> opt-type [paramters]",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "PASS", 
        globus_l_gsc_cmd_pass,
        GLOBUS_GSC_COMMAND_PRE_AUTH,
        1,
        2,
        "PASS <sp> password",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "PASV", 
        globus_l_gsc_cmd_pasv,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        1,
        1,
        "PASV",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "PBSZ", 
        globus_l_gsc_cmd_pbsz,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        2,
        2,
        "PBSZ <sp> size",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "PORT", 
        globus_l_gsc_cmd_port,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        2,
        2,
        "PORT <port>",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "PROT", 
        globus_l_gsc_cmd_prot,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        2,
        2,
        "PROT <C|P|S>",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "EPRT", 
        globus_l_gsc_cmd_port,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        2,
        2,
        "EPRT <sp> <port>",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "SPOR", 
        globus_l_gsc_cmd_port,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        2,
        2,
        "SPOR <sp> <port list>",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "TREV", 
        globus_l_gsc_cmd_trev,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        2,
        2,
        "TREV <event name> <frequency> [info list]",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "PWD", 
        globus_l_gsc_cmd_pwd,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        1,
        1,
        "PWD (returns current working directory)",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "QUIT", 
        globus_l_gsc_cmd_quit,
        GLOBUS_GSC_COMMAND_PRE_AUTH | 
            GLOBUS_GSC_COMMAND_POST_AUTH,
        1,
        1,
        "QUIT (close control connection)",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "REST", 
        globus_l_gsc_cmd_rest,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        2,
        2,
        "REST [<sp> restart marker]",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "RETR", 
        globus_l_gsc_cmd_stor_retr,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        2,
        2,
        "RETR [<sp> pathname]",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "SBUF", 
        globus_l_gsc_cmd_sbuf,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        2,
        2,
        "SBUF <sp> window-size",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "SIZE", 
        globus_l_gsc_cmd_size,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        2,
        2,
        "SIZE <sp> pathname",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "SPAS", 
        globus_l_gsc_cmd_pasv,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        1,
        2,
        "SPAS",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "STAT", 
        globus_l_gsc_cmd_stat,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        1,
        2,
        "STAT [<sp> pathname]",
        0);

    globus_gsc_959_command_add(
        server_handle,
        "STOR", 
        globus_l_gsc_cmd_stor_retr,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        2,
        2,
        "STOR [<sp> pathname]",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "STRU", 
        globus_l_gsc_cmd_stru,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        2,
        2,
        "STRU (specify file structure)",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "SYST", 
        globus_l_gsc_cmd_syst,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        1,
        1,
        "SYST (returns system type)",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "TYPE", 
        globus_l_gsc_cmd_type,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        2,
        2,
        "TYPE <sp> type-code",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "USER", 
        globus_l_gsc_cmd_user,
        GLOBUS_GSC_COMMAND_PRE_AUTH,
        2,
        2,
        "USER <sp> username",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "SITE SBUF", 
        globus_l_gsc_cmd_site_sbuf,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        3,
        3,
        "SITE SBUF: set send and receive buffers",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "SITE RETRBUFSIZE", 
        globus_l_gsc_cmd_site_receive_buf,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        3,
        3,
        "SITE RETRBUFSIZE: set receive buffers",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "SITE RBUFSZ", 
        globus_l_gsc_cmd_site_receive_buf,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        3,
        3,
        "SITE RBUFSZ: set receive buffers",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "SITE RBUFSIZ", 
        globus_l_gsc_cmd_site_receive_buf,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        3,
        3,
        "SITE RBUFSIZ: set receive buffers",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "SITE STORBUFSIZE", 
        globus_l_gsc_cmd_site_send_buf,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        3,
        3,
        "SITE STORBUFSIZE: set send buffers",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "SITE SBUFSZ", 
        globus_l_gsc_cmd_site_send_buf,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        3,
        3,
        "SITE SBUFSZ: set send buffers",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "SITE SBUFSIZ", 
        globus_l_gsc_cmd_site_send_buf,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        3,
        3,
        "SITE SBUFSIZ: set send buffers",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "SITE HELP", 
        globus_l_gsc_cmd_help,
        GLOBUS_GSC_COMMAND_PRE_AUTH | 
            GLOBUS_GSC_COMMAND_POST_AUTH,
        2,
        3,
        "SITE HELP: help on server commands",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "SITE FAULT", 
        globus_l_gsc_cmd_site_fault,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        3,
        3,
        "SITE FAULT: force a failure on given command",
        NULL);

    globus_gsc_959_command_add(
        server_handle,
        "LANG", 
        globus_l_gsc_cmd_lang,
        GLOBUS_GSC_COMMAND_POST_AUTH,
        1,
        2,
        "LANG: set language for messages",
        NULL);
    
    /* add features */
    globus_gridftp_server_control_add_feature(server_handle, "MDTM");
    globus_gridftp_server_control_add_feature(server_handle, "REST STREAM");
    globus_gridftp_server_control_add_feature(server_handle, "SPOR");
    globus_gridftp_server_control_add_feature(server_handle, "SPAS");
    globus_gridftp_server_control_add_feature(server_handle, "ESTO");
    globus_gridftp_server_control_add_feature(server_handle, "ERET");
    globus_gridftp_server_control_add_feature(server_handle, "MLST Type*;Size*;Modify*;Perm*;Charset;UNIX.mode*;Unique*;UNIX.slink*;");    
    globus_gridftp_server_control_add_feature(server_handle, "SIZE");    
    globus_gridftp_server_control_add_feature(server_handle, "PARALLEL");    
    globus_gridftp_server_control_add_feature(server_handle, "DCAU");    
    globus_gridftp_server_control_add_feature(server_handle, "LANG EN");    
    globus_gridftp_server_control_add_feature(server_handle, "UTF8");
}
