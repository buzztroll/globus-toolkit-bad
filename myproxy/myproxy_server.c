/*
 * myproxy-server
 *
 * program to store user's delegated credentials for use in a portal
 */

#include "myproxy.h"
#include "myproxy_server.h"
#include "myproxy_creds.h"
#include "myproxy_log.h"
#include "gnu_getopt.h"
#include "version.h"
#include "verror.h"
#include "string_funcs.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/wait.h> 
#include <netinet/in.h>	/* Might be needed before <arpa/inet.h> */
#include <arpa/inet.h> 
#include <unistd.h>
#include <netdb.h> 
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <time.h>

#define MIN(x,y) ((x) < (y) ? (x) : (y))

static char usage[] = \
"\n"\
"Syntax: myproxy-server [-d|-debug] [-p|-port #] [-c config-file] ...\n"\
"        myproxy-server [-h|-help] [-v|-version]\n"\
"\n"\
"   Options\n"\
"       -h | --help                Displays usage\n"\
"       -u | --usage                             \n"\
"                                               \n"\
"       -v | --version             Displays version\n"\
"       -d | --debug               Turns on debugging\n"\
"       -c | --config              Specifies configuration file to use\n"\
"       -p | --port <portnumber>   Specifies the port to run on\n"\
"       -s | --storage <directory> Specifies the credential storage directory\n"\
"\n";

struct option long_options[] =
{
    {"debug",            no_argument, NULL, 'd'},
    {"help",             no_argument, NULL, 'h'},
    {"port",       required_argument, NULL, 'p'},
    {"config",     required_argument, NULL, 'c'},       
    {"usage",            no_argument, NULL, 'u'},
    {"version",          no_argument, NULL, 'v'},
    {0, 0, 0, 0}
};

static char short_options[] = "dhc:p:vu";

static char version[] =
"myproxy-server version " MYPROXY_VERSION " (" MYPROXY_VERSION_DATE ") "  "\n";

static char default_config_file[] =
MYPROXY_SERVER_DIR "/etc/myproxy-server.config";

/* Signal handling */
typedef void Sigfunc(int);  

Sigfunc *my_signal(int signo, Sigfunc *func);
void sig_exit(int signo);
void sig_chld(int signo);
void sig_ign(int signo);

/* Function declarations */
int init_arguments(int argc, 
                   char *argv[], 
                   myproxy_socket_attrs_t *server_attrs, 
                   myproxy_server_context_t *server_context);

int myproxy_init_server(myproxy_socket_attrs_t *server_attrs);

int handle_client(myproxy_socket_attrs_t *server_attrs, 
                  myproxy_server_context_t *server_context);

void respond_with_error_and_die(myproxy_socket_attrs_t *attrs,
				const char *error);

void send_response(myproxy_socket_attrs_t *server_attrs, 
		   myproxy_response_t *response, 
		   char *client_name);

void get_proxy(myproxy_socket_attrs_t *server_attrs, 
	       myproxy_creds_t *creds,
	       myproxy_request_t *request,
	       myproxy_response_t *response);

void put_proxy(myproxy_socket_attrs_t *server_attrs, 
	      myproxy_creds_t *creds, 
	      myproxy_response_t *response);

void info_proxy(myproxy_creds_t *creds, myproxy_response_t *response);

void destroy_proxy(myproxy_creds_t *creds, myproxy_response_t *response);

static void failure(const char *failure_message); 

static void my_failure(const char *failure_message);

static char *timestamp(void);

static int become_daemon(myproxy_server_context_t *server_context);

static int debug = 0;

