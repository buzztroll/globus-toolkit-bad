/*
 * myproxy.h
 *
 * Main header for MyProxy library
 *
 */

#ifndef __MYPROXY_H
#define __MYPROXY_H

#define MYPROXY_VERSION "MYPROXYv2"	/* protocol version string */

/* software version constants */
#define MYPROXY_VERSION_MAJOR 0
#define MYPROXY_VERSION_MINOR 5
#define MYPROXY_VERSION_MICRO 4
#define MYPROXY_VERSION_DATE "v0.5.4 2 May 2003"

#include "myproxy_constants.h"
#include "myproxy_authorization.h"
#include "myproxy_protocol.h"
#include "myproxy_creds.h"
#include "myproxy_delegation.h"
#include "myproxy_log.h"
#include "myproxy_read_pass.h"
#include "myproxy_server.h"
#include "verror.h"

#endif /* __MYPROXY_H */
