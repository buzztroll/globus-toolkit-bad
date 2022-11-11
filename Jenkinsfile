#! groovy

import groovy.transform.Field

@Library('gcs-build-scripts') _

@Field
def ALL_TAGS = null

def get_params(src_dir) {
    def tag
    def change_branch = env.CHANGE_BRANCH ?: ""
    def branch_name = env.BRANCH_NAME ?: ""
    def change_id = env.CHANGE_ID ?: ""
    def package_name = STAGE_NAME

    if (params.NEWEST_TAG) {
        // When choosing a tag, override the branch/change info from this task
        // in the build of the ToolkitPackage task
        tag = findHighestTag("^${package_name}-(?<version>[0-9\\.]+)(-(?<release>[0-9]+))?\$", ALL_TAGS)
        change_branch = ""
        change_id = ""
        branch_name = tag
    } else {
        tag = env.TAG_NAME ?: ""
    }

    return [
        string(name: 'CHANGE_BRANCH', value: change_branch),
        string(name: 'CHANGE_ID', value: change_id),
        string(name: 'BRANCH_NAME', value: branch_name),
        string(name: 'EPIC', value: env.EPIC ?: ""),
        string(name: 'GIT_URL', value: env.GIT_URL ?: ""),
        string(name: 'TAG_NAME', value: tag),
        string(name: 'SOURCE_DIR', value: src_dir),
        string(name: 'STABLE_TAG', value: '${PACKAGE_NAME}-${PACKAGE_VERSION}')
    ]
}

def load_tags() {
    ALL_TAGS = enumerateTags(env.GIT_URL)
}