int
main(int argc, char *argv[]) 
{    
    int   listenfd;
    pid_t childpid;
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    myproxy_socket_attrs_t         *socket_attrs;
    myproxy_server_context_t       *server_context;
  
    socket_attrs    = malloc(sizeof(*socket_attrs));
    memset(socket_attrs, 0, sizeof(*socket_attrs));

    server_context  = malloc(sizeof(*server_context));
    memset(server_context, 0, sizeof(*server_context));

    /* Set context defaults */
    server_context->run_as_daemon = 1;
    
    if (init_arguments(argc, argv, socket_attrs, server_context) < 0) {
        fprintf(stderr, usage);
        exit(1);
    }

    if (server_context->config_file == NULL)
    {
        server_context->config_file = strdup(default_config_file);

	if (server_context->config_file == NULL)
	{
	    perror("strdup()");
	    exit(1);
	}
    }

    /* Read my configuration */
    if (myproxy_server_config_read(server_context) == -1)
    {
	fprintf(stderr, "%s %s\n", verror_get_string(), verror_strerror());
	exit(1);
    }
    
    /* 
     * Test to see if we're run out of inetd 
     * If so, then stdin will be connected to a socket,
     * so getpeername() will succeed.
     */
    if (getpeername(fileno(stdin), (struct sockaddr *) &client_addr, &client_addr_len) < 0) 
    {
       server_context->run_as_daemon = 1;
       if (!debug) 
       {
	  if (become_daemon(server_context) < 0) 
	  {
	     fprintf(stderr, "Error starting daemon\n");
	     exit(1);
	  }
       }
    } 
    else 
    { 
       server_context->run_as_daemon = 0;
       close(1);
       (void) open("/dev/null",O_WRONLY);
    }
    /* Initialize Logging */
    if (debug) {
	myproxy_debug_set_level(1);
        myproxy_log_use_stream(stderr);
    }
    else
    {
	myproxy_log_use_syslog(LOG_DAEMON, server_context->my_name);
    }

    /*
     * Logging initialized: For here on use myproxy_log functions
     * instead of fprintf() and ilk.
     */
    myproxy_log("starting at %s", timestamp());

    /* Set up signal handling to deal with zombie processes left over  */
    my_signal(SIGCHLD, sig_chld);
    
    /* If process is killed or Ctrl-C */
    my_signal(SIGTERM, sig_exit); 
    my_signal(SIGINT,  sig_exit); 
    
    if (!server_context->run_as_daemon) 
    {
       myproxy_log("Connection from %s", inet_ntoa(client_addr.sin_addr));
       socket_attrs->socket_fd = fileno(stdin);
       if (handle_client(socket_attrs, server_context) < 0) {
	  my_failure("error in handle_client()");
       } 
       exit(0);
    }
    else
    {    
       /* Run as a daemon */
       listenfd = myproxy_init_server(socket_attrs);
       /* Set up concurrent server */
       while (1) {
	  socket_attrs->socket_fd = accept(listenfd,
					   (struct sockaddr *) &client_addr,
					   &client_addr_len);
	  myproxy_log("Connection from %s", inet_ntoa(client_addr.sin_addr));
	  if (socket_attrs->socket_fd < 0) {
	     if (errno == EINTR) {
		continue; 
	     } else {
		myproxy_log_perror("Error in accept()");
	     }
	  }
	  if (!debug)
	  {
	     childpid = fork();
	     
	     if (childpid < 0) {              /* check for error */
		myproxy_log_perror("Error in fork");
		close(socket_attrs->socket_fd);
	  }
	     else if (childpid != 0)
	     {
		/* Parent */
		/* parent closes connected socket */
		close(socket_attrs->socket_fd);	     
	     continue;	/* while(1) */
	     }
	     
	     /* child process */
	     close(0);
	     close(1);
	     if (!debug) {
		close(2);
	     }
	     close(listenfd);
	  }
	  if (handle_client(socket_attrs, server_context) < 0) {
	     my_failure("error in handle_client()");
	  } 
	  _exit(0);
       }
       exit(0);
    }
}   

