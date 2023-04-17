Name:		globus-gridmap-verify-myproxy-callout
%global _name %(tr - _ <<< %{name})
Epoch:          1
Version:	2.12
Release:	1%{?dist}
Vendor:	Globus Support
Summary:	Globus Toolkit - Globus gridmap myproxy callout

Group:		System Environment/Libraries
License:	ASL 2.0
URL:           https://www.globus.org/
Source:        https://downloads.globus.org/toolkit/gt6/packages/%{_name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  openssl
%if 0%{?suse_version} == 0
BuildRequires:  openssl-devel
%else
BuildRequires:  libopenssl-1_1-devel
%endif

BuildRequires:	globus-gsi-sysconfig-devel >= 5
BuildRequires:	globus-gss-assist-devel >= 8
BuildRequires:	globus-gridmap-callout-error-devel
BuildRequires:	globus-gssapi-gsi-devel >= 9
BuildRequires:	globus-gsi-credential-devel >= 6
BuildRequires:  automake >= 1.11
BuildRequires:  autoconf >= 2.60
BuildRequires:  libtool >= 2.2
BuildRequires:  pkgconfig

%description
The Globus Toolkit is an open source software toolkit used for building Grid
systems and applications. It is being developed by the Globus Alliance and
many others all over the world. A growing number of projects and companies are
using the Globus Toolkit to unlock the potential of grids for their cause.

The %{name} package contains:
Globus gridmap myproxy callout

%prep
%setup -q -n %{_name}-%{version}

%build
# Remove files that should be replaced during bootstrap
rm -rf autom4te.cache

autoreconf -if

%configure \
           --disable-static \
           --docdir=%{_docdir}/%{name}-%{version} \
           --includedir=%{_includedir}/globus \
           --libexecdir=%{_datadir}/globus

make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

# Remove libtool archives (.la files)
find $RPM_BUILD_ROOT%{_libdir} -name 'lib*.la' -exec rm -v '{}' \;

%check
make %{?_smp_mflags} check

%clean
rm -rf $RPM_BUILD_ROOT

%post %{?nmainpkg}
/sbin/ldconfig

%postun %{?nmainpkg} -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%dir %{_docdir}/%{name}-%{version}
%{_docdir}/%{name}-%{version}/GLOBUS_LICENSE
%config(noreplace) %{_sysconfdir}/gridmap_verify_myproxy_callout-gsi_authz.conf
%{_libdir}/libglobus_*

%changelog
* Wed Dec 01 2021 Globus Toolkit <support@globus.org> - 2.12-1
- Rebuild for new OSes
- Update license text

* Thu Apr 29 2021 Globus Toolkit <support@globus.org> - 2.11-1
- Allow prerelease version tags

* Mon Nov 25 2019 Globus Toolkit <support@globus.org> - 2.10-2
- Packaging update to ensure priority of Globus packages

* Fri May 03 2019 Globus Toolkit <support@globus.org> - 2.10-1
- fix verification to allow proxy certs

* Thu Sep 08 2016 Globus Toolkit <support@globus.org> - 2.9-1
- Update for el.5 openssl101e

* Mon Aug 29 2016 Globus Toolkit <support@globus.org> - 2.8-3
- Updates for SLES 12

* Sat Aug 20 2016 Globus Toolkit <support@globus.org> - 2.8-1
- Update bug report URL

* Thu Aug 06 2015 Globus Toolkit <support@globus.org> - 2.7-2
- Add vendor

* Wed Oct 08 2014 Globus Toolkit <support@globus.org> - 2.7-1
- GT-560: Verify sharing certs

* Tue Sep 30 2014 Globus Toolkit <support@globus.org> - 2.6-1
- Add missing dependencies
- Don't autoconf substitute conf file with no substitutions in it

* Fri Aug 22 2014 Globus Toolkit <support@globus.org> - 2.5-1
- Merge fixes from ellert-globus_6_branch

* Wed Aug 20 2014 Globus Toolkit <support@globus.org> - 2.4-2
- Fix Source path

* Mon Jun 09 2014 Globus Toolkit <support@globus.org> - 2.4-1
- Merge changes from Mattias Ellert

* Fri Apr 18 2014 Globus Toolkit <support@globus.org> - 2.3-1
- Version bump for consistency

* Thu Feb 27 2014 Globus Toolkit <support@globus.org> - 2.2-1
- Packaging fixes, Warning Cleanup

* Sat Feb 15 2014 Globus Toolkit <support@globus.org> - 2.1-1
- Packaging fixes

* Wed Jan 22 2014 Globus Toolkit <support@globus.org> - 2.0-1
- Repackage for GT6 without GPT

* Mon Oct 28 2013 Globus Toolkit <support@globus.org> - 1.5-1
- Update dependencies for new credential/assist functions

* Wed Jun 26 2013 Globus Toolkit <support@globus.org> - 1.3-2
- GT-424: New Fedora Packaging Guideline - no %%_isa in BuildRequires

* Wed Jun 19 2013 Globus Toolkit <support@globus.org> - 1.3-1
- Add GLOBUS_OPENSSL in configure

* Tue Mar 19 2013 Globus Toolkit <support@globus.org> - 1.2-2
- Update sharing to support a full cert chain at logon

* Wed Mar 06 2013 Globus Toolkit <support@globus.org> - 1.2-1
- GT-341: GPT metadata problem in new myproxy callout package

* Tue Mar 05 2013 Globus Toolkit <support@globus.org> - 1.1-1
- GT-365: Switch sharing user identification from DN to CERT

* Mon Aug 13 2012 Joseph Bester <bester@mcs.anl.gov> - 0.1-1
- Autogenerated