def create_repo_package() {
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

def publish_repo_packages() {
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
    env.STABLE_TAG = '${PACKAGE_NAME}-${PACKAGE_VERSION}'
    publishResults(
        stashname,
        env.GLOBUS_REPO_PKG,
        env.GLOBUS_REPO_VERSION,
        false)
}

@Field
def ordered_stage_names = [
    "",
    "globus_common",
    "globus_xio",
    "globus_gfork",
    "globus_gsi_openssl_error",
    "globus_gsi_proxy_ssl",
    "globus_openssl_module",
    "globus_gsi_cert_utils",
    "globus_gsi_sysconfig",
    "globus_callout",
    "globus_gsi_callback",
    "globus_gsi_credential",
    "globus_gsi_proxy_core",
    "globus_gssapi_gsi",
    "globus_gss_assist",
    "globus_gssapi_error",
    "globus_xio_gsi_driver",
    "globus_io",
    "globus_ftp_control",
    "globus_usage",
    "globus_xio_pipe_driver",
    "globus_gridftp_server_control",
    "globus_authz_callout_error",
    "globus_authz",
    "globus_xio_rate_driver",
    "globus_xio_popen_driver",
    "globus_xio_udt_driver",
    "globus_net_manager",
    "globus_gridftp_server",
    "globus_gass_transfer",
    "globus_ftp_client",
    "globus_gass_copy",
    "globus_xio_gridftp_multicast",
    "globus_simple_ca",
    "globus_gridmap_callout_error",
    "globus_gridmap_callout",
    "globus_xio_gridftp_driver",
    "globus_gridmap_verify_myproxy_callout",
    "globus_gridmap_eppn_callout",
    "globus_proxy_utils",
    "globus_xioperf",
    "myproxy",
    "globus_repo"
]

def checkout_source() {
    def scmVars = checkout scm
    env.GIT_URL = "${scmVars.GIT_URL}"
    sh '''
    git clean -fdx
    mkdir -p m4
    autoreconf -i --no-recursive
    sh ./write-globus-version "$(cat globus-version.inc)"
    ./update-dirt.sh
    '''

    if (params.NEWEST_TAG) {
        load_tags()
    }
}

def buildit() {
    def stage_dirs = [
        "": "",
        "globus_common": "common/source",
        "globus_xio": "xio/src",
        "globus_gfork": "gridftp/gfork/source",
        "globus_gsi_openssl_error": "gsi/openssl_error/source",
        "globus_gsi_proxy_ssl": "gsi/proxy/proxy_ssl/source",
        "globus_openssl_module": "gsi/openssl_module/source",
        "globus_gsi_cert_utils": "gsi/cert_utils/source",
        "globus_gsi_sysconfig": "gsi/sysconfig/source",
        "globus_callout": "callout/source/",
        "globus_gsi_callback": "gsi/callback/source",
        "globus_gsi_credential": "gsi/credential/source",
        "globus_gsi_proxy_core": "gsi/proxy/proxy_core/source",
        "globus_gssapi_gsi": "gsi/gssapi/source",
        "globus_gss_assist": "gsi/gss_assist/source",
        "globus_gssapi_error": "gsi/gssapi_error/source",
        "globus_xio_gsi_driver": "xio/drivers/gsi",
        "globus_io": "io/compat",
        "globus_ftp_control": "gridftp/control/source",
        "globus_usage": "usage/c/sender/source",
        "globus_xio_pipe_driver": "xio/drivers/pipe/source",
        "globus_gridftp_server_control": "gridftp/server-lib/src",
        "globus_authz_callout_error": "gsi/authz/error",
        "globus_authz": "gsi/authz/source",
        "globus_xio_rate_driver": "xio/drivers/rate/source",
        "globus_xio_popen_driver": "xio/drivers/popen/source",
        "globus_xio_udt_driver": "xio/drivers/udt/source",
        "globus_net_manager": "gridftp/net_manager",
        "globus_gridftp_server": "gridftp/server/src",
        "globus_gass_transfer": "gass/transfer/source",
        "globus_ftp_client": "gridftp/client/source",
        "globus_gass_copy": "gass/copy/source",
        "globus_xio_gridftp_multicast": "gridftp/server/multi/source",
        "globus_simple_ca": "gsi/simple_ca/source",
        "globus_gridmap_callout_error": "gsi/gridmap_callout/error",
        "globus_gridmap_callout": "gsi/gridmap_callout/source",
        "globus_xio_gridftp_driver": "gridftp/gridftp_driver/source",
        "globus_gridmap_verify_myproxy_callout": "gsi/gridmap_verify_myproxy_callout",
        "globus_gridmap_eppn_callout": "gsi/gridmap_eppn_callout",
        "globus_proxy_utils": "gsi/proxy/proxy_utils/source",
        "globus_xioperf": "xio/xioperf/source",
        "myproxy": "myproxy/source",
        "globus_repo": "packaging/debian/globus-repo"
    ]

    build(job: 'ToolkitPackage-fall2022', parameters: get_params(stage_dirs[STAGE_NAME]))
}

def not_before_restart() {
    restart_point = params.RESTART_POINT
    if (restart_point) {
        for (stage in ordered_stage_names) {
            if (stage == restart_point) {
                // If we hit the restart point before the current stage name,
                // this stage is after the restart point
                return true
            } else if (stage == STAGE_NAME) {
                // If we hit the current stage name, then it is before the
                // restart point, so we won't run it
                return false
            }
        }
    }
    return true
}

pipeline {
    agent none
    parameters {
        booleanParam(
            name: 'ALL_PACKAGES',
            defaultValue: false
        )
        booleanParam(
            name: 'NEWEST_TAG',
            defaultValue: false
        )
        booleanParam(
            name: 'COMMON',
            defaultValue: false
        )
        booleanParam(
            name: 'XIO',
            defaultValue: false
        )
        booleanParam(
            name: 'GFORK',
            defaultValue: false
        )
        booleanParam(
            name: 'OPENSSL_ERROR',
            defaultValue: false
        )
        booleanParam(
            name: 'GSI_PROXY_SSL',
            defaultValue: false
        )
        booleanParam(
            name: 'OPENSSL_MODULE',
            defaultValue: false
        )
        booleanParam(
            name: "GSI_CERT_UTILS",
            defaultValue: false
        )
        booleanParam(
            name: "GSI_SYSCONFIG",
            defaultValue: false
        )
        booleanParam(
            name: "CALLOUT",
            defaultValue: false
        )
        booleanParam(
            name: "GSI_CALLBACK",
            defaultValue: false
        )
        booleanParam(
            name: "GSI_CREDENTIAL",
            defaultValue: false
        )
        booleanParam(
            name: "GSI_PROXY_CORE",
            defaultValue: false
        )
        booleanParam(
            name: "GSSAPI_GSI",
            defaultValue: false
        )
        booleanParam(
            name: "GSS_ASSIST",
            defaultValue: false
        )
        booleanParam(
            name: "GSSAPI_ERROR",
            defaultValue: false
        )
        booleanParam(
            name: "XIO_GSI_DRIVER",
            defaultValue: false
        )
        booleanParam(
            name: "IO",
            defaultValue: false
        )
        booleanParam(
            name: "FTP_CONTROL",
            defaultValue: false
        )
        booleanParam(
            name: "USAGE",
            defaultValue: false
        )
        booleanParam(
            name: "XIO_PIPE_DRIVER",
            defaultValue: false
        )
        booleanParam(
            name: "GRIDFTP_SERVER_CONTROL",
            defaultValue: false
        )
        booleanParam(
            name: "AUTHZ_CALLOUT_ERROR",
            defaultValue: false
        )
        booleanParam(
            name: "AUTHZ",
            defaultValue: false
        )
        booleanParam(
            name: "XIO_RATE_DRIVER",
            defaultValue: false
        )
        booleanParam(
            name: "XIO_POPEN_DRIVER",
            defaultValue: false
        )
        booleanParam(
            name: "XIO_UDT_DRIVER",
            defaultValue: false
        )
        booleanParam(
            name: "NET_MANAGER",
            defaultValue: false
        )
        booleanParam(
            name: "GRIDFTP_SERVER",
            defaultValue: false
        )
        booleanParam(
            name: "GASS_TRANSFER",
            defaultValue: false
        )
        booleanParam(
            name: "FTP_CLIENT",
            defaultValue: false
        )
        booleanParam(
            name: "GASS_COPY",
            defaultValue: false
        )
        booleanParam(
            name: "XIO_GRIDFTP_MULTICAST",
            defaultValue: false
        )
        booleanParam(
            name: "SIMPLE_CA",
            defaultValue: false
        )
        booleanParam(
            name: "GRIDMAP_CALLOUT_ERROR",
            defaultValue: false
        )
        booleanParam(
            name: "GRIDMAP_CALLOUT",
            defaultValue: false
        )
        booleanParam(
            name: "XIO_GRIDFTP_DRIVER",
            defaultValue: false
        )
        booleanParam(
            name: "GRIDMAP_VERIFY_MYPROXY_CALLOUT",
            defaultValue: false
        )
        booleanParam(
            name: "GRIDMAP_EPPN_CALLOUT",
            defaultValue: false
        )
        booleanParam(
            name: "PROXY_UTILS",
            defaultValue: false
        )
        booleanParam(
            name: "XIOPERF",
            defaultValue: false
        )
        booleanParam(
            name: "MYPROXY",
            defaultValue: false
        )
        booleanParam(
            name: "REPO",
            defaultValue: false
        )
        choice(
            name: "RESTART_POINT",
            choices: ordered_stage_names,
            description: """
                Package name of the first package to build in dependency order. If
                omitted, this task will build all checked packages (or all packages if
                the ALL_PACKAGES option is selected).

                If set, then it will only build packages that are checked and
                after the named package.
            """
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
                    checkout_source()
                }
            }
        }
        stage ("globus_common") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.COMMON;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "common/**/*";
                        changeset "packaging/debian/globus-common/**/*";
                        changeset "packaging/fedora/globus-common.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_xio") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.XIO;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "xio/src/**/*";
                        changeset "packaging/debian/globus-xio/**/*";
                        changeset "packaging/fedora/globus-xio.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_gfork") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.GFORK;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "gridftp/gfork/source/**/*";
                        changeset "packaging/debian/globus-gfork/**/*";
                        changeset "packaging/fedora/globus-gfork.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_gsi_openssl_error") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.OPENSSL_ERROR;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "gsi/openssl_error/source/**/*";
                        changeset "packaging/debian/globus-gsi-openssl-error/**/*";
                        changeset "packaging/fedora/globus-gsi-openssl-error.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_gsi_proxy_ssl") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.GSI_PROXY_SSL;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "gsi/proxy/proxy_ssl/source/**/*";
                        changeset "packaging/debian/globus-gsi-proxy-ssl/**/*";
                        changeset "packaging/fedora/globus-gsi-proxy-ssl.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_openssl_module") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.OPENSSL_MODULE;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "gsi/openssl_module/source/**/*";
                        changeset "packaging/debian/globus-openssl-module/**/*";
                        changeset "packaging/fedora/globus-openssl-module.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_gsi_cert_utils") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.GSI_CERT_UTILS;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "gsi/cert_utils/source/**/*";
                        changeset "packaging/debian/globus-gsi-cert-utils/**/*";
                        changeset "packaging/fedora/globus-gsi-cert-utils.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_gsi_sysconfig") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.GSI_SYSCONFIG;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "gsi/sysconfig/source/**/*";
                        changeset "packaging/debian/globus-gsi-sysconfig/**/*";
                        changeset "packaging/fedora/globus-gsi-sysconfig.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_callout") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.CALLOUT;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "callout/source/**/*";
                        changeset "packaging/debian/globus-callout/**/*";
                        changeset "packaging/fedora/globus-callout.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_gsi_callback") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.GSI_CALLBACK;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "gsi/callback/source/**/*";
                        changeset "packaging/debian/globus-gsi-callback/**/*";
                        changeset "packaging/fedora/globus-gsi-callback.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_gsi_credential") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.GSI_CREDENTIAL;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "gsi/credential/source/**/*";
                        changeset "packaging/debian/globus-gsi-credential/**/*";
                        changeset "packaging/fedora/globus-gsi-credential.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_gsi_proxy_core") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.GSI_PROXY_CORE;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "gsi/proxy/proxy_core/source/**/*";
                        changeset "packaging/debian/globus-gsi-proxy-core/**/*";
                        changeset "packaging/fedora/globus-gsi-proxy-core.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_gssapi_gsi") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.GSSAPI_GSI;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "gsi/gssapi/source/**/*";
                        changeset "packaging/debian/globus-gssapi-gsi/**/*";
                        changeset "packaging/fedora/globus-gssapi-gsi.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_gss_assist") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.GSS_ASSIST;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "gsi/gss_assist/source/**/*";
                        changeset "packaging/debian/globus-gss-assist/**/*";
                        changeset "packaging/fedora/globus-gss-assist.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_gssapi_error") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.GSSAPI_ERROR;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "gsi/gssapi_error/source/**/*";
                        changeset "packaging/debian/globus-gssapi-error/**/*";
                        changeset "packaging/fedora/globus-gssapi-error.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_xio_gsi_driver") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.XIO_GSI_DRIVER;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "xio/drivers/gsi/**/*";
                        changeset "packaging/debian/globus-xio-gsi-driver/**/*";
                        changeset "packaging/fedora/globus-xio-gsi-driver.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_io") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.IO;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "io/compat/**/*";
                        changeset "packaging/debian/globus-io/**/*";
                        changeset "packaging/fedora/globus-io.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_ftp_control") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.FTP_CONTROL;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "gridftp/control/source/**/*";
                        changeset "packaging/debian/globus-ftp-control/**/*";
                        changeset "packaging/fedora/globus-ftp-control.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_usage") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.USAGE;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "usage/c/sender/source/**/*";
                        changeset "packaging/debian/globus-usage/**/*";
                        changeset "packaging/fedora/globus-usage.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_xio_pipe_driver") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.XIO_PIPE_DRIVER;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "xio/drivers/pipe/source/**/*";
                        changeset "packaging/debian/globus-xio-pipe-driver/**/*";
                        changeset "packaging/fedora/globus-xio-pipe-driver.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_gridftp_server_control") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.GRIDFTP_SERVER_CONTROL;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "gridftp/server-lib/src/**/*";
                        changeset "packaging/debian/globus-gridftp-server-control/**/*";
                        changeset "packaging/fedora/globus-gridftp-server-control.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_authz_callout_error") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.AUTHZ_CALLOUT_ERROR;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "gsi/authz/error/**/*";
                        changeset "packaging/debian/globus-authz-callout-error/**/*";
                        changeset "packaging/fedora/globus-authz-callout-error.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_authz") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.AUTHZ;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "gsi/authz/source/**/*";
                        changeset "packaging/debian/globus-authz/**/*";
                        changeset "packaging/fedora/globus-authz.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_xio_rate_driver") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.XIO_RATE_DRIVER;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "xio/drivers/rate/source/**/*";
                        changeset "packaging/debian/globus-xio-rate-driver/**/*";
                        changeset "packaging/fedora/globus-xio-rate-driver.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_xio_popen_driver") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.XIO_POPEN_DRIVER;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "xio/drivers/popen/source/**/*";
                        changeset "packaging/debian/globus-xio-popen-driver/**/*";
                        changeset "packaging/fedora/globus-xio-popen-driver.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_xio_udt_driver") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.XIO_UDT_DRIVER;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "xio/drivers/udt/source/**/*";
                        changeset "packaging/debian/globus-xio-udt-driver/**/*";
                        changeset "packaging/fedora/globus-xio-udt-driver.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_net_manager") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.NET_MANAGER;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "gridftp/net_manager/**/*";
                        changeset "packaging/debian/globus-net-manager/**/*";
                        changeset "packaging/fedora/globus-net-manager.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_gridftp_server") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.GRIDFTP_SERVER;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "gridftp/server/src/**/*";
                        changeset "packaging/debian/globus-gridftp-server/**/*";
                        changeset "packaging/fedora/globus-gridftp-server.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_gass_transfer") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.GASS_TRANSFER;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "gass/transfer/source/**/*";
                        changeset "packaging/debian/globus-gass-transfer/**/*";
                        changeset "packaging/fedora/globus-gass-transfer.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_ftp_client") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.FTP_CLIENT;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "gridftp/client/source/**/*";
                        changeset "packaging/debian/globus-ftp-client/**/*";
                        changeset "packaging/fedora/globus-ftp-client.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_gass_copy") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.GASS_COPY;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "gass/copy/source";
                        changeset "packaging/debian/globus-gass-copy/**/*";
                        changeset "packaging/fedora/globus-gass-copy.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_xio_gridftp_multicast") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.XIO_GRIDFTP_MULTICAST;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "gridftp/server/multi/source/**/*";
                        changeset "packaging/debian/globus-xio-gridftp-multicast/**/*";
                        changeset "packaging/fedora/globus-xio-gridftp-multicast.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_simple_ca") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.SIMPLE_CA;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "gsi/simple_ca/source/**/*";
                        changeset "packaging/debian/globus-simple-ca/**/*";
                        changeset "packaging/fedora/globus-simple-ca.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_gridmap_callout_error") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.GRIDMAP_CALLOUT_ERROR;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "gsi/gridmap_callout/error/**/*";
                        changeset "packaging/debian/globus-gridmap-callout-error/**/*";
                        changeset "packaging/fedora/globus-gridmap-callout-error.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_gridmap_callout") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.GRIDMAP_CALLOUT;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "gsi/gridmap_callout/source/**/*";
                        changeset "packaging/debian/globus-gridmap-callout/**/*";
                        changeset "packaging/fedora/globus-gridmap-callout.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_xio_gridftp_driver") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.XIO_GRIDFTP_DRIVER;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "gridftp/gridftp_driver/source/**/*";
                        changeset "packaging/debian/globus-xio-gridftp-driver/**/*";
                        changeset "packaging/fedora/globus-xio-gridftp-driver.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_gridmap_verify_myproxy_callout") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.GRIDMAP_VERIFY_MYPROXY_CALLOUT;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "gsi/gridmap_verify_myproxy_callout/**/*";
                        changeset "packaging/debian/globus-gridmap-verify-myproxy-callout/**/*";
                        changeset "packaging/fedora/globus-gridmap-verify-myproxy-callout.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_gridmap_eppn_callout") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.GRIDMAP_EPPN_CALLOUT;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "gsi/gridmap_eppn_callout/**/*";
                        changeset "packaging/debian/globus-gridmap-eppn-callout/**/*";
                        changeset "packaging/fedora/globus-gridmap-eppn-callout.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_proxy_utils") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.PROXY_UTILS;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "gsi/proxy/proxy_utils/source/**/*";
                        changeset "packaging/debian/globus-proxy-utils/**/*";
                        changeset "packaging/fedora/globus-proxy-utils.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_xioperf") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.XIOPERF;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "xio/xioperf/source/**/*";
                        changeset "packaging/debian/globus-xioperf/**/*";
                        changeset "packaging/fedora/globus-xioperf.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("myproxy") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.MYPROXY;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "myproxy/source/**/*";
                        changeset "packaging/debian/myproxy/**/*";
                        changeset "packaging/fedora/myproxy.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                buildit()
            }
        }
        stage ("globus_repo") {
            when {
                allOf {
                    anyOf {
                        equals expected: true, actual: params.REPO;
                        equals expected: true, actual: params.ALL_PACKAGES;
                        changeset "packaging/debian/globus-repo/**/*";
                        changeset "packaging/fedora/globus-repo.spec";
                    }
                    expression { return not_before_restart() }
                }
            }
            steps {
                script {
                    node (label: "debian_package_builder") {
                        create_repo_package()
                    }
                    node (label: "master") {
                        publish_repo_packages()
                    }
                }
            }
        }
    }
}
