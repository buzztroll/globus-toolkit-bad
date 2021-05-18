#! groovy

@Library('gcs-build-scripts') _

def get_params(src_dir) {
    return [
        string(name: 'CHANGE_BRANCH', value: env.CHANGE_BRANCH ?: ""),
        string(name: 'CHANGE_ID', value: env.CHANGE_ID ?: ""),
        string(name: 'BRANCH_NAME', value: env.BRANCH_NAME ?: ""),
        string(name: 'EPIC', value: env.EPIC ?: ""),
        string(name: 'GIT_URL', value: env.GIT_URL ?: ""),
        string(name: 'TAG_NAME', value: env.TAG_NAME ?: ""),
        string(name: 'SOURCE_DIR', value: src_dir),
        string(name: 'STABLE_TAG', value: '${PACKAGE_NAME}-${PACKAGE_VERSION}')
    ]
}

pipeline {
    agent none
    parameters {
        booleanParam(
            name: 'ALL_PACKAGES',
            defaultValue: false,
            description: "Set to true to rebuild all packages, otherwise only when changed or when the per-package checkbox is selected"
        )
        booleanParam(
            name: 'COMMON',
            defaultValue: false,
            description: "Set to true to rebuild globus-common, otherwise only when changed"
        )
        booleanParam(
            name: 'XIO',
            defaultValue: false,
            description: "Set to true to rebuild globus-xio, otherwise only when changed"
        )
        booleanParam(
            name: 'GFORK',
            defaultValue: false,
            description: "Set to true to rebuild globus-gfork, otherwise only when changed"
        )
        booleanParam(
            name: 'OPENSSL_ERROR',
            defaultValue: false,
            description: "Set to true to rebuild globus-gsi-openssl-error, otherwise only when changed"
        )
        booleanParam(
            name: 'GSI_PROXY_SSL',
            defaultValue: false,
            description: "Set to true to rebuild globus-gsi-proxy-ssl, otherwise only when changed"
        )
        booleanParam(
            name: 'OPENSSL_MODULE',
            defaultValue: false,
            description: "Set to true to rebuild globus-openssl-module, otherwise only when changed"
        )
        booleanParam(
            name: "GSI_CERT_UTILS",
            defaultValue: false,
            description: "Set to true to rebuild globus-gsi-cert-utils, otherwise only when changed"
        )
        booleanParam(
            name: "GSI_SYSCONFIG",
            defaultValue: false,
            description: "Set to true to rebuild globus-gsi-sysconfig, otherwise only when changed"
        )
        booleanParam(
            name: "CALLOUT",
            defaultValue: false,
            description: "Set to true to rebuild globus-callout, otherwise only when changed"
        )
        booleanParam(
            name: "GSI_CALLBACK",
            defaultValue: false,
            description: "Set to true to rebuild globus-gsi-callback, otherwise only when changed"
        )
        booleanParam(
            name: "GSI_CREDENTIAL",
            defaultValue: false,
            description: "Set to true to rebuild globus-gsi-credential, otherwise only when changed"
        )
        booleanParam(
            name: "GSI_PROXY_CORE",
            defaultValue: false,
            description: "Set to true to rebuild globus-gsi-proxy-core, otherwise only when changed"
        )
        booleanParam(
            name: "GSSAPI_GSI",
            defaultValue: false,
            description: "Set to true to rebuild globus-gssapi-gsi, otherwise only when changed"
        )
        booleanParam(
            name: "GSS_ASSIST",
            defaultValue: false,
            description: "Set to true to rebuild globus-gss-assist, otherwise only when changed"
        )
        booleanParam(
            name: "GSSAPI_ERROR",
            defaultValue: false,
            description: "Set to true to rebuild globus-gssapi-error, otherwise only when changed"
        )
        booleanParam(
            name: "XIO_GSI_DRIVER",
            defaultValue: false,
            description: "Set to true to rebuild globus-xio-gsi-driver, otherwise only when changed"
        )
        booleanParam(
            name: "IO",
            defaultValue: false,
            description: "Set to true to rebuild globus-io, otherwise only when changed"
        )
        booleanParam(
            name: "FTP_CONTROL",
            defaultValue: false,
            description: "Set to true to rebuild globus-ftp-control, otherwise only when changed"
        )
        booleanParam(
            name: "USAGE",
            defaultValue: false,
            description: "Set to true to rebuild globus-usage, otherwise only when changed"
        )
        booleanParam(
            name: "XIO_PIPE_DRIVER",
            defaultValue: false,
            description: "Set to true to rebuild globus-xio-pipe-driver, otherwise only when changed"
        )
        booleanParam(
            name: "GRIDFTP_SERVER_CONTROL",
            defaultValue: false,
            description: "Set to true to rebuild globus-gridftp-server-control, otherwise only when changed"
        )
        booleanParam(
            name: "AUTHZ_CALLOUT_ERROR",
            defaultValue: false,
            description: "Set to true to rebuild globus-authz-callout-error, otherwise only when changed"
        )
        booleanParam(
            name: "AUTHZ",
            defaultValue: false,
            description: "Set to true to rebuild globus-authz, otherwise only when changed"
        )
        booleanParam(
            name: "XIO_RATE_DRIVER",
            defaultValue: false,
            description: "Set to true to rebuild globus-xio-rate-driver, otherwise only when changed"
        )
        booleanParam(
            name: "XIO_POPEN_DRIVER",
            defaultValue: false,
            description: "Set to true to rebuild globus-xio-popen-driver, otherwise only when changed"
        )
        booleanParam(
            name: "XIO_UDT_DRIVER",
            defaultValue: false,
            description: "Set to true to rebuild globus-xio-udt-driver, otherwise only when changed"
        )
        booleanParam(
            name: "NET_MANAGER",
            defaultValue: false,
            description: "Set to true to rebuild globus-net-manager, otherwise only when changed"
        )
        booleanParam(
            name: "GRIDFTP_SERVER",
            defaultValue: false,
            description: "Set to true to rebuild globus-gridftp-server, otherwise only when changed"
        )
        booleanParam(
            name: "GASS_TRANSFER",
            defaultValue: false,
            description: "Set to true to rebuild globus-gass-transfer, otherwise only when changed"
        )
        booleanParam(
            name: "FTP_CLIENT",
            defaultValue: false,
            description: "Set to true to rebuild globus-ftp-client, otherwise only when changed"
        )
        booleanParam(
            name: "GASS_COPY",
            defaultValue: false,
            description: "Set to true to rebuild globus-gass-copy, otherwise only when changed"
        )
        booleanParam(
            name: "XIO_GRIDFTP_MULTICAST",
            defaultValue: false,
            description: "Set to true to rebuild globus-xio-gridftp-multicast, otherwise only when changed"
        )
        booleanParam(
            name: "SIMPLE_CA",
            defaultValue: false,
            description: "Set to true to rebuild globus-simple-ca, otherwise only when changed"
        )
        booleanParam(
            name: "GRIDMAP_CALLOUT_ERROR",
            defaultValue: false,
            description: "Set to true to rebuild globus-gridmap-callout-error, otherwise only when changed"
        )
        booleanParam(
            name: "GRIDMAP_CALLOUT",
            defaultValue: false,
            description: "Set to true to rebuild globus-gridmap-callout, otherwise only when changed"
        )
        booleanParam(
            name: "XIO_GRIDFTP_DRIVER",
            defaultValue: false,
            description: "Set to true to rebuild globus-xio-gridftp-driver, otherwise only when changed"
        )
        booleanParam(
            name: "GRIDMAP_VERIFY_MYPROXY_CALLOUT",
            defaultValue: false,
            description: "Set to true to rebuild globus-gridmap-verify-myproxy-callout, otherwise only when changed"
        )
        booleanParam(
            name: "GRIDMAP_EPPN_CALLOUT",
            defaultValue: false,
            description: "Set to true to rebuild globus-gridmap-eppn-callout, otherwise only when changed"
        )
        booleanParam(
            name: "PROXY_UTILS",
            defaultValue: false,
            description: "Set to true to rebuild globus-proxy-utils, otherwise only when changed"
        )
        booleanParam(
            name: "XIOPERF",
            defaultValue: false,
            description: "Set to true to rebuild globus-xioperf, otherwise only when changed"
        )
        booleanParam(
            name: "MYPROXY",
            defaultValue: false,
            description: "Set to true to rebuild myproxy, otherwise only when changed"
        )
        booleanParam(
            name: "REPO",
            defaultValue: false,
            description: "Set to true to rebuild globus-repo, otherwise only when changed"
        )
    }
    options {
        buildDiscarder(logRotator(numToKeepStr: '5'))
        parallelsAlwaysFailFast()
        disableConcurrentBuilds()
    }
    stages {
        stage ("Checkout Source") {
            agent {
                label "package_creator"
            }
            steps {
                script {
                    def scmVars = checkout scm
                    env.GIT_URL = "${scmVars.GIT_URL}"
                    sh '''
                    git clean -fdx
                    mkdir -p m4
                    autoreconf -i --no-recursive
                    sh ./write-globus-version "$(cat globus-version.inc)"
                    ./update-dirt.sh
                    '''
                }
            }
        }
        stage ("Common") {
            when {
                anyOf {
                    equals expected: true, actual: params.COMMON;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "common/**/*";
                    changeset "packaging/debian/globus-common/**/*";
                    changeset "packaging/fedora/globus-common.spec";
                }
            }
            steps {
                build(job: 'ToolkitPackage', parameters: get_params("common/source"))
            }
        }
        stage ("XIO") {
            when {
                anyOf {
                    equals expected: true, actual: params.XIO;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "xio/src/**/*";
                    changeset "packaging/debian/globus-xio/**/*";
                    changeset "packaging/fedora/globus-xio.spec";
                }
            }
            steps {
                build(job: 'ToolkitPackage', parameters: get_params("xio/src"))
            }
        }
        stage ("GFork") {
            when {
                anyOf {
                    equals expected: true, actual: params.GFORK;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "gridftp/gfork/source/**/*";
                    changeset "packaging/debian/globus-gfork/**/*";
                    changeset "packaging/fedora/globus-gfork.spec";
                }
            }
            steps {
                build(job: 'ToolkitPackage', parameters: get_params("gridftp/gfork/source"))
            }
        }
        stage ("GSI: OpenSSL Error") {
            when {
                anyOf {
                    equals expected: true, actual: params.OPENSSL_ERROR;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "gsi/openssl_error/source/**/*";
                    changeset "packaging/debian/globus-gsi-openssl-error/**/*";
                    changeset "packaging/fedora/globus-gsi-openssl-error.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("gsi/openssl_error/source")
                )
            }
        }
        stage ("GSI: Proxy SSL") {
            when {
                anyOf {
                    equals expected: true, actual: params.GSI_PROXY_SSL;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "gsi/proxy/proxy_ssl/source/**/*";
                    changeset "packaging/debian/globus-gsi-proxy-ssl/**/*";
                    changeset "packaging/fedora/globus-gsi-proxy-ssl.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("gsi/proxy/proxy_ssl/source")
                )
            }
        }
        stage ("GSI: OpenSSL Module") {
            when {
                anyOf {
                    equals expected: true, actual: params.OPENSSL_MODULE;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "gsi/openssl_module/source/**/*";
                    changeset "packaging/debian/globus-openssl-module/**/*";
                    changeset "packaging/fedora/globus-openssl-module.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("gsi/openssl_module/source")
                )
            }
        }
        stage ("GSI: Cert Utils") {
            when {
                anyOf {
                    equals expected: true, actual: params.GSI_CERT_UTILS;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "gsi/cert_utils/source/**/*";
                    changeset "packaging/debian/globus-gsi-cert-utils/**/*";
                    changeset "packaging/fedora/globus-gsi-cert-utils.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("gsi/cert_utils/source")
                )
            }
        }
        stage ("GSI: Sysconfig") {
            when {
                anyOf {
                    equals expected: true, actual: params.GSI_SYSCONFIG;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "gsi/sysconfig/source/**/*";
                    changeset "packaging/debian/globus-gsi-sysconfig/**/*";
                    changeset "packaging/fedora/globus-gsi-sysconfig.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("gsi/sysconfig/source")
                )
            }
        }
        stage ("Callout") {
            when {
                anyOf {
                    equals expected: true, actual: params.CALLOUT;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "callout/source/**/*";
                    changeset "packaging/debian/globus-callout/**/*";
                    changeset "packaging/fedora/globus-callout.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("callout/source")
                )
            }
        }
        stage ("GSI Callback") {
            when {
                anyOf {
                    equals expected: true, actual: params.GSI_CALLBACK;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "gsi/callback/source/**/*";
                    changeset "packaging/debian/globus-gsi-callback/**/*";
                    changeset "packaging/fedora/globus-gsi-callback.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("gsi/callback/source")
                )
            }
        }
        stage ("GSI Credential") {
            when {
                anyOf {
                    equals expected: true, actual: params.GSI_CREDENTIAL;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "gsi/credential/source/**/*";
                    changeset "packaging/debian/globus-gsi-credential/**/*";
                    changeset "packaging/fedora/globus-gsi-credential.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("gsi/credential/source")
                )
            }
        }
        stage ("GSI Proxy Core") {
            when {
                anyOf {
                    equals expected: true, actual: params.GSI_PROXY_CORE;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "gsi/proxy/proxy_core/source/**/*";
                    changeset "packaging/debian/globus-gsi-proxy-core/**/*";
                    changeset "packaging/fedora/globus-gsi-proxy-core.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("gsi/proxy/proxy_core/source")
                )
            }
        }
        stage ("GSSAPI GSI") {
            when {
                anyOf {
                    equals expected: true, actual: params.GSSAPI_GSI;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "gsi/gssapi/source/**/*";
                    changeset "packaging/debian/globus-gssapi-gsi/**/*";
                    changeset "packaging/fedora/globus-gssapi-gsi.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("gsi/gssapi/source")
                )
            }
        }
        stage ("GSS Assist") {
            when {
                anyOf {
                    equals expected: true, actual: params.GSS_ASSIST;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "gsi/gss_assist/source/**/*";
                    changeset "packaging/debian/globus-gss-assist/**/*";
                    changeset "packaging/fedora/globus-gss-assist.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("gsi/gss_assist/source")
                )
            }
        }
        stage ("GSSAPI Error") {
            when {
                anyOf {
                    equals expected: true, actual: params.GSSAPI_ERROR;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "gsi/gssapi_error/source/**/*";
                    changeset "packaging/debian/globus-gssapi-error/**/*";
                    changeset "packaging/fedora/globus-gssapi-error.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("gsi/gssapi_error/source")
                )
            }
        }
        stage ("XIO GSI Driver") {
            when {
                anyOf {
                    equals expected: true, actual: params.XIO_GSI_DRIVER;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "xio/drivers/gsi/**/*";
                    changeset "packaging/debian/globus-xio-gsi-driver/**/*";
                    changeset "packaging/fedora/globus-xio-gsi-driver.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("xio/drivers/gsi")
                )
            }
        }
        stage ("IO") {
            when {
                anyOf {
                    equals expected: true, actual: params.IO;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "io/compat/**/*";
                    changeset "packaging/debian/globus-io/**/*";
                    changeset "packaging/fedora/globus-io.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("io/compat")
                )
            }
        }
        stage ("FTP Control") {
            when {
                anyOf {
                    equals expected: true, actual: params.FTP_CONTROL;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "gridftp/control/source/**/*";
                    changeset "packaging/debian/globus-ftp-control/**/*";
                    changeset "packaging/fedora/globus-ftp-control.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("gridftp/control/source")
                )
            }
        }
        stage ("Usage") {
            when {
                anyOf {
                    equals expected: true, actual: params.USAGE;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "usage/c/sender/source/**/*";
                    changeset "packaging/debian/globus-usage/**/*";
                    changeset "packaging/fedora/globus-usage.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("usage/c/sender/source")
                )
            }
        }
        stage ("XIO Pipe Driver") {
            when {
                anyOf {
                    equals expected: true, actual: params.XIO_PIPE_DRIVER;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "xio/drivers/pipe/source/**/*";
                    changeset "packaging/debian/globus-xio-pipe-driver/**/*";
                    changeset "packaging/fedora/globus-xio-pipe-driver.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("xio/drivers/pipe/source")
                )
            }
        }
        stage ("GridFTP Server Control") {
            when {
                anyOf {
                    equals expected: true, actual: params.GRIDFTP_SERVER_CONTROL;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "gridftp/server-lib/src/**/*";
                    changeset "packaging/debian/globus-gridftp-server-control/**/*";
                    changeset "packaging/fedora/globus-gridftp-server-control.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("gridftp/server-lib/src")
                )
            }
        }
        stage ("Authz Callout Error") {
            when {
                anyOf {
                    equals expected: true, actual: params.AUTHZ_CALLOUT_ERROR;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "gsi/authz/error/**/*";
                    changeset "packaging/debian/globus-authz-callout-error/**/*";
                    changeset "packaging/fedora/globus-authz-callout-error.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("gsi/authz/error")
                )
            }
        }
        stage ("Authz") {
            when {
                anyOf {
                    equals expected: true, actual: params.AUTHZ;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "gsi/authz/source/**/*";
                    changeset "packaging/debian/globus-authz/**/*";
                    changeset "packaging/fedora/globus-authz.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("gsi/authz/source")
                )
            }
        }
        stage ("XIO Rate Driver") {
            when {
                anyOf {
                    equals expected: true, actual: params.XIO_RATE_DRIVER;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "xio/drivers/rate/source/**/*";
                    changeset "packaging/debian/globus-xio-rate-driver/**/*";
                    changeset "packaging/fedora/globus-xio-rate-driver.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("xio/drivers/rate/source")
                )
            }
        }
        stage ("XIO Popen Driver") {
            when {
                anyOf {
                    equals expected: true, actual: params.XIO_POPEN_DRIVER;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "xio/drivers/popen/source/**/*";
                    changeset "packaging/debian/globus-xio-popen-driver/**/*";
                    changeset "packaging/fedora/globus-xio-popen-driver.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("xio/drivers/popen/source")
                )
            }
        }
        stage ("XIO UDT Driver") {
            when {
                anyOf {
                    equals expected: true, actual: params.XIO_UDT_DRIVER;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "xio/drivers/udt/source/**/*";
                    changeset "packaging/debian/globus-xio-udt-driver/**/*";
                    changeset "packaging/fedora/globus-xio-udt-driver.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("xio/drivers/udt/source")
                )
            }
        }
        stage ("Net Manager") {
            when {
                anyOf {
                    equals expected: true, actual: params.NET_MANAGER;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "gridftp/net_manager/**/*";
                    changeset "packaging/debian/globus-net-manager/**/*";
                    changeset "packaging/fedora/globus-net-manager.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("gridftp/net_manager")
                )
            }
        }
        stage ("GridFTP Server") {
            when {
                anyOf {
                    equals expected: true, actual: params.GRIDFTP_SERVER;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "gridftp/server/src/**/*";
                    changeset "packaging/debian/globus-gridftp-server/**/*";
                    changeset "packaging/fedora/globus-gridftp-server.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("gridftp/server/src")
                )
            }
        }
        stage ("GASS Transfer") {
            when {
                anyOf {
                    equals expected: true, actual: params.GASS_TRANSFER;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "gass/transfer/source/**/*";
                    changeset "packaging/debian/globus-gass-transfer/**/*";
                    changeset "packaging/fedora/globus-gass-transfer.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("gass/transfer/source")
                )
            }
        }
        stage ("FTP Client") {
            when {
                anyOf {
                    equals expected: true, actual: params.FTP_CLIENT;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "gridftp/client/source/**/*";
                    changeset "packaging/debian/globus-ftp-client/**/*";
                    changeset "packaging/fedora/globus-ftp-client.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("gridftp/client/source")
                )
            }
        }
        stage ("GASS Copy") {
            when {
                anyOf {
                    equals expected: true, actual: params.GASS_COPY;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "gass/copy/source";
                    changeset "packaging/debian/globus-gass-copy/**/*";
                    changeset "packaging/fedora/globus-gass-copy.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("gass/copy/source")
                )
            }
        }
        stage ("XIO GridFTP Multicast Driver") {
            when {
                anyOf {
                    equals expected: true, actual: params.XIO_GRIDFTP_MULTICAST;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "gridftp/server/multi/source/**/*";
                    changeset "packaging/debian/globus-xio-gridftp-multicast/**/*";
                    changeset "packaging/fedora/globus-xio-gridftp-multicast.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("gridftp/server/multi/source")
                )
            }
        }
        stage ("Simple CA") {
            when {
                anyOf {
                    equals expected: true, actual: params.SIMPLE_CA;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "gsi/simple_ca/source/**/*";
                    changeset "packaging/debian/globus-simple-ca/**/*";
                    changeset "packaging/fedora/globus-simple-ca.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("gsi/simple_ca/source")
                )
            }
        }
        stage ("Gridmap Callout Error") {
            when {
                anyOf {
                    equals expected: true, actual: params.GRIDMAP_CALLOUT_ERROR;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "gsi/gridmap_callout/error/**/*";
                    changeset "packaging/debian/globus-gridmap-callout-error/**/*";
                    changeset "packaging/fedora/globus-gridmap-callout-error.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("gsi/gridmap_callout/error")
                )
            }
        }
        stage ("Gridmap Callout") {
            when {
                anyOf {
                    equals expected: true, actual: params.GRIDMAP_CALLOUT;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "gsi/gridmap_callout/source/**/*";
                    changeset "packaging/debian/globus-gridmap-callout/**/*";
                    changeset "packaging/fedora/globus-gridmap-callout.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("gsi/gridmap_callout/source")
                )
            }
        }
        stage ("XIO GridFTP Driver") {
            when {
                anyOf {
                    equals expected: true, actual: params.XIO_GRIDFTP_DRIVER;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "gridftp/gridftp_driver/source/**/*";
                    changeset "packaging/debian/globus-xio-gridftp-driver/**/*";
                    changeset "packaging/fedora/globus-xio-gridftp-driver.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("gridftp/gridftp_driver/source")
                )
            }
        }
        stage ("Gridmap Verify MyProxy Callout") {
            when {
                anyOf {
                    equals expected: true, actual: params.GRIDMAP_VERIFY_MYPROXY_CALLOUT;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "gsi/gridmap_verify_myproxy_callout/**/*";
                    changeset "packaging/debian/globus-gridmap-verify-myproxy-callout/**/*";
                    changeset "packaging/fedora/globus-gridmap-verify-myproxy-callout.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("gsi/gridmap_verify_myproxy_callout")
                )
            }
        }
        stage ("Gridmap ePPN Callout") {
            when {
                anyOf {
                    equals expected: true, actual: params.GRIDMAP_EPPN_CALLOUT;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "gsi/gridmap_eppn_callout/**/*";
                    changeset "packaging/debian/globus-gridmap-eppn-callout/**/*";
                    changeset "packaging/fedora/globus-gridmap-eppn-callout.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("gsi/gridmap_eppn_callout")
                )
            }
        }
        stage ("Globus Proxy Utils") {
            when {
                anyOf {
                    equals expected: true, actual: params.PROXY_UTILS;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "gsi/proxy/proxy_utils/source/**/*";
                    changeset "packaging/debian/globus-proxy-utils/**/*";
                    changeset "packaging/fedora/globus-proxy-utils.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("gsi/proxy/proxy_utils/source")
                )
            }
        }
        stage ("XIOPerf") {
            when {
                anyOf {
                    equals expected: true, actual: params.XIOPERF;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "xio/xioperf/source/**/*";
                    changeset "packaging/debian/globus-xioperf/**/*";
                    changeset "packaging/fedora/globus-xioperf.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("xio/xioperf/source")
                )
            }
        }
        stage ("MyProxy") {
            when {
                anyOf {
                    equals expected: true, actual: params.MYPROXY;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "myproxy/source/**/*";
                    changeset "packaging/debian/myproxy/**/*";
                    changeset "packaging/fedora/myproxy.spec";
                }
            }
            steps {
                build(
                    job: 'ToolkitPackage',
                    parameters: get_params("myproxy/source")
                )
            }
        }
        stage ("Package Repos") {
            when {
                anyOf {
                    equals expected: true, actual: params.REPO;
                    equals expected: true, actual: params.ALL_PACKAGES;
                    changeset "packaging/debian/globus-repo/**/*";
                    changeset "packaging/fedora/globus-repo.spec";
                }
            }
            steps {
                script {
                    node (label: "debian_package_builder") {
                        checkout scm
                        def stash_name = "${UUID.randomUUID()}"
                        def props = null
                        dirs (path: stash_name, clean: true) {
                            withEnv(["STASH_NAME=${stash_name}"]) {
                                sh '''
                                    stash_dir="$(pwd)"
                                    cd ../packaging/debian/globus-repo
                                    dpkg-source -b .
                                    pkg="$(dpkg-parsechangelog -S Source)"
                                    version="$(dpkg-parsechangelog  -S version)"
                                    cp "../${pkg}_${version}.tar.xz" "${stash_dir}"
                                    cp -R debian "${stash_dir}/debian"
                                    cp "../../fedora/${pkg}.spec" "${stash_dir}"
                                    cd "${stash_dir}"
                                    cat <<-EOF > package.props
                                        NAME=${pkg}
                                        VERSION=${version}
                                        TARBALL=${pkg}_${version}.tar.xz
                                        STASH_NAME=${STASH_NAME}
                                        EOF
                                '''
                            }
                        }
                        stash(name: stash_name, includes: "${stash_name}/**/*")
                        props = readProperties(file: "${stash_name}/package.props")
                        env.GLOBUS_REPO_PKG = props.NAME
                        env.GLOBUS_REPO_VERSION = props.VERSION
                        dir (stash_name) {
                            deleteDir()
                        }
                        def (mock_targets, deb_targets) = enumerateBuildTargets()
                        def rpm_exclude = mock_targets.findAll {
                            it != "el-7"
                        }
                        def deb_exclude = deb_targets.findAll {
                            it != "bionic"
                        }
                        echo "rpm_exclude=${rpm_exclude}, deb_exclude=${deb_exclude}";

                        parallel "debian": {
                            env.DEB_REPO_STASH = buildDebian(
                                    props.STASH_NAME,
                                    props.TARBALL,
                                    false,
                                    getClubhouseEpic(),
                                    deb_exclude)
                        }, "rpm": {
                            env.RPM_REPO_STASH = buildMock(
                                props.STASH_NAME,
                                props.TARBALL,
                                false,
                                getClubhouseEpic(),
                                rpm_exclude)
                        }, "failFast": false
                    }
                    node (label: "master") {
                        def stashname = "${UUID.randomUUID()}"
                        dir("artifacts") {
                            if (env.DEB_REPO_STASH) {
                                unstash(name: env.DEB_REPO_STASH)
                            }
                            if (env.RPM_REPO_STASH) {
                                unstash(name: env.RPM_REPO_STASH)
                            }
                            withEnv([
                                "REPO_PKG=${env.GLOBUS_REPO_PKG}",
                                "REPO_VERSION=${env.GLOBUS_REPO_VERSION}",
                                ]) {
                                sh '''
                                    mkdir -p installers/repo/deb installers/repo/rpm
                                    find deb -name '*_all.deb' \
                                        -exec cp {} installers/repo/deb ";"
                                    cd installers/repo/deb
                                    ln -s ${REPO_PKG}_${REPO_VERSION}_all.deb \
                                        ${REPO_PKG}_latest_all.deb
                                    cd "$OLDPWD"

                                    find rpm -name '*.noarch.rpm' \
                                        -exec cp {} installers/repo/rpm/ ";"
                                    cd installers/repo/rpm
                                    ln -s ${REPO_PKG}-${REPO_VERSION}*.noarch.rpm \
                                        ${REPO_PKG}-latest.noarch.rpm
                                    cd "$OLDPWD"
                                    rm -rf deb rpm
                                    find .
                                '''
                            }
                            stash(name: stashname, includes: "**/*")
                            deleteDir()
                        }
                        publishResults(
                            stashname,
                            env.GLOBUS_REPO_PKG,
                            env.GLOBUS_REPO_VERSION,
                            false)
                    }
                }
            }
        }
    }
}
