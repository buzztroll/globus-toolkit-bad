/*
 * Portions of this file Copyright 1999-2005 University of Chicago
 * Portions of this file Copyright 1999-2005 The University of Southern California.
 *
 * This file or a portion of this file is licensed under the
 * terms of the Globus Toolkit Public License, found at
 * http://www.globus.org/toolkit/download/license.html.
 * If you redistribute this file, with or without
 * modifications, you must include this notice in the file.
 */


#ifndef HEADER_PROXYPOLICY_H
#define HEADER_PROXYPOLICY_H

/**
 * @defgroup proxypolicy ProxyPolicy
 *
 * @author Sam Meder
 * @author Sam Lang
 *
 * The proxypolicy set of data structures
 * and functions provides an interface to generating
 * a PROXYPOLICY structure which is maintained as
 * a field in the PROXYCERTINFO structure,
 * and ultimately gets written to a DER encoded string.
 *
 * @see Further Information about proxy policies
 * is available in the Internet Draft Document:
 * 
 * draft-ietf-pkix-proxy-01.txt
 */

#ifndef EXTERN_C_BEGIN
#    ifdef __cplusplus
#        define EXTERN_C_BEGIN extern "C" {
#        define EXTERN_C_END }
#    else
#        define EXTERN_C_BEGIN
#        define EXTERN_C_END
#    endif
#endif

EXTERN_C_BEGIN

#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <string.h>

#define ANY_LANGUAGE_OID         "1.3.6.1.5.5.7.21.0"
#define ANY_LANGUAGE_SN          "ANY_LANGUAGE"
#define ANY_LANGUAGE_LN          "Any Language"

#define IMPERSONATION_PROXY_OID         "1.3.6.1.5.5.7.21.1"
#define IMPERSONATION_PROXY_SN          "IMPERSONATION_PROXY"
#define IMPERSONATION_PROXY_LN          "GSI impersonation proxy"

#define INDEPENDENT_PROXY_OID           "1.3.6.1.5.5.7.21.2"
#define INDEPENDENT_PROXY_SN            "INDEPENDENT_PROXY"
#define INDEPENDENT_PROXY_LN            "GSI independent proxy"

#define LIMITED_PROXY_OID               "1.3.6.1.4.1.3536.1.1.1.9"
#define LIMITED_PROXY_SN                "LIMITED_PROXY"
#define LIMITED_PROXY_LN                "GSI limited proxy"

/* Used for error handling */
#define ASN1_F_PROXYPOLICY_NEW          450
#define ASN1_F_D2I_PROXYPOLICY          451

/* data structures */

/**
 * @ingroup proxypolicy
 *
 * @note NOTE: The API provides functions to manipulate
 * the fields of a PROXYPOLICY.  Accessing the fields
 * directly will not work.
 *
 * This typedef maintains information about the policies
 * that have been placed on a proxy certificate
 *
 * @param policy_language defines which policy language
 * is to be used to define the policies
 * @param policy the policy that determines the policies
 * on a certificate
 */
struct PROXYPOLICY_st
{
    ASN1_OBJECT *                       policy_language;
    ASN1_OCTET_STRING *                 policy;
};

typedef struct PROXYPOLICY_st PROXYPOLICY;

DECLARE_STACK_OF(PROXYPOLICY)
DECLARE_ASN1_SET_OF(PROXYPOLICY)

/* functions */

ASN1_METHOD * PROXYPOLICY_asn1_meth();

PROXYPOLICY * PROXYPOLICY_new();

void PROXYPOLICY_free();

PROXYPOLICY * PROXYPOLICY_dup(
    PROXYPOLICY *                       policy);

int PROXYPOLICY_cmp(
    const PROXYPOLICY *                 a,
    const PROXYPOLICY *                 b);

int PROXYPOLICY_print(
    BIO *                               bp,
    PROXYPOLICY *                       policy);

int PROXYPOLICY_print_fp(
    FILE *                              fp,
    PROXYPOLICY *                       policy);

int PROXYPOLICY_set_policy_language(
    PROXYPOLICY *                       policy,
    ASN1_OBJECT *                       policy_language);

ASN1_OBJECT * PROXYPOLICY_get_policy_language(
    PROXYPOLICY *                       policy);

int PROXYPOLICY_set_policy(
    PROXYPOLICY *                       proxypolicy,
    unsigned char *                     policy,
    int                                 length);

unsigned char * PROXYPOLICY_get_policy(
    PROXYPOLICY *                       policy,
    int *                               length);

int i2d_PROXYPOLICY(
    PROXYPOLICY *                       policy,
    unsigned char **                    a);

PROXYPOLICY * d2i_PROXYPOLICY(
    PROXYPOLICY **                      policy,
    unsigned char **                    a,
    long                                length);

X509V3_EXT_METHOD * PROXYPOLICY_x509v3_ext_meth();

STACK_OF(CONF_VALUE) * i2v_PROXYPOLICY(
    struct v3_ext_method *              method,
    PROXYPOLICY *                       ext,
    STACK_OF(CONF_VALUE) *              extlist);

EXTERN_C_END

#endif /* HEADER_PROXYPOLICY_H */