int
handle_client(myproxy_socket_attrs_t *attrs, myproxy_server_context_t *context) 
{
    char  error_string[1024];
    char  client_name[1024];
    char  client_buffer[1024];
    int   requestlen;
    int   authorization_ok = 0;
    int   credentials_exist = 0;
    int   client_owns_credentials = 0;
    
    myproxy_creds_t *client_creds;          
    myproxy_request_t *client_request;
    myproxy_response_t *server_response;

    client_creds    = malloc(sizeof(*client_creds));
    memset(client_creds, 0, sizeof(*client_creds));

    client_request  = malloc(sizeof(*client_request));
    memset(client_request, 0, sizeof(*client_request));

    server_response = malloc(sizeof(*server_response));
    memset(server_response, 0, sizeof(*server_response));

    /* Set response OK unless error... */
    server_response->response_type =  MYPROXY_OK_RESPONSE;
 
    /* Create a new gsi socket */
    attrs->gsi_socket = GSI_SOCKET_new(attrs->socket_fd);
    if (attrs->gsi_socket == NULL) {
        myproxy_log_perror("GSI_SOCKET_new()");
        return -1;
    }

#if !defined(CONDITIONAL_ENCRYPTION)
    if (GSI_SOCKET_set_encryption(attrs->gsi_socket, 1) == GSI_SOCKET_ERROR)
    {
	GSI_SOCKET_get_error_string(attrs->gsi_socket, error_string,
                                   sizeof(error_string));
	myproxy_log("Error enabling encryption: %s\n", error_string);
	return -1;
    }
#endif

    /* Authenticate server to client and get DN of client */
    if (myproxy_authenticate_accept(attrs, client_name, sizeof(client_name)) < 0) {
	/* Client_name may not be set on error so don't use it. */
	myproxy_log_verror();
	respond_with_error_and_die(attrs, "authentication failed");
    }

    /* Log client name */
    myproxy_log("Authenticated client %s", client_name); 
    
    /* Receive client request */
    requestlen = myproxy_recv(attrs, client_buffer, sizeof(client_buffer));
    if (requestlen < 0) {
        myproxy_log_verror();
	respond_with_error_and_die(attrs, "Error in myproxy_recv()");
    }
   
    /* Deserialize client request */
    if (myproxy_deserialize_request(client_buffer, requestlen, 
                                    client_request) < 0) {
	myproxy_log_verror();
        respond_with_error_and_die(attrs,
				   "error in myproxy_deserialize_request()");
    }

    /* Check client version */
    if (strcmp(client_request->version, MYPROXY_VERSION) != 0) {
	myproxy_log("client %s Invalid version number (%s) received",
		    client_name, client_request->version);
        respond_with_error_and_die(attrs,
				   "Invalid version number received.\n");
    }

    /* Check client username and pass phrase */
    if ((client_request->username == NULL) ||
	(strlen(client_request->username) == 0)) 
    {
	myproxy_log("client %s Invalid username (%s) received",
		    client_name,
		    (client_request->username == NULL ? "<NULL>" :
		     client_request->username));
	respond_with_error_and_die(attrs,
				   "Invalid username received.\n");
    }

    if ((client_request->passphrase == NULL) ||
	(strlen(client_request->passphrase) == 0)) 
    {
	myproxy_log("client %s Invalid pass phrase received",
		    client_name);
	respond_with_error_and_die(attrs,
				   "Invalid pass phrase received.\n");
    }

    /* XXX Put real pass word policy here */
    if (strlen(client_request->passphrase) < MIN_PASS_PHRASE_LEN)
    {
	myproxy_log("client %s Pass phrase too short",
		    client_name);
	respond_with_error_and_die(attrs,
				   "Pass phrase too short.\n");
    }
	
    /* Fill in credential structure = owner, user, passphrase, proxy location */
    client_creds->owner_name  = strdup(client_name);
    client_creds->user_name   = strdup(client_request->username);
    client_creds->pass_phrase = strdup(client_request->passphrase);
    
    /* Check authorization for request */
    switch (client_request->command_type)	
    {	
      case MYPROXY_GET_PROXY:
	/* Only servers are allowed to retrieve proxies */
	authorization_ok = myproxy_server_check_service(context,
							client_name);
	break;
	
      case MYPROXY_PUT_PROXY:
      case MYPROXY_INFO_PROXY:
      case MYPROXY_DESTROY_PROXY:
	/* Only a client can do these operations */
	authorization_ok = myproxy_server_check_client(context, client_name);
	
	break;
    }

    if (authorization_ok == -1)
    {
	myproxy_log_verror();
	respond_with_error_and_die(attrs, "Error checking authorization");
    }
    
    if (authorization_ok != 1)
    {
	respond_with_error_and_die(attrs, "Authorization failed");
    }

    /*
     * Find out if the crentials already exist and if the client owns
     * them. These values are then used below for further checking.
     */
    credentials_exist = myproxy_creds_exist(client_request->username);
    
    if (credentials_exist == -1)
    {
	myproxy_log_verror();
	respond_with_error_and_die(attrs,
				    "Error checking credential existance");
    }
    
    if (credentials_exist == 1)
    {
	/* If credentials exist are we the owner? */
	client_owns_credentials =
	    myproxy_creds_is_owner(client_request->username,
				   client_name);
	
    }
    else 
    {
	client_owns_credentials = 0;
    }
    
    if (client_owns_credentials == -1)
    {
	myproxy_log_verror();
	respond_with_error_and_die(attrs,
				    "Error checking credential ownership");
    }	
	
    /* Handle client request */
    switch (client_request->command_type) {
    case MYPROXY_GET_PROXY:
	/* log request type */
        myproxy_log("Received GET request from %s", client_name);

	/* Check that passphrase matches and that creds. can be retrieved */
	if (myproxy_creds_retrieve(client_creds) < 0) {
	    myproxy_log_verror();
	    respond_with_error_and_die(attrs, "Unable to retrieve credentials.\n");
	}

	myproxy_debug("  Username is \"%s\"", client_request->username);
	myproxy_debug("  Location is %s", client_creds->location);
	myproxy_debug("  Lifetime is %d seconds", client_request->portal_lifetime);

	/* return server response */
	send_response(attrs, server_response, client_name);
	
        get_proxy(attrs, client_creds, client_request, server_response);
        break;
    case MYPROXY_PUT_PROXY:
	/* log request type */
        myproxy_log("Received PUT request from %s", client_name);
	myproxy_debug("  Username is \"%s\"", client_request->username);
	myproxy_debug("  Lifetime is %d seconds", client_request->portal_lifetime);

	/*
	 * If credentials exist, the user must own them
	 */
	if (credentials_exist &&
	    !client_owns_credentials)
	{
	    myproxy_log("Username \"%s\" in use by another client",
			client_request->username);
	    respond_with_error_and_die(attrs,
				       "Username in use by another client");
	}

	/* Set lifetime of credentials on myproxy-server */ 
	client_creds->lifetime = client_request->portal_lifetime;

	/* return server response */
	send_response(attrs, server_response, client_name);
        put_proxy(attrs, client_creds, server_response);
        break;
    case MYPROXY_INFO_PROXY:
	/* log request type */
        myproxy_log("Received client %s command: INFO", client_name);
	myproxy_debug("  Username is \"%s\"", client_request->username);
        info_proxy(client_creds, server_response);
        break;
    case MYPROXY_DESTROY_PROXY:
	/* log request type */
        myproxy_log("Received client %s command: DESTROY", client_name);
	myproxy_debug("  Username is \"%s\"", client_request->username);

	/*
	 * If credentials exist, the user must own them
	 */
	if (credentials_exist &&
	    !client_owns_credentials)
	{
	    myproxy_log("Credentials not owned by client",
			client_request->username);
	    respond_with_error_and_die(attrs,
				       "Not owner");
	}

        destroy_proxy(client_creds, server_response);
        break;
    default:
	/* log request type */
        myproxy_log("Received client %s command: Invalid request type");
        strcat(server_response->error_string, "Invalid client request command.\n");
        break;
    }
    
    /* return server response */
    send_response(attrs, server_response, client_name);

    /* Log request */
    myproxy_log("Client %s disconnected", client_name);
   
    /* free stuff up */
    if (client_creds != NULL) {
	if (client_creds->owner_name != NULL) {
	    free(client_creds->owner_name);
	}
	if (client_creds->user_name != NULL) {
	    free(client_creds->user_name);
	}
	if (client_creds->pass_phrase != NULL) {
	    free(client_creds->pass_phrase);
	}
	if (client_creds->location != NULL) {
	    free(client_creds->location);
	}
	free(client_creds);
    }

    myproxy_free(attrs, client_request, server_response);
    if (context->config_file != NULL) {
        free(context->config_file);
        context->config_file = NULL;
    }
    free(context);

    return 0;
}

