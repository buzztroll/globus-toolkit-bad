Name:		globus-gsi-sysconfig
%global soname 1
%global _name %(tr - _ <<< %{name})
Epoch:          1
Version:	8.5~rc1
Release:	1%{?dist}
Vendor:	Globus Support
Summary:	Globus Toolkit - Globus GSI System Config Library

Group:		System Environment/Libraries
License:	ASL 2.0
URL:           https://www.globus.org/
Source:        https://downloads.globus.org/toolkit/gt6/packages/%{_name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:	globus-common-devel >= 15
BuildRequires:	globus-openssl-module-devel >= 3
BuildRequires:	globus-gsi-openssl-error-devel >= 2
BuildRequires:	automake >= 1.11
BuildRequires:	autoconf >= 2.60
BuildRequires:	libtool >= 2.2
BuildRequires:  pkgconfig

BuildRequires:  openssl
BuildRequires:  openssl-devel

%package devel
Summary:	Globus Toolkit - Globus GSI System Config Library Development Files
Group:		Development/Libraries
Requires:	%{name}%{?_isa} = %{epoch}:%{version}-%{release}
Requires:	globus-common-devel%{?_isa} >= 15
Requires:	globus-openssl-module-devel%{?_isa} >= 3
Requires:	globus-gsi-openssl-error-devel%{?_isa} >= 2
Requires:  openssl
Requires:  openssl-devel

%package doc
Summary:	Globus Toolkit - Globus GSI System Config Library Documentation Files
Group:		Documentation
BuildArch:	noarch
Requires:	%{name} = %{epoch}:%{version}-%{release}

%description
The Globus Toolkit is an open source software toolkit used for building Grid
systems and applications. It is being developed by the Globus Alliance and
many others all over the world. A growing number of projects and companies are
using the Globus Toolkit to unlock the potential of grids for their cause.

The %{name} package contains:
Globus GSI System Config Library

%description devel
The Globus Toolkit is an open source software toolkit used for building Grid
systems and applications. It is being developed by the Globus Alliance and
many others all over the world. A growing number of projects and companies are
using the Globus Toolkit to unlock the potential of grids for their cause.

The %{name}-devel package contains:
Globus GSI System Config Library Development Files

%description doc
The Globus Toolkit is an open source software toolkit used for building Grid
systems and applications. It is being developed by the Globus Alliance and
many others all over the world. A growing number of projects and companies are
using the Globus Toolkit to unlock the potential of grids for their cause.

The %{name}-doc package contains:
Globus GSI System Config Library Documentation Files

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
mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}/grid-security/certificates

# Remove libtool archives (.la files)
find $RPM_BUILD_ROOT%{_libdir} -name 'lib*.la' -exec rm -v '{}' \;

%clean
rm -rf $RPM_BUILD_ROOT

%post %{?nmainpkg} -p /sbin/ldconfig

%postun %{?nmainpkg} -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%dir %{_docdir}/%{name}-%{version}
%dir %{_sysconfdir}/grid-security
%dir %{_sysconfdir}/grid-security/certificates
%doc %{_docdir}/%{name}-%{version}/GLOBUS_LICENSE
%{_libdir}/libglobus*.so.*

