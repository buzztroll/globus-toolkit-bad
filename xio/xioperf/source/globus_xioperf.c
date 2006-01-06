#include "globus_i_xioperf.h"
#include "globus_options.h"

extern globus_options_entry_t            globus_i_xioperf_opts_table[];

static
globus_result_t
xioperf_next_write(
    globus_i_xioperf_info_t *           info);

static
globus_result_t
xioperf_start(
    globus_i_xioperf_info_t *           info);

static
void
xioperf_l_log(
    const char *                        msg,
    globus_result_t                     res)
{
    fprintf(stderr, "%s: %s\n", msg, globus_object_printable_to_string(
        globus_error_get(res)));
}

static
char *
xioperf_outformat_bw(
    char                                type,
    double                              time,
    globus_off_t                        bytes)
{
    char * str;
    double val;

    if(type == 'm' || type == 'k' || type == 'b' || type == 'g')
    {
        bytes *= 8;
    }
    val = (double) bytes;
    switch(type)
    {
        case 'G':
        case 'g':
            val /= 1024;
        case 'M':
        case 'm':
            val /= 1024;
        case 'K':
        case 'k':
            val /= 1024;

        default:
            break;
    }
    val /= time;

    str = globus_common_create_string("%-10.2lf       ", val);
    sprintf(strchr(str, ' '), " %c/s", type);
    return str;
}

static
char *
xioperf_outformat_bytes(
    char                                type,
    globus_off_t                        bytes)
{
    char * str;
    double val;

    type = toupper(type);
    val = (double) bytes;
    switch(type)
    {
        case 'G':
            val /= 1024;
        case 'M':
            val /= 1024;
        case 'K':
            val /= 1024;

        default:
            break;
    }

    str = globus_common_create_string("%-10.2lf       ", val);
    sprintf(strchr(str, ' '), " %c", type);
    return str;
}

static
void
xioperf_l_print_summary(
    globus_i_xioperf_info_t *           info)
{
    double                              secs;
    int                                 mins;
    globus_reltime_t                    elps_time;
    long                                usecs;

    GlobusTimeAbstimeGetCurrent(info->end_time);
    GlobusTimeAbstimeDiff(elps_time, info->end_time, info->start_time);
    GlobusTimeReltimeToUSec(usecs, elps_time);

    secs = usecs / 1000000.0;
    mins = (int)secs/60;
    printf("\tTime:         %02d:%02.4f\n", mins, secs-(mins*60));

    if(info->writer)
    {
        printf("\tBytes sent:   %s\n",
            xioperf_outformat_bytes(info->format, info->bytes_sent));
        printf("\tWrite BW:     %s\n",
            xioperf_outformat_bw(info->format, secs, info->bytes_sent));
    }
    if(info->reader)
    {
        printf("\tBytes recv:   %s\n",
            xioperf_outformat_bytes(info->format, info->bytes_recv));
        printf("\tRead BW:      %s\n",
            xioperf_outformat_bw(info->format, secs, info->bytes_recv));
    }
}

static
globus_result_t
xioperf_l_opts_unknown(
    const char *                        parm,
    void *                              arg)
{
    return globus_error_put(globus_error_construct_error(
        NULL,
        NULL,
        2,
        __FILE__,
        "xioperf_l_opts_unknown",
        __LINE__,
        "Unknown parameter: %s",
        parm));
}