int 
init_arguments(int argc, char *argv[], 
               myproxy_socket_attrs_t *attrs, 
               myproxy_server_context_t *context) 
{   
    extern char *gnu_optarg;

    int arg;
    int arg_error = 0;

    char *last_directory_seperator;
    char directory_seperator = '/';
    
    /* Could do something smarter to get FQDN */
    attrs->pshost = strdup("localhost");
    
    attrs->psport = MYPROXY_SERVER_PORT;

    /* Get my name, removing any preceding path */
    last_directory_seperator = strrchr(argv[0], directory_seperator);
    
    if (last_directory_seperator == NULL)
    {
	context->my_name = strdup(argv[0]);
    }
    else
    {
	context->my_name = strdup(last_directory_seperator + 1);
    }
    
    while((arg = gnu_getopt_long(argc, argv, short_options, 
			     long_options, NULL)) != EOF) 
    {
        switch(arg) 
        {
        case 'p': 	/* port */
            attrs->psport = atoi(gnu_optarg);
            break;
        case 'h': 	/* print help and exit */
            fprintf(stderr, usage);
            exit(1);
            break;
        case 'c':
            context->config_file =  malloc(strlen(gnu_optarg) + 1);
            strcpy(context->config_file, gnu_optarg);   
            break;
        case 'v': /* print version and exit */
            fprintf(stderr, version);
            exit(1);
            break;
        case 's': /* set the credential storage directory */
            { char *s;
            s=(char *) malloc(strlen(gnu_optarg) + 1);
            strcpy(s,gnu_optarg);
            myproxy_set_storage_dir(s);
            break;
            }
	case 'u': /* print version and exit */
            fprintf(stderr, usage);
            exit(1);
            break;
        case 'd':
            debug = 1;
            break;
        default: /* ignore unknown */ 
            arg_error = -1;
            break;	
        }
    }

    return arg_error;
}