%files devel
%defattr(-,root,root,-)
%{_includedir}/globus/*
%{_libdir}/libglobus*.so
%{_libdir}/pkgconfig/*.pc

%files doc
%defattr(-,root,root,-)
%dir %{_docdir}/%{name}-%{version}/html
%{_docdir}/%{name}-%{version}/html/*
%{_mandir}/man3/*

%changelog
* Thu Nov 18 2021 Globus Toolkit <support@globus.org> - 8.5~rc1-1
- Prerelease build

* Thu Apr 29 2021 Globus Toolkit <support@globus.org> - 8.4-1
- Allow prerelease version tags

* Fri Mar 26 2021 Globus Toolkit <support@globus.org> - 8.3-1
- Use prebuilt doxyxgen if available

* Fri Mar 26 2021 Globus Toolkit <support@globus.org> - 8.2-1
- Fix test to run in chroot without homedir

* Wed Nov 27 2019 Globus Toolkit <support@globus.org> - 8.1-4
- Packaging update to ensure priority of Globus packages

* Thu Jul 26 2018 Globus Toolkit <support@globus.org> - 8.1-2
- Package /etc/grid-security/certificates

* Wed Jan 24 2018 Globus Toolkit <support@globus.org> - 8.1-1
- fix typo in windows function name

* Wed Nov 01 2017 Globus Toolkit <support@globus.org> - 8.0-1
- Add cert and key checks based on different uid

* Wed Sep 06 2017 Globus Toolkit <support@globus.org> - 7.1-1
- Enable c99 compiler syntax

* Tue Sep 05 2017 Globus Toolkit <support@globus.org> - 7.0-1
- Add SNI vhost cred dir support

* Thu Sep 08 2016 Globus Toolkit <support@globus.org> - 6.11-1
- Update for el.5 openssl101e

* Thu Aug 25 2016 Globus Toolkit <support@globus.org> - 6.10-6
- Updates for SLES 12

* Tue Aug 16 2016 Globus Toolkit <support@globus.org> - 6.10-1
- Updates for OpenSSL 1.1.0

* Wed Nov 25 2015 Globus Toolkit <support@globus.org> - 6.9-1
- Remove @} without matching @{

* Thu Aug 06 2015 Globus Toolkit <support@globus.org> - 6.8-2
- Add vendor

* Thu Sep 25 2014 Globus Toolkit <support@globus.org> - 6.8-1
- Include more manpages for API
- Doxygen markup fixes
- Fix typos and clarify some documentation
- Quiet some autoconf/automake warnings

* Fri Aug 22 2014 Globus Toolkit <support@globus.org> - 6.7-1
- Merge fixes from ellert-globus_6_branch

* Wed Aug 20 2014 Globus Toolkit <support@globus.org> - 6.6-2
- Fix Source path

* Mon Jun 09 2014 Globus Toolkit <support@globus.org> - 6.6-1
- Merge changes from Mattias Ellert

* Tue May 27 2014 Globus Toolkit <support@globus.org> - 6.5-1
- Use package-named config.h

* Fri Apr 18 2014 Globus Toolkit <support@globus.org> - 6.4-1
- Version bump for consistency

* Fri Apr 18 2014 Globus Toolkit <support@globus.org> - 6.3-1
- Version bump for consistency

* Thu Feb 27 2014 Globus Toolkit <support@globus.org> - 6.2-1
- Packaging fixes, Warning Cleanup

* Mon Feb 10 2014 Globus Toolkit <support@globus.org> - 6.1-1
- Packaging fixes

* Tue Jan 21 2014 Globus Toolkit <support@globus.org> - 6.0-1
- Repackage for GT6 without GPT

* Wed Jun 26 2013 Globus Toolkit <support@globus.org> - 5.3-6
- GT-424: New Fedora Packaging Guideline - no %%_isa in BuildRequires

* Wed Feb 20 2013 Globus Toolkit <support@globus.org> - 5.3-5
- Workaround missing F18 doxygen/latex dependency

* Mon Nov 26 2012 Globus Toolkit <support@globus.org> - 5.3-4
- 5.2.3

* Mon Jul 16 2012 Joseph Bester <bester@mcs.anl.gov> - 5.3-3
- GT 5.2.2 final

* Fri Jun 29 2012 Joseph Bester <bester@mcs.anl.gov> - 5.3-2
- GT 5.2.2 Release

* Tue May 15 2012 Joseph Bester <bester@mcs.anl.gov> - 5.3-1
- GT-149 Memory leaks in globus-job-manager
- GT-188: gsi sysconfig leaves internal results in the error cache

* Wed May 09 2012 Joseph Bester <bester@mcs.anl.gov> - 5.2-3
- RHEL 4 patches

* Fri May 04 2012 Joseph Bester <bester@mcs.anl.gov> - 5.2-2
- SLES 11 patches

* Tue Feb 14 2012 Joseph Bester <bester@mcs.anl.gov> - 5.2-1
- RIC-224: Eliminate some doxygen warnings
- RIC-226: Some dependencies are missing in GPT metadata

* Mon Dec 05 2011 Joseph Bester <bester@mcs.anl.gov> - 5.1-4
- Update for 5.2.0 release

* Mon Dec 05 2011 Joseph Bester <bester@mcs.anl.gov> - 5.1-3
- Last sync prior to 5.2.0

* Tue Oct 11 2011 Joseph Bester <bester@mcs.anl.gov> - 5.1-2
- Add explicit dependencies on >= 5.2 libraries

* Thu Oct 06 2011 Joseph Bester <bester@mcs.anl.gov> - 5.1-1
- Add backward-compatibility aging

* Thu Sep 01 2011 Joseph Bester <bester@mcs.anl.gov> - 5.0-3
- Fix missing whitespace in Requires

* Thu Sep 01 2011 Joseph Bester <bester@mcs.anl.gov> - 5.0-2
- Update for 5.1.2 release

* Wed Apr 14 2010 Mattias Ellert <mattias.ellert@fysast.uu.se> - 3.1-1
- Update to Globus Toolkit 5.0.1
- Drop patch globus-gsi-sysconfig-doxygen.patch (fixed upstream)

* Fri Jan 22 2010 Mattias Ellert <mattias.ellert@fysast.uu.se> - 3.0-1
- Update to Globus Toolkit 5.0.0

* Fri Aug 21 2009 Tomas Mraz <tmraz@redhat.com> - 2.2-4
- rebuilt with new openssl

* Thu Jul 23 2009 Mattias Ellert <mattias.ellert@fysast.uu.se> - 2.2-3
- Add instruction set architecture (isa) tags
- Make doc subpackage noarch

* Wed Jun 03 2009 Mattias Ellert <mattias.ellert@fysast.uu.se> - 2.2-2
- Update to official Fedora Globus packaging guidelines

* Thu Apr 16 2009 Mattias Ellert <mattias.ellert@fysast.uu.se> - 2.2-1
- Make comment about source retrieval more explicit
- Change defines to globals
- Remove explicit requires on library packages
- Put GLOBUS_LICENSE file in extracted source tarball

* Sun Mar 15 2009 Mattias Ellert <mattias.ellert@fysast.uu.se> - 2.2-0.5
- Adapting to updated globus-core package

* Thu Feb 26 2009 Mattias Ellert <mattias.ellert@fysast.uu.se> - 2.2-0.4
- Add s390x to the list of 64 bit platforms

* Thu Jan 01 2009 Mattias Ellert <mattias.ellert@fysast.uu.se> - 2.2-0.3
- Adapt to updated GPT package

* Tue Oct 14 2008 Mattias Ellert <mattias.ellert@fysast.uu.se> - 2.2-0.2
- Update to Globus Toolkit 4.2.1

* Mon Jul 14 2008 Mattias Ellert <mattias.ellert@fysast.uu.se> - 2.1-0.1
- Autogenerated
