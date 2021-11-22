Name:		myproxy-oauth
%global _name %(tr - _ <<< %{name})
Epoch:          1
Version:	0.30
Release:	4%{?dist}
Vendor:	Globus Support
Summary:	MyProxy OAuth Delegation Serice

Group:		System Environment/Libraries
License:	Globus Connect Community Source Code License Agreement
URL:           https://www.globus.org/
Source:        https://downloads.globus.org/toolkit/gt6/packages/%{_name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildArch:      noarch

%if %{?fedora}%{!?fedora:0} >= 30 || %{?rhel}%{!?rhel:0} >= 8
BuildRequires:	python2
Requires:	python2-pyOpenSSL
Requires:       python2-mod_wsgi
%global		python2 python2
%else
BuildRequires:	python
Requires:	pyOpenSSL
%global		python2 python
%endif

%if 0%{?suse_version} == 0
Requires:       mod_ssl
Requires:       mod_wsgi
Requires(pre): shadow-utils
%else
# Available from http://download.opensuse.org/repositories/Apache/SLE_11_SP3/Apache.repo
Requires:       apache2 >= 2.4
# Available from http://download.opensuse.org/repositories/Apache:/Modules/Apache_SLE_12_SP1/Apache:Modules.repo
Requires:       apache2-mod_wsgi
BuildRequires:   shadow
Requires(pre):   shadow
%endif

%if 0%{?rhel} >= 6 
Requires:       python-crypto
Requires:       m2crypto
%endif

%if 0%{?suse_version} > 0
Requires:       python-crypto
Requires:       python-m2crypto
%endif

%if %{?fedora}%{!?fedora:0} >= 30 || %{?rhel}%{!?rhel:0} >= 8
Requires:       python2-crypto >= 2.2
%endif

%if %{?fedora}%{!?fedora:0} >= 28 && %{?fedora}%{!?fedora:0}  < 30
Requires:       python-crypto >= 2.2
%endif

%description
The Globus Toolkit is an open source software toolkit used for building Grid
systems and applications. It is being developed by the Globus Alliance and
many others all over the world. A growing number of projects and companies are
using the Globus Toolkit to unlock the potential of grids for their cause.

The %{name} package contains:
MyProxy OAuth Delegation Service

%prep
%setup -q -n %{_name}-%{version}

%build
:

%install
rm -rf $RPM_BUILD_ROOT
%{python2} setup.py install \
    --install-lib /usr/share/%{name} \
    --install-scripts /usr/share/%{name} \
    --install-data %{_docdir}/%{name} \
    --root $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/%{_docdir}/%{name}
cp README.md $RPM_BUILD_ROOT%{_docdir}/%{name}/README.txt
mkdir -p $RPM_BUILD_ROOT/%{_sbindir}
pythonpath="/usr/share/%{name}"
cat > $RPM_BUILD_ROOT%{_sbindir}/myproxy-oauth-setup <<EOF 
#! /bin/sh
if [ "\$(id -u)" = 0 ]; then
    idarg="-i \$(id -u myproxyoauth)"
fi
exec /usr/bin/env PYTHONPATH="$pythonpath" %{python2} /usr/share/%{name}/myproxy-oauth-setup "\$@" \${idarg}
EOF
chmod a+x $RPM_BUILD_ROOT%{_sbindir}/myproxy-oauth-setup
%if 0%{?fedora} >= 18 || 0%{?rhel} >= 7
mkdir -p $RPM_BUILD_ROOT/etc/httpd/conf.d
cp $RPM_BUILD_ROOT%{_docdir}/%{name}/apache/myproxy-oauth-2.4 \
   $RPM_BUILD_ROOT/etc/httpd/conf.d/wsgi-myproxy-oauth.conf 
%else
%if 0%{?suse_version} > 0
mkdir -p $RPM_BUILD_ROOT/etc/apache2/conf.d
cp $RPM_BUILD_ROOT%{_docdir}/%{name}/apache/myproxy-oauth-2.4 \
   $RPM_BUILD_ROOT/etc/apache2/conf.d/wsgi-myproxy-oauth.conf 

%else
mkdir -p $RPM_BUILD_ROOT/etc/httpd/conf.d
cp $RPM_BUILD_ROOT%{_docdir}/%{name}/apache/myproxy-oauth \
   $RPM_BUILD_ROOT/etc/httpd/conf.d/wsgi-myproxy-oauth.conf 
%endif
%endif

mkdir -p "$RPM_BUILD_ROOT/var/lib/myproxy-oauth"

