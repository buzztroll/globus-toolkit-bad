Name:		globus-gass-copy
%global soname 2
%global _name %(tr - _ <<< %{name})
Epoch:          1
Version:	9.30
Release:	1%{?dist}
Vendor:	Globus Support
Summary:	Globus Toolkit - Globus Gass Copy

Group:		System Environment/Libraries
License:	ASL 2.0
URL:           https://www.globus.org/
Source:        https://downloads.globus.org/toolkit/gt6/packages/%{_name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  openssl
BuildRequires:  openssl-devel

BuildRequires:	globus-ftp-client-devel >= 7
BuildRequires:	globus-common-devel >= 15
BuildRequires:	globus-gssapi-gsi-devel >= 9
BuildRequires:	globus-io-devel >= 8
BuildRequires:	globus-gass-transfer-devel >= 7
BuildRequires:	globus-ftp-control-devel >= 4
BuildRequires:	globus-gridftp-server-progs
BuildRequires:	globus-gridftp-server-devel
BuildRequires:	globus-xio-gsi-driver-devel
BuildRequires:	globus-xio-pipe-driver-devel

BuildRequires:  automake >= 1.11
BuildRequires:  autoconf >= 2.60
BuildRequires:  libtool >= 2.2
BuildRequires:  pkgconfig
BuildRequires: libtool-ltdl-devel

# Perl modules used in test scripts
BuildRequires: perl(Cwd)
BuildRequires: perl(File::Compare)
BuildRequires: perl(File::Copy)
BuildRequires: perl(File::Path)
BuildRequires: perl(File::Temp)
BuildRequires: perl(Getopt::Long)
BuildRequires: perl(IPC::Open3)
BuildRequires: perl(Symbol)
BuildRequires: perl(Test::More)
BuildRequires: perl(URI)

%package progs
Summary:	Globus Toolkit - Globus Gass Copy Programs
Group:		Applications/Internet
Requires:	%{name}%{?_isa} = %{epoch}:%{version}-%{release}

%package devel
Summary:	Globus Toolkit - Globus Gass Copy Development Files
Group:		Development/Libraries
Requires:	%{name}%{?_isa} = %{epoch}:%{version}-%{release}
Requires:	globus-ftp-client-devel%{?_isa} >= 7
Requires:	globus-common-devel%{?_isa} >= 15
Requires:	globus-gssapi-gsi-devel%{?_isa} >= 9
Requires:	globus-io-devel%{?_isa} >= 8
Requires:	globus-gass-transfer-devel%{?_isa} >= 7
Requires:	globus-ftp-control-devel%{?_isa} >= 4

%package doc
Summary:	Globus Toolkit - Globus Gass Copy Documentation Files
Group:		Documentation
BuildArch:	noarch
Requires:	%{name} = %{epoch}:%{version}-%{release}

%description
The Globus Toolkit is an open source software toolkit used for building Grid
systems and applications. It is being developed by the Globus Alliance and
many others all over the world. A growing number of projects and companies are
using the Globus Toolkit to unlock the potential of grids for their cause.

The %{name} package contains:
Globus Gass Copy

%description progs
The Globus Toolkit is an open source software toolkit used for building Grid
systems and applications. It is being developed by the Globus Alliance and
many others all over the world. A growing number of projects and companies are
using the Globus Toolkit to unlock the potential of grids for their cause.

The %{name}-progs package contains:
Globus Gass Copy Programs

%description devel
The Globus Toolkit is an open source software toolkit used for building Grid
systems and applications. It is being developed by the Globus Alliance and
many others all over the world. A growing number of projects and companies are
using the Globus Toolkit to unlock the potential of grids for their cause.

The %{name}-devel package contains:
Globus Gass Copy Development Files

%description doc
The Globus Toolkit is an open source software toolkit used for building Grid
systems and applications. It is being developed by the Globus Alliance and
many others all over the world. A growing number of projects and companies are
using the Globus Toolkit to unlock the potential of grids for their cause.

The %{name}-doc package contains:
Globus Gass Copy Documentation Files

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

%post %{?nmainpkg} -p /sbin/ldconfig

%postun %{?nmainpkg} -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%dir %{_docdir}/%{name}-%{version}
%doc %{_docdir}/%{name}-%{version}/GLOBUS_LICENSE
%{_libdir}/libglobus*.so.*