/*
 * myproxy_init_server()
 *
 * Create a generic server socket ready on the given port ready to accept.
 *
 * returns the listener fd on success 
 */
int 
myproxy_init_server(myproxy_socket_attrs_t *attrs) 
{
    int on = 1;
    int listen_sock;
    struct sockaddr_in sin;
    
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);

    if (listen_sock == -1) {
        failure("Error in socket()");
    } 

    /* Allow reuse of socket */
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, (void *) &on, sizeof(on));

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(attrs->psport);

    if (bind(listen_sock, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
	    failure("Error in bind()");
    }
    if (listen(listen_sock, 5) < 0) {
	    failure("Error in listen()");
    }
    return listen_sock;
}

void
respond_with_error_and_die(myproxy_socket_attrs_t *attrs,
			   const char *error)
{
    myproxy_response_t		response;
    int				responselen;
    char			response_buffer[2048];
    

    response.version = strdup(MYPROXY_VERSION);
    response.response_type = MYPROXY_ERROR_RESPONSE;
    my_strncpy(response.error_string, error, sizeof(response.error_string));
    
    responselen = myproxy_serialize_response(&response,
					     response_buffer,
					     sizeof(response_buffer));
    
    if (responselen < 0) {
        my_failure("error in myproxy_serialize_response()");
    }

    if (myproxy_send(attrs, response_buffer, responselen) < 0) {
        my_failure("error in myproxy_send()\n");
    } 

    myproxy_log("ERROR: %s. Exiting", error);
    
    exit(1);
}