static
globus_i_xioperf_info_t *
xioperf_l_parse_opts(
    int                                 argc,
    char **                             argv)
{
    globus_size_t                       nbytes;
    globus_result_t                     res;
    globus_options_handle_t             opt_h;
    globus_i_xioperf_info_t *           info;
    GlobusXIOPerfFuncName(xioperf_l_parse_opts);

    info = (globus_i_xioperf_info_t *) globus_calloc(
        1, sizeof(globus_i_xioperf_info_t));
    if(info == NULL)
    {
        goto error;
    }

    globus_mutex_init(&info->mutex, NULL);
    globus_cond_init(&info->cond, NULL);
    info->server = GLOBUS_TRUE;
    info->stream_count = 1;
    info->len = 8*1024;
    info->block_size = 64*1024;
    info->format = 'm';
    globus_hashtable_init(&info->driver_table, 8, 
        globus_hashtable_string_hash,
        globus_hashtable_string_keyeq);
    GlobusTimeReltimeSet(info->time, 10, 0);
    globus_xio_stack_init(&info->stack, NULL);

    globus_options_init(
        &opt_h, xioperf_l_opts_unknown, info, globus_i_xioperf_opts_table);
    res = globus_options_command_line_process(opt_h, argc, argv);
    if(res != GLOBUS_SUCCESS)
    {
        goto error_result;
    }

    if(info->interval > 0 &&
        (info->file || info->bytes_to_transfer)) 
    {
        fprintf(stderr, "ignoring interval parameter\n");
    }
    if(info->daemon && !info->server)
    {
        res = GlobusXIOPerfError(
                "only a server can be in daemon mode",
                GLOBUS_XIO_PERF_ERROR_PARM);
        goto error_result;
    }

    info->next_write_buffer = (globus_byte_t *)globus_malloc(info->block_size);
    if(!info->reader && !info->writer)
    {
        if(info->server)
        {
            info->reader = GLOBUS_TRUE;
        }
        else
        {
            info->writer = GLOBUS_TRUE;
        }
    }
    if(!info->reader)
    {
        info->read_done = GLOBUS_TRUE;
    }
    if(!info->writer)
    {
        info->write_done = GLOBUS_TRUE;
        if(info->bytes_to_transfer > 0)
        {
            fprintf(stderr, "ignoring --num, only relvent when sending\n");
        }
        info->bytes_to_transfer = 0;
    }

    if(info->reader && info->writer && info->file)
    {
        res = GlobusXIOPerfError(
                "cannot read and write if using a file",
                GLOBUS_XIO_PERF_ERROR_PARM);
        goto error_result;
    }
    if(info->file)
    {
        if(info->reader)
        {
            info->fptr = fopen(info->file, "w");
            if(info->fptr == NULL)
            {
                res = GlobusXIOPerfError(
                    "could not open the specified file for writing",
                    GLOBUS_XIO_PERF_ERROR_PARM);
                goto error_result;
            }
        }
        else if(info->writer)
        {
            info->fptr = fopen(info->file, "r");
            if(info->fptr == NULL)
            {
                res = GlobusXIOPerfError(
                    "could not open the specified file for writing",
                    GLOBUS_XIO_PERF_ERROR_PARM);
                goto error_result;
            }
            nbytes = 
               fread(info->next_write_buffer, 1, info->block_size, info->fptr);
            if(nbytes < info->block_size)
            {
                info->block_size = nbytes;
                info->eof = GLOBUS_TRUE;
            }
        }
    }

    return info;
error_result:
    fprintf(stderr, "%s\n",
        globus_error_print_friendly(globus_error_get(res)));
error:
    return NULL;
}

static
void
xioperf_interval(
    void *                              user_arg)
{
    globus_i_xioperf_info_t *           info;

    info = (globus_i_xioperf_info_t *) user_arg;

    globus_mutex_lock(&info->mutex);
    {
        xioperf_l_print_summary(info);
        fprintf(stdout, 
        "---------------------------------------------------------------\n");
        GlobusTimeAbstimeGetCurrent(info->start_time);
        info->bytes_sent = 0;
        info->bytes_recv = 0;
    }
    globus_mutex_unlock(&info->mutex);
}

static
void
xioperf_timeout(
    void *                              user_arg)
{
    globus_i_xioperf_info_t *           info;

    info = (globus_i_xioperf_info_t *) user_arg;

    globus_mutex_lock(&info->mutex);
    {
        printf("Time exceeded.  Terminating.\n");
        info->read_done = GLOBUS_TRUE;
        info->write_done = GLOBUS_TRUE;
        globus_xio_handle_cancel_operations(
            info->xio_handle, GLOBUS_XIO_CANCEL_WRITE);
    }
    globus_mutex_unlock(&info->mutex);
}

