/*
 * myproxy-init
 *
 * Client program to delegate a credential to a myproxy-server
 */

#include "myproxy.h"
#include "myproxy_server.h"
#include "gnu_getopt.h"
#include "version.h"
#include "verror.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/* specify default lifetime for delegated credentials */
#define MYPROXY_DEFAULT_HOURS  84

static char usage[] = \
"\n"\
"Syntax: myproxy-init [-h hours] [-l username] ...\n"\
"        myproxy-init [--usage|--help] [-v|--version]\n"\
"\n"\
"    Options\n"\
"    --help | --usage            Displays usage\n"\
"    -v | -version             Displays version\n"\
"    -l | -username <username> Specifies the username for the delegated proxy\n"\
"    -h | -hours    <hours>    Specifies the lifetime of the delegated proxy\n"\
"    -s | -pshost   <hostname> Specifies the hostname of the myproxy-server\n"\
"    -p | -psport   #          Specifies the port of the myproxy-server\n"\
"\n";

struct option long_options[] =
{
  {"help",             no_argument, NULL, 'u'},
  {"pshost",   	 required_argument, NULL, 's'},
  {"psport",     required_argument, NULL, 'p'},
  {"hours",      required_argument, NULL, 'h'},
  {"usage",            no_argument, NULL, 'u'},
  {"username",   required_argument, NULL, 'l'},
  {"version",          no_argument, NULL, 'v'},
  {0, 0, 0, 0}
};

static char short_options[] = "us:p:t:h:v";

static char version[] =
"myproxy-init version " MYPROXY_VERSION " (" MYPROXY_VERSION_DATE ") "  "\n";


/* Function declarations */
void init_arguments(int argc, char *argv[], 
		    myproxy_socket_attrs_t *attrs, myproxy_request_t *request);

int  grid_proxy_init(int hours, const char *proxyfile);

int  grid_proxy_destroy(const char *proxyfile);

int  read_passphrase(char *passphrase, const int passlen, 
                     const int min, const int max);

void receive_response(myproxy_socket_attrs_t *attrs, myproxy_response_t *response); 

int
main(int argc, char *argv[]) 
{    
    int hours;
    char *username, *pshost; 
    char proxyfile[64];
    char request_buffer[1024]; 
    int requestlen;

    myproxy_socket_attrs_t *socket_attrs;
    myproxy_request_t      *client_request;
    myproxy_response_t     *server_response;
    
    socket_attrs = malloc(sizeof(*socket_attrs));
    memset(socket_attrs, 0, sizeof(*socket_attrs));

    client_request = malloc(sizeof(*client_request));
    memset(client_request, 0, sizeof(*client_request));

    server_response = malloc(sizeof(*server_response));
    memset(server_response, 0, sizeof(*server_response));

    /* setup defaults */
    client_request->version = malloc(strlen(MYPROXY_VERSION) + 1);
    strcpy(client_request->version, MYPROXY_VERSION);
    client_request->command_type = MYPROXY_PUT_PROXY;

    username = getenv("LOGNAME");
    if (username != NULL) {
	client_request->username = malloc(strlen(username)+1);
	strcpy(client_request->username, username);
    }

    pshost = getenv("MYPROXY_SERVER");
    if (pshost != NULL) {
	socket_attrs->pshost = malloc(strlen(pshost) + 1);
	strcpy(socket_attrs->pshost, pshost);
    }

    client_request->lifetime_seconds = 60*60*MYPROXY_DEFAULT_HOURS;
 
    socket_attrs->psport = MYPROXY_SERVER_PORT;

    /* Initialize client arguments and create client request object */
    init_arguments(argc, argv, socket_attrs, client_request);

    /* Create a proxy by running [grid-proxy-init] */
    sprintf(proxyfile, "%s.%s", MYPROXY_DEFAULT_PROXY, client_request->username);

    /* Run grid-proxy-init to create a proxy */
    hours = (int)(client_request->lifetime_seconds/3600);
    if (grid_proxy_init(hours, proxyfile) != 0) {
        fprintf(stderr, "Program grid_proxy_init failed\n");
        exit(1);
    }

    /* Allow user to provide a passphrase */
    if (read_passphrase(client_request->passphrase, MAX_PASS_LEN+1, 
                                       MIN_PASS_LEN, MAX_PASS_LEN) < 0) {
        fprintf(stderr, "error in myproxy_read_passphrase(): %s\n", verror_get_string());
        exit(1);
    }
    
    /* Set up client socket attributes */
    if (myproxy_init_client(socket_attrs) < 0) {
        fprintf(stderr, "error in myproxy_init_client(): %s\n", 
		verror_get_string());
        exit(1);
    }

    /* Authenticate client to server */
    if (myproxy_authenticate_init(socket_attrs, proxyfile) < 0) {
        fprintf(stderr, "error in myproxy_authenticate_init(): %s\n", 
		verror_get_string());
        exit(1);
    }

    /* Serialize client request object */
    requestlen = myproxy_serialize_request(client_request, 
                                           request_buffer, sizeof(request_buffer));
    if (requestlen < 0) {
        fprintf(stderr, "error in myproxy_serialize_request()\n");
        exit(1);
    }

    /* Send request to the myproxy-server */
    if (myproxy_send(socket_attrs, request_buffer, requestlen) < 0) {
        fprintf(stderr, "error in myproxy_send_request(): %s\n", 
		verror_get_string());
        exit(1);
    }

    /* Continue unless the response is not OK */
    receive_response(socket_attrs, server_response);
    
    /* Delegate credentials to server  */
    if (myproxy_init_delegation(socket_attrs, proxyfile, client_request->lifetime_seconds) < 0) {
	fprintf(stderr, "error in myproxy_init_delegation(): %s\n", 
		verror_get_string());
	exit(1);
    }

    /* Delete proxy file */
    if (grid_proxy_destroy(proxyfile) != 0) {
        fprintf(stderr, "Program grid_proxy_destroy failed\n");
        exit(1);
    }
    
    /* Get final response from server */
    receive_response(socket_attrs, server_response);

    printf("A proxy valid for %d hours for user %s now exists on %s.\n", 
	   hours, client_request->username, socket_attrs->pshost); 
    
    /* free memory allocated */
    myproxy_destroy(socket_attrs, client_request, server_response);

    exit(0);
}