%files progs
%defattr(-,root,root,-)
%{_bindir}/*
%{_mandir}/man1/*

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
* Fri Mar 26 2021 Globus Toolkit <support@globus.org> - 9.30-1
- Use prebuilt doxyxgen if available

* Wed Nov 27 2019 Globus Toolkit <support@globus.org> - 9.29-3
- Packaging update to ensure priority of Globus packages

* Fri Aug 24 2018 Globus Toolkit <support@globus.org> - 9.29-1
- use 2048 bit keys to support openssl 1.1.1

* Mon Sep 25 2017 Globus Toolkit <support@globus.org> - 9.28-1
- add popen driver to cygwin static builds

* Fri Apr 21 2017 Globus Toolkit <support@globus.org> - 9.27-1
- prevent some race conditions

* Fri Apr 21 2017 Globus Toolkit <support@globus.org> - 9.26-1
- remove checksum data from public handle

* Thu Apr 20 2017 Globus Toolkit <support@globus.org> - 9.25-1
- Fix uninitialized field related crash

* Fri Mar 10 2017 Globus Toolkit <support@globus.org> - 9.24-1
- don't attempt sshftp data protection without creds
- checksum verification based on contribution from IBM

* Thu Sep 08 2016 Globus Toolkit <support@globus.org> - 9.23-1
- Update for el.5 openssl101e, replace docbook with asciidoc

* Fri Aug 26 2016 Globus Toolkit <support@globus.org> - 9.22-3
- Updates for SLES 12

* Fri Aug 19 2016 Globus Toolkit <support@globus.org> - 9.22-1
- Fix tests run as root

* Thu Aug 18 2016 Globus Toolkit <support@globus.org> - 9.21-1
- Makefile fix

* Tue Aug 16 2016 Globus Toolkit <support@globus.org> - 9.20-1
- Updates for OpenSSL 1.1.0

* Mon Apr 18 2016 Globus Toolkit <support@globus.org> - 9.19-1
- Use prelinks for tests so that they run on El Capitan

* Fri Nov 20 2015 Globus Toolkit <support@globus.org> - 9.18-1
- Disable ipv6 default for tests

* Thu Aug 06 2015 Globus Toolkit <support@globus.org> - 9.17-2
- Add vendor

* Tue Jul 28 2015 Globus Toolkit <support@globus.org> - 9.17-1
- use SIGINT to terminating test server for gcov

* Wed Jul 01 2015 Globus Toolkit <support@globus.org> - 9.16-1
- Improve error handling
- Fix non-terminated string

* Wed Apr 08 2015 Globus Toolkit <support@globus.org> - 9.15-1
- Clarify documentation of stack options
- Add openssl build dependency

* Wed Apr 08 2015 Globus Toolkit <support@globus.org> - 9.14-1
- Fix user-specified data channel stack handling

* Mon Nov 03 2014 Globus Toolkit <support@globus.org> - 9.13-1
- doxygen fixes

* Mon Sep 22 2014 Globus Toolkit <support@globus.org> - 9.12-1
- Include more manpages for API
- Fix some Doxygen issues
- Quiet some autoconf/automake warnings

* Fri Aug 22 2014 Globus Toolkit <support@globus.org> - 9.11-1
- Merge fixes from ellert-globus_6_branch

* Wed Aug 20 2014 Globus Toolkit <support@globus.org> - 9.10-2
- Fix Source path

* Mon Jun 09 2014 Globus Toolkit <support@globus.org> - 9.10-1
- Merge changes from Mattias Ellert

* Thu Apr 24 2014 Globus Toolkit <support@globus.org> - 9.9-1
- Packaging fixes

* Fri Apr 18 2014 Globus Toolkit <support@globus.org> - 9.8-1
- Version bump for consistency

* Fri Apr 18 2014 Globus Toolkit <support@globus.org> - 9.7-1
- Version bump for consistency

* Tue Feb 25 2014 Globus Toolkit <support@globus.org> - 9.6-1
- Packaging fixes

* Thu Feb 20 2014 Globus Toolkit <support@globus.org> - 9.5-1
- Test fixes

* Thu Feb 20 2014 Globus Toolkit <support@globus.org> - 9.4-1
- Test fixes

* Thu Feb 20 2014 Globus Toolkit <support@globus.org> - 9.3-1
- Test fixes

* Wed Feb 19 2014 Globus Toolkit <support@globus.org> - 9.2-1
- Packaging fixes

* Wed Jan 22 2014 Globus Toolkit <support@globus.org> - 9.1-1
- Repackage for GT6 without GPT

* Wed Jan 22 2014 Globus Toolkit <support@globus.org> - 9.0-1
- Repackage for GT6 without GPT

* Wed Jun 26 2013 Globus Toolkit <support@globus.org> - 8.6-4
- GT-424: New Fedora Packaging Guideline - no %%_isa in BuildRequires

* Wed Feb 20 2013 Globus Toolkit <support@globus.org> - 8.6-3
- Workaround missing F18 doxygen/latex dependency

* Mon Nov 26 2012 Globus Toolkit <support@globus.org> - 8.6-2
- 5.2.3

* Tue Jul 17 2012 Joseph Bester <bester@mcs.anl.gov> - 8.6-1
- GT-241: wrong SIGINT handling in globus-url-copy

* Mon Jul 16 2012 Joseph Bester <bester@mcs.anl.gov> - 8.5-3
- GT 5.2.2 final

* Fri Jun 29 2012 Joseph Bester <bester@mcs.anl.gov> - 8.5-2
- GT 5.2.2 Release

* Wed Jun 27 2012 Joseph Bester <bester@mcs.anl.gov> - 8.5-1
- GRIDFTP-200: mixing ftp:// with -cred fails
- GRIDFTP-203: -create-dest fails when input is stdin
- GRIDFTP-208: Add manpage for globus-url-copy
- GRIDFTP-211: potentially unsafe format strings in globus-url-copy
- GRIDFTP-216: continue on error doesn't continue when a dir listing fails
- GRIDFTP-220: don't attempt mkdir when dir is known to exist.
- GT-153: make gridftp-v2 GET/PUT the default for server that support it
- RIC-224: Eliminate some doxygen warnings
- RIC-226: Some dependencies are missing in GPT metadata

* Wed May 09 2012 Joseph Bester <bester@mcs.anl.gov> - 8.4-3
- RHEL 4 patches

* Fri May 04 2012 Joseph Bester <bester@mcs.anl.gov> - 8.4-2
- SLES 11 patches

* Tue Mar 06 2012 Joseph Bester <bester@mcs.anl.gov> - 8.4-1
- GRIDFTP-200: mixing ftp:// with -cred fails
- GRIDFTP-203: -create-dest fails when input is stdin
- GRIDFTP-216: continue on error doesn't continue when a dir listing fails
- GRIDFTP-220: don't attempt mkdir when dir is known to exist.

* Tue Feb 14 2012 Joseph Bester <bester@mcs.anl.gov> - 8.3-1
- GRIDFTP-208: Add manpage for globus-url-copy
- GRIDFTP-211: potentially unsafe format strings in globus-url-copy
- RIC-224: Eliminate some doxygen warnings
- RIC-226: Some dependencies are missing in GPT metadata

* Mon Dec 05 2011 Joseph Bester <bester@mcs.anl.gov> - 8.2-4
- Update for 5.2.0 release

* Mon Dec 05 2011 Joseph Bester <bester@mcs.anl.gov> - 8.2-3
- Last sync prior to 5.2.0

* Tue Oct 11 2011 Joseph Bester <bester@mcs.anl.gov> - 8.1-2
- Add explicit dependencies on >= 5.2 libraries

* Thu Oct 06 2011 Joseph Bester <bester@mcs.anl.gov> - 8.1-1
- Add backward-compatibility aging

* Thu Sep 01 2011 Joseph Bester <bester@mcs.anl.gov> - 8.0-2
- Update for 5.1.2 release

* Sat Jul 17 2010 Mattias Ellert <mattias.ellert@fysast.uu.se> - 5.7-1
- Update to Globus Toolkit 5.0.2

* Wed Apr 14 2010 Mattias Ellert <mattias.ellert@fysast.uu.se> - 5.4-1
- Update to Globus Toolkit 5.0.1

* Sat Jan 23 2010 Mattias Ellert <mattias.ellert@fysast.uu.se> - 5.3-1
- Update to Globus Toolkit 5.0.0

* Fri Aug 21 2009 Tomas Mraz <tmraz@redhat.com> - 4.14-4
- rebuilt with new openssl

* Thu Jul 23 2009 Mattias Ellert <mattias.ellert@fysast.uu.se> - 4.14-3
- Add instruction set architecture (isa) tags
- Make doc subpackage noarch

* Thu Jun 04 2009 Mattias Ellert <mattias.ellert@fysast.uu.se> - 4.14-2
- Update to official Fedora Globus packaging guidelines

* Thu Apr 16 2009 Mattias Ellert <mattias.ellert@fysast.uu.se> - 4.14-1
- Make comment about source retrieval more explicit
- Change defines to globals
- Remove explicit requires on library packages
- Put GLOBUS_LICENSE file in extracted source tarball

* Sun Mar 15 2009 Mattias Ellert <mattias.ellert@fysast.uu.se> - 4.14-0.5
- Adapting to updated globus-core package

* Thu Feb 26 2009 Mattias Ellert <mattias.ellert@fysast.uu.se> - 4.14-0.4
- Add s390x to the list of 64 bit platforms

* Thu Jan 01 2009 Mattias Ellert <mattias.ellert@fysast.uu.se> - 4.14-0.3
- Adapt to updated GPT package

* Tue Oct 21 2008 Mattias Ellert <mattias.ellert@fysast.uu.se> - 4.14-0.2
- Update to Globus Toolkit 4.2.1

* Tue Jul 15 2008 Mattias Ellert <mattias.ellert@fysast.uu.se> - 4.10-0.1
- Autogenerated