static
void
xioperf_read_cb(
    globus_xio_handle_t                 handle,
    globus_result_t                     result,
    globus_byte_t *                     buffer,
    globus_size_t                       len,
    globus_size_t                       nbytes,
    globus_xio_data_descriptor_t        data_desc,
    void *                              user_arg)
{
    globus_i_xioperf_info_t *           info;

    info = (globus_i_xioperf_info_t *) user_arg;

    globus_mutex_lock(&info->mutex);
    {
        info->bytes_recv += nbytes;

        if(info->fptr != NULL && nbytes > 0)
        {
            /* seek when needed */
            fwrite(buffer, 1, nbytes, info->fptr);
        }
        info->ref--;
        if(result != GLOBUS_SUCCESS)
        {
            info->err = globus_error_get(result);
            goto error;
        }
        if(info->read_done)
        {
            /* happens with ctl+c */
            goto error;
        }

        /* if we are going by a count only register a new read if
            we have not gotten all we want.  need this for bi-directional */
        if(info->bytes_to_transfer == 0 ||
            info->bytes_recv < info->bytes_to_transfer ||
            !info->writer)
        {
            result = globus_xio_register_read(
                info->xio_handle,
                buffer,
                info->block_size,
                1,
                NULL,
                xioperf_read_cb,
                info);
            if(result != GLOBUS_SUCCESS)
            {
                goto error;
            }
            info->ref++;
        }
        else
        {
            info->read_done = GLOBUS_TRUE;
            globus_cond_signal(&info->cond);
        }
    }
    globus_mutex_unlock(&info->mutex);

    return;
error:
    globus_free(buffer);
    info->read_done = GLOBUS_TRUE;
    globus_cond_signal(&info->cond);
    globus_mutex_unlock(&info->mutex);
}

static
void
xioperf_write_cb(
    globus_xio_handle_t                 handle,
    globus_result_t                     result,
    globus_byte_t *                     buffer,
    globus_size_t                       len,
    globus_size_t                       nbytes,
    globus_xio_data_descriptor_t        data_desc,
    void *                              user_arg)
{
    globus_i_xioperf_info_t *           info;

    info = (globus_i_xioperf_info_t *) user_arg;

    globus_mutex_lock(&info->mutex);
    {
        globus_free(buffer);
        info->bytes_sent += nbytes;
        info->ref--;
        if(result != GLOBUS_SUCCESS)
        {
            goto error;
        }
        if(info->bytes_to_transfer > 0 &&
            info->bytes_sent >= info->bytes_to_transfer)
        {
            info->write_done = GLOBUS_TRUE;
        }
        if(!info->write_done)
        {
            result = xioperf_next_write(info);
            if(result != GLOBUS_SUCCESS)
            {
                goto error;
            }
        }
        globus_cond_signal(&info->cond);
    }
    globus_mutex_unlock(&info->mutex);

    return;
error:

    if(!globus_xio_error_is_canceled(result))
    {
        xioperf_l_log("write_cb error: ", result);
    }
    info->err = globus_error_get(result);
    info->write_done = GLOBUS_TRUE;
    globus_cond_signal(&info->cond);
    globus_mutex_unlock(&info->mutex);
}


static
globus_result_t
xioperf_next_write(
    globus_i_xioperf_info_t *           info)
{
    size_t                              nbytes;
    globus_result_t                     res;

    globus_assert(!info->write_done);
    res = globus_xio_register_write(
        info->xio_handle,
        info->next_write_buffer,
        info->block_size,
        info->block_size,
        NULL,
        xioperf_write_cb,
        info);
    if(res != GLOBUS_SUCCESS)
    {
        goto error;
    }
    info->ref++;

    info->next_write_buffer = (globus_byte_t*)globus_malloc(info->block_size);
    if(info->eof)
    {
        info->write_done = GLOBUS_TRUE;
    }
    else if(info->fptr != NULL)
    {
        nbytes = fread(
            info->next_write_buffer, 1, info->block_size, info->fptr);
        if(nbytes != info->block_size)
        {
            info->eof = GLOBUS_TRUE;
        }
    }
    return GLOBUS_SUCCESS;
error:
    return res;
}