void 
init_arguments(int argc, 
	       char *argv[], 
	       myproxy_socket_attrs_t *attrs,
	       myproxy_request_t *request) 
{   
    extern char *gnu_optarg;

    int arg;

    while((arg = gnu_getopt_long(argc, argv, short_options, 
				 long_options, NULL)) != EOF) 
    {
	switch(arg) 
	{
	    
	case 'h': 	/* Specify lifetime in seconds */
	    request->lifetime_seconds = 60*60*atoi(gnu_optarg);
	    break;      
	case 's': 	/* pshost name */
	    attrs->pshost = malloc(strlen(gnu_optarg) + 1);
	    strcpy(attrs->pshost, gnu_optarg); 
	    break;
	case 'p': 	/* psport */
	    attrs->psport = atoi(gnu_optarg);
	    break;
	case 'u': 	/* print help and exit */
	    fprintf(stderr, usage);
	    exit(1);
	    break;
	case 'l':	/* username */
	    request->username = malloc(strlen(gnu_optarg) + 1);
	    strcpy(request->username, gnu_optarg); 
	    break;
	case 'v': /* print version and exit */
	    fprintf(stderr, version);
	    exit(1);
	    break;
        default:  
	    fprintf(stderr, usage);
	    exit(1);
	    break;	
        }
    }
    /* Check to see if myproxy-server specified */
    if (attrs->pshost == NULL) {
	fprintf(stderr, "Unspecified myproxy-server! Either set the MYPROXY_SERVER environment variable or explicitly set the myproxy-server via the -s flag\n");
	exit(1);
    }

    /* Check to see if username specified */
    if (request->username == NULL) {
	fprintf(stderr, usage);
	fprintf(stderr, "Unspecified username!\n");
	exit(1);
    }

    /* Check to see that lifetime is < MYPROXY_SERVER_MAX_CRED_HOURS */
    if (request->lifetime_seconds > 60*60*MYPROXY_SERVER_MAX_CRED_HOURS) {
        fprintf(stderr, "The credential lifetime cannot be greater than %d.\n", MYPROXY_SERVER_MAX_CRED_HOURS);
        exit(1);
    } 
    return;
}


/* read_passphrase()
 * 
 * Reads a passphrase from stdin. The passphrase must be allocated and
 * be less than min and greater than max characters
 */
int
read_passphrase(char *passphrase, const int passlen, const int min, const int max) 
{
    int  i;
    char pass[1024];
    int  done = 0;

    assert(passphrase != NULL);
    assert(passlen < 1024);

    /* Get user's passphrase */    
    do {
        printf("Enter password to protect proxy on  myproxy-server:\n");
        
        if (!(fgets(pass, 1024, stdin))) {
            fprintf(stderr,"Failed to read password from stdin\n");   
            return -1;
        }	
        i = strlen(pass) - 1;
        if ((i < min) || (i > max)) {
            printf("Password must be between %d and %d characters\n", min, max);
        } else {
            done = 1;
        }
    } while (!done);
    
    if (pass[i] == '\n') {
        pass[i] = '\0';
    }
    strncpy(passphrase, pass, passlen);
    return 0;
}

/* grid_proxy_init()
 *
 * Uses the system() call to run grid-proxy-init to create a user proxy
 *
 * returns grid-proxy-init status 0 if OK, -1 on error
 */
int
grid_proxy_init(int hours, const char *proxyfile) {

    int rc;
    char command[128];
  
    assert(proxyfile != NULL);
    
    sprintf(command, "grid-proxy-init -hours %d -out %s", hours, proxyfile);
    rc = system(command);

    return rc;
}

/* grid_proxy_destroy()
 *
 * Uses the system() call to run grid-proxy-destroy to create a user proxy
 *
 * returns grid-proxy-destroy status 0 if OK, -1 on error
 */
int
grid_proxy_destroy(const char *proxyfile) {
  
    int rc;
    char command[128];

    assert(proxyfile != NULL);

    sprintf(command, "grid-proxy-destroy %s", proxyfile);
    rc = system(command);

    return rc;
}

void
receive_response(myproxy_socket_attrs_t *attrs, myproxy_response_t *response) {
    int responselen;
    char response_buffer[1024];

    /* Receive a response from the server */
    responselen = myproxy_recv(attrs, response_buffer, sizeof(response_buffer));
    if (responselen < 0) {
        fprintf(stderr, "error in myproxy_recv_response()\n");
        exit(1);
    }

    /* Make a response object from the response buffer */
    if (myproxy_deserialize_response(response, response_buffer, responselen) < 0) {
      fprintf(stderr, "error in myproxy_deserialize_response()\n");
      exit(1);
    }

    /* Check version */
    if (strcmp(response->version, MYPROXY_VERSION) != 0) {
      fprintf(stderr, "Received invalid version number from server\n");
      exit(1);
    } 

    /* Check response */
    switch(response->response_type) {
        case MYPROXY_ERROR_RESPONSE:
            fprintf(stderr, "Received ERROR: %s\n", response->error_string);
	    exit(1);
            break;
        case MYPROXY_OK_RESPONSE:
            break;
        default:
            fprintf(stderr, "Received unknown response type\n");
	    exit(1);
            break;
    }
    return;
}