%pre
getent group myproxyoauth >/dev/null || groupadd -r myproxyoauth
getent passwd myproxyoauth >/dev/null || \
    useradd -r -g myproxyoauth -d /usr/share/myproxy-oauth -s /sbin/nologin \
        -c "MyProxy Oauth Daemon" myproxyoauth

%if 0%{?suse_version} != 0
mkdir -p /srv/www/run
%endif

exit 0
%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%if %{?suse_version}%{!?suse_version:0} >= 1315
%dir %{_sysconfdir}/apache2
%dir %{_sysconfdir}/apache2/conf.d
%dir %{_docdir}/%{name}
%dir %{_docdir}/%{name}/apache
%endif
%doc %{_docdir}/%{name}/README.txt
%doc %{_docdir}/%{name}/apache/*
%config(noreplace) /etc/*/conf.d/wsgi-myproxy-oauth.conf
%dir %attr(0700,myproxyoauth,myproxyoauth) /var/lib/myproxy-oauth
/usr/share/%{name}

%{_sbindir}/myproxy-oauth-setup

%changelog
* Mon Nov 25 2019 Globus Toolkit <support@globus.org> - 0.30-3
- Packaging update to ensure priority of Globus packages

* Fri May 10 2019 Globus Toolkit <support@globus.org> - 0.30-2
- Explicitly py2

* Mon May 14 2018 Globus Toolkit <support@globus.org> - 0.29-1
- Allow newer TLS versions

* Tue May 8 2018 Globus Toolkit <support@globus.org> - 0.28-1
- Resolve packaging issue                                                      

* Mon Dec 18 2017 Globus Toolkit <support@globus.org> - 0.27-1
- Revert

* Mon Mar 13 2017 Globus Toolkit <support@globus.org> - 0.26-1
- Move apache config file deployment into setup script

* Thu Nov 10 2016 Globus Toolkit <support@globus.org> - 0.25-1
- Python exception handling workaround for 2.5-3.x

* Wed Nov 09 2016 Globus Toolkit <support@globus.org> - 0.24-1
- Verify all paths yield proper response or error response

* Fri Nov 04 2016 Globus Toolkit <support@globus.org> - 0.23-1
- Fix indent issue

* Tue Oct 18 2016 Globus Toolkit <support@globus.org> - 0.22-1
- Catch exceptions and return "400 Bad Request"

* Wed Aug 31 2016 Globus Toolkit <support@globus.org> - 0.21-4
- Updates for SLES 12

* Thu Mar 10 2016 Globus Toolkit <support@globus.org> - 0.21-1
- Fix redirect when callback_uri contains a query

* Thu Oct 29 2015 Globus Toolkit <support@globus.org> - 0.20-1
- Use setsebool if it is installed and semanage is not available

* Tue Nov 11 2014 Globus Toolkit <support@globus.org> - 0.18-1
- Run selinux commands as root, run database commands as myproxyoauth

* Wed Nov 05 2014 Globus Toolkit <support@globus.org> - 0.16-1
- Remove httplib2 dependent code which is not used

* Mon Aug 04 2014 Globus Toolkit <support@globus.org> - 0.15-2
- Fix error in scriptlet to create wsgi socket dir on SLES 11

* Thu Jul 31 2014 Globus Toolkit <support@globus.org> - 0.15-1
- Update to 0.15 for EC2-public hostname awareness
- Create wsgi socket dir on SLES 11

* Tue Jul 29 2014 Globus Toolkit <support@globus.org> - 0.14-3
- EL7 requires Apache 2.4 configuration file

* Fri Jul 25 2014 Globus Toolkit <support@globus.org> - 0.14-2
- EL7 doesn't require python-sqlite2

* Mon Jan 20 2014 Globus Toolkit <support@globus.org> - 0.14-1
- move to globus repo

* Wed Sep 04 2013 Globus Toolkit <support@globus.org> - 0.13-1
- Fix regression on python path setting

* Wed Sep 04 2013 Globus Toolkit <support@globus.org> - 0.12-1
- Fall back to pysqlite2 when sqlite3 is not available

* Wed Sep 04 2013 Globus Toolkit <support@globus.org> - 0.11-1
- Remove dependency on sql alchemy

* Fri Aug 23 2013 Globus Toolkit <support@globus.org> - 0.10-1
- Remove dependency on jinja2

* Wed Mar 27 2013 Globus Toolkit <support@globus.org> - 0.0-1
- Initial packaging