static
void
xioperf_l_interrupt_cb(
    void *                              user_arg)
{
    globus_i_xioperf_info_t *           info;

    info = (globus_i_xioperf_info_t *) user_arg;

    globus_mutex_lock(&info->mutex);
    {
        printf("Dieing...\n");
        if(info->die)
        {
            /* if they hit it twice */
            exit(1);
        }
        info->die = GLOBUS_TRUE;
        info->read_done = GLOBUS_TRUE;
        info->write_done = GLOBUS_TRUE;
        globus_xio_handle_cancel_operations(
            info->xio_handle, GLOBUS_XIO_CANCEL_WRITE | GLOBUS_XIO_CANCEL_READ);
        globus_cond_signal(&info->cond);
    }
    globus_mutex_unlock(&info->mutex);
}

int
main(
    int                                 argc,
    char **                             argv)
{
    char *                              cs;
    globus_i_xioperf_info_t *           info;
    globus_i_xioperf_info_t             info_copy;
    globus_result_t                     res;
    globus_xio_driver_t                 driver;
 
    globus_module_activate(GLOBUS_XIO_MODULE);

    info = xioperf_l_parse_opts(argc, argv);
    if(info == NULL)
    {
        goto error;
    }
    globus_callback_register_signal_handler(
        GLOBUS_SIGNAL_INTERRUPT,
        GLOBUS_TRUE,
        xioperf_l_interrupt_cb,
        info);
    globus_xio_attr_init(&info->attr);
    /* tcp specific */
    driver = (globus_xio_driver_t) globus_hashtable_lookup(
        &info->driver_table, (void *)"tcp");
    if(driver != NULL)
    {
        if(info->window > 0)
        {
            int                         w = (int)info->window;
            res = globus_xio_attr_cntl(
                info->attr, driver, GLOBUS_XIO_TCP_SET_SNDBUF, w);
            res = globus_xio_attr_cntl(
                info->attr, driver, GLOBUS_XIO_TCP_SET_RCVBUF, w);
        }
        globus_xio_attr_cntl(
            info->attr, driver, GLOBUS_XIO_TCP_SET_NODELAY, info->nodelay);
        if(info->bind_addr != NULL)
        {
            globus_xio_attr_cntl(
                info->attr,
                driver, GLOBUS_XIO_TCP_SET_INTERFACE, info->bind_addr);
        }
        if(info->port != 0)
        {
            globus_xio_attr_cntl(
                info->attr, driver, GLOBUS_XIO_TCP_SET_PORT, info->port);
        }
    }



    fprintf(stdout, 
    "---------------------------------------------------------------\n");
    /* driver specif stuff will be tricky */
    if(info->server)
    {
        res = globus_xio_server_create(
            &info->server_handle, info->attr, info->stack);
        if(res != GLOBUS_SUCCESS)
        {
            xioperf_l_log("setup error:",res);
            goto error;
        }
        globus_xio_server_get_contact_string(info->server_handle, &cs);
        fprintf(stdout, "server listening on: %s\n", cs);
        fprintf(stdout, 
        "---------------------------------------------------------------\n");
        globus_free(cs);

        do
        {
            res = globus_xio_server_accept(
                &info->xio_handle, info->server_handle);
            if(res != GLOBUS_SUCCESS)
            {
                xioperf_l_log("accept error:", res);
            }
            else
            {
                /* copy initial values */
                memcpy(&info_copy, info, sizeof(globus_i_xioperf_info_t));
                res = xioperf_start(&info_copy);
                if(res != GLOBUS_SUCCESS)
                {
                    xioperf_l_log("connection error:",res);
                }
            }
        } while(info->daemon && !info->die);
    }
    else
    {
        res = globus_xio_handle_create(&info->xio_handle, info->stack);
        if(res != GLOBUS_SUCCESS)
        {
            xioperf_l_log("setup error:",res);
            goto error;
        }
        res = xioperf_start(info);
        if(res != GLOBUS_SUCCESS)
        {
            goto error;
        }
    }
    globus_module_deactivate(GLOBUS_XIO_MODULE);
    return 0;

error:
    fprintf(stdout, "failed.\n");
    globus_module_deactivate(GLOBUS_XIO_MODULE);
    return 1;
}