void send_response(myproxy_socket_attrs_t *attrs, myproxy_response_t *response, char *client_name) {
    char server_buffer[1024];
    int responselen;
    assert(response != NULL);

    /* set version */
    response->version = malloc(strlen(MYPROXY_VERSION) + 1);
    sprintf(response->version, "%s", MYPROXY_VERSION);

    responselen = myproxy_serialize_response(response, server_buffer, sizeof(server_buffer));
    
    if (responselen < 0) {
        my_failure("error in myproxy_serialize_response()");
    }

    /* Log response */
    if (response->response_type == MYPROXY_OK_RESPONSE) {
      myproxy_debug("Sending OK response to client %s", client_name);
    } else if (response->response_type == MYPROXY_ERROR_RESPONSE) {
      myproxy_debug("Sending ERROR response \"%s\" to client %s", response->error_string, client_name);
    }

    if (myproxy_send(attrs, server_buffer, responselen) < 0) {
	myproxy_log_verror();
        my_failure("error in myproxy_send()\n");
    } 
    free(response->version);
    response->version = NULL;

    return;
}

void get_proxy(myproxy_socket_attrs_t *attrs, 
	       myproxy_creds_t *creds,
	       myproxy_request_t *request,
	       myproxy_response_t *response) 
{
    int min_lifetime;
  
    /* Delegate credentials to client */
    min_lifetime = MIN(creds->lifetime, request->portal_lifetime);

    if (myproxy_init_delegation(attrs, creds->location, min_lifetime) < 0) {
        myproxy_log_verror();
	response->response_type =  MYPROXY_ERROR_RESPONSE; 
	strcat(response->error_string, "Unable to delegate credentials.\n");
    } else {
        myproxy_log("Delegating credentials for %s lifetime=%d", creds->owner_name, min_lifetime);
	response->response_type = MYPROXY_OK_RESPONSE;
    } 
}

void put_proxy(myproxy_socket_attrs_t *attrs, 
	      myproxy_creds_t *creds, 
	      myproxy_response_t *response) 
{
    char delegfile[64];

    myproxy_debug("Storing credentials for username \"%s\"", creds->user_name);
    myproxy_debug("  Owner is \"%s\"", creds->owner_name);
    myproxy_debug("  Delegation lifetime is %d seconds", creds->lifetime);
    
    /* Accept delegated credentials from client */
    if (myproxy_accept_delegation(attrs, delegfile, sizeof(delegfile)) < 0) {
	myproxy_log_verror();
        response->response_type =  MYPROXY_ERROR_RESPONSE; 
        strcat(response->error_string, "Failed to accept credentials.\n"); 
	return;
    }

    myproxy_debug("  Accepted delegation: %s", delegfile);
 
    creds->location = strdup(delegfile);

    if (myproxy_creds_store(creds) < 0) {
	myproxy_log_verror();
        response->response_type =  MYPROXY_ERROR_RESPONSE; 
        strcat(response->error_string, "Unable to store credentials.\n"); 
    } else {
	response->response_type = MYPROXY_OK_RESPONSE;
    }

    /* Clean up temporary delegation */
    if (unlink(delegfile) != 0)
    {
	myproxy_log_perror("Removal of temporary credentials file %s failed",
			   delegfile);
    }
}

void info_proxy(myproxy_creds_t *creds, myproxy_response_t *response) {
    response->response_type = MYPROXY_OK_RESPONSE;
}