static
globus_result_t
xioperf_post_io(
    globus_i_xioperf_info_t *           info)
{
    int                                 i;
    globus_result_t                     res;
    globus_byte_t *                     buffer;

    for(i = 0; i < info->stream_count; i++)
    {
        if(info->reader && !info->read_done)
        {
            buffer = (globus_byte_t*)globus_malloc(info->block_size);
            res = globus_xio_register_read(
                info->xio_handle,
                buffer,
                info->block_size,
                1,
                NULL,
                xioperf_read_cb,
                info);
            if(res != GLOBUS_SUCCESS)
            {
                info->read_done = GLOBUS_TRUE;
                xioperf_l_log("initial read error:", res);
                goto error;
            }
            else
            {
                info->ref++;
            }
        }
        if(info->writer && !info->write_done)
        {
            res = xioperf_next_write(info);
            if(res != GLOBUS_SUCCESS)
            {
                info->write_done = GLOBUS_TRUE;
                xioperf_l_log("initial write error:", res);
                goto error;
            }
        }
    }
    return GLOBUS_SUCCESS;
error:
    return res;
}


static
globus_result_t
xioperf_start(
    globus_i_xioperf_info_t *           info)
{
    globus_result_t                     res;
    globus_reltime_t                    period; 
    globus_xio_attr_t                   close_attr;

    /* do driver specific stuff */
    globus_xio_attr_init(&close_attr);
    globus_xio_attr_cntl(
        close_attr, NULL, GLOBUS_XIO_ATTR_CLOSE_NO_CANCEL, GLOBUS_TRUE);
    /* tcp specific */

    globus_mutex_lock(&info->mutex);
    {
        /* if we are doing time */
        if(info->interval > 0)
        {
            GlobusTimeReltimeSet(period, info->interval, 0);
            globus_callback_register_periodic(
                NULL,
                &period,
                &period,
                xioperf_interval,
                info);
        }
        else if(info->bytes_to_transfer == 0 
            && info->writer && info->fptr == NULL)
        {
            globus_callback_register_oneshot(
                NULL,
                &info->time,
                xioperf_timeout,
                info);
        }

        GlobusTimeAbstimeGetCurrent(info->start_time);
        res = globus_xio_open(info->xio_handle, info->client, info->attr);
        if(res != GLOBUS_SUCCESS)
        {
            goto error;
        }
        printf("Connection esstablished\n");
        fprintf(stdout, 
        "---------------------------------------------------------------\n");
        res = xioperf_post_io(info);

        while(!info->read_done || info->ref > 0 || !info->write_done)
        {
            globus_cond_wait(&info->cond, &info->mutex);
        }
        if(!info->die && info->dual)
        {
            if(info->server)
            {
                info->write_done = GLOBUS_FALSE;
                info->writer = GLOBUS_TRUE;
                info->reader = GLOBUS_FALSE;
            }
            else
            {
                info->read_done = GLOBUS_FALSE;
                info->reader = GLOBUS_TRUE;
                info->writer = GLOBUS_FALSE;
            }
            res = xioperf_post_io(info);
            while(!info->read_done || info->ref > 0 || !info->write_done)
            {
                globus_cond_wait(&info->cond, &info->mutex);
            }
        }
        res = globus_xio_close(info->xio_handle, close_attr);
        if(res != GLOBUS_SUCCESS)
        {
            xioperf_l_log("close error", res);
        }
        xioperf_l_print_summary(info);
    }
    globus_mutex_unlock(&info->mutex);

    if(info->fptr != NULL)
    {
        fclose(info->fptr);
    }
    if(res != GLOBUS_SUCCESS)
    {
        goto error;
    }

    return GLOBUS_SUCCESS;
error:
    return res;
}