void destroy_proxy(myproxy_creds_t *creds, myproxy_response_t *response) {
    
    myproxy_debug("Deleting credentials for username \"%s\"", creds->user_name);
    myproxy_debug("  Owner is \"%s\"", creds->owner_name);
    myproxy_debug("  Delegation lifetime is %d seconds", creds->lifetime);
    
    if (myproxy_creds_delete(creds) < 0) { 
	myproxy_log_verror();
        response->response_type =  MYPROXY_ERROR_RESPONSE; 
        strcat(response->error_string, "Unable to delete credential.\n"); 
    } else {
	response->response_type = MYPROXY_OK_RESPONSE;
    }
 
}

/*
 * my_signal
 *
 * installs a signal handler, and returns the old handler.
 * This emulates the semi-standard signal() function in a
 * standard way using the Posix sigaction function.
 *
 * from Stevens, 1998, section 5.8
 */
Sigfunc *my_signal(int signo, Sigfunc *func)
{
    struct sigaction new_action, old_action;

    assert(func != NULL);

    new_action.sa_handler = func;
    sigemptyset( &new_action.sa_mask );
    new_action.sa_flags = 0;

    if (signo == SIGALRM) {
#ifdef SA_INTERRUPT
        new_action.sa_flags |= SA_INTERRUPT;  /* SunOS 4.x */
#endif
    }
    else { 
#ifdef SA_RESTART
        new_action.sa_flags |= SA_RESTART;    /* SVR4, 4.4BSD */
#endif
    }

    if (sigaction(signo, &new_action, &old_action) < 0) {
        return SIG_ERR;
    }
    else {
        return old_action.sa_handler;
    }
} 

void
sig_chld(int signo) {
    pid_t pid;
    int   stat;
    
    while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0)
        myproxy_debug("child %d terminated", pid);
    return;
} 

void sig_exit(int signo) {
    myproxy_log("Server killed by signal %d", signo);
    exit(0);
}


static void
failure(const char *failure_message) {
    myproxy_log_perror("Failure: %s", failure_message);
    exit(1);
} 

static void
my_failure(const char *failure_message) {
    myproxy_log("Failure: %s", failure_message);       
    exit(1);
} 

static char *
timestamp(void)
{
    time_t clock;
    struct tm *tmp;

    time(&clock);
    tmp = (struct tm *)localtime(&clock);
    return (char *)asctime(tmp);
}

static int
become_daemon(myproxy_server_context_t *context)
{
    pid_t childpid;
    int fd = 0;
    int fdlimit;
    
    /* Steps taken from UNIX Programming FAQ */
    
    /* 1. Fork off a child so the new process is not a process group leader */
    childpid = fork();
    switch (childpid) {
    case 0:         /* child */
      break;
    case -1:        /* error */
      perror("Error in fork()");
      return -1;
    default:        /* exit the original process */
      _exit(0);
    }

    /* 2. Set session id to become a process group and session group leader */
    if (setsid() < 0) { 
        perror("Error in setsid()"); 
	return -1;
    } 

    /* 3. Fork again so the parent, (the session group leader), can exit.
          This means that we, as a non-session group leader, can never 
          regain a controlling terminal. 
    */
    signal(SIGHUP, SIG_IGN);
    childpid = fork();
    switch (childpid) {
    case 0:             /* child */
	break;
    case -1:            /* error */
	perror("Error in fork()");
	return -1;
    default:            /* exit the original process */
	_exit(0);
    }
	
   
    
    /* 4. `chdir("/")' to ensure that our process doesn't keep any directory in use */
    chdir("/");

    /* 5. `umask(0)' so that we have complete control over the permissions of 
          anything we write
    */
    umask(0);

    /* 6. Close all file descriptors */
    fdlimit = sysconf(_SC_OPEN_MAX);
    while (fd < fdlimit)
      close(fd++);

    /* 7.Establish new open descriptors for stdin, stdout and stderr */    
    (void)open("/dev/null", O_RDWR);
    dup(0); 
    dup(0);
#ifdef TIOCNOTTY
    fd = open("/dev/tty", O_RDWR);
    if (fd >= 0) {
      ioctl(fd, TIOCNOTTY, 0);
      (void)close(fd);
    } 
#endif /* TIOCNOTTY */
    return 0;
}
