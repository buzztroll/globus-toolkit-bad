Name:		globus-xio-gridftp-driver
%global _name %(tr - _ <<< %{name})
Epoch:          1
Version:	2.20~rc1
Release:	1%{?dist}
Vendor:	Globus Support
Summary:	Globus Toolkit - Globus XIO GridFTP Driver

Group:		System Environment/Libraries
License:	ASL 2.0
URL:           https://www.globus.org/
Source:        https://downloads.globus.org/toolkit/gt6/packages/%{_name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

Requires:	globus-xio-gsi-driver%{?_isa} >= 2

BuildRequires:	globus-xio-devel >= 3
BuildRequires:	globus-ftp-client-devel >= 7
BuildRequires:	globus-xio-gsi-driver-devel >= 2
BuildRequires:	globus-xio-pipe-driver-devel >= 0
BuildRequires:	globus-gridftp-server-progs >= 0
BuildRequires:	globus-gridftp-server-devel >= 0
BuildRequires:	globus-xio-doc >= 3
BuildRequires:  automake >= 1.11
BuildRequires:  autoconf >= 2.60
BuildRequires:  libtool >= 2.2
BuildRequires:  pkgconfig
BuildRequires: libtool-ltdl-devel

BuildRequires: perl
BuildRequires: perl(File::Compare)
BuildRequires: perl(File::Temp)
BuildRequires: perl(Getopt::Long)
BuildRequires: perl(IPC::Open3)
BuildRequires: perl(Symbol)
BuildRequires: perl(Test::More)

%package devel
Summary:	Globus Toolkit - Globus XIO GSI Driver Development Files
Group:		Development/Libraries
Requires:	%{name}%{?_isa} = %{epoch}:%{version}-%{release}
Requires:	globus-ftp-client-devel%{?_isa} >= 7
Requires:	globus-xio-devel%{?_isa} >= 3
Requires:	globus-xio-gsi-driver-devel%{?_isa} >= 2

%package doc
Summary:	Globus Toolkit - Globus XIO GSI Driver Documentation Files
Group:		Documentation
BuildArch:	noarch
Requires:	%{name} = %{epoch}:%{version}-%{release}

%description
The Globus Toolkit is an open source software toolkit used for building Grid
systems and applications. It is being developed by the Globus Alliance and
many others all over the world. A growing number of projects and companies are
using the Globus Toolkit to unlock the potential of grids for their cause.

The %{name} package contains:
Globus XIO GridFTP Driver

%description devel
The Globus Toolkit is an open source software toolkit used for building Grid
systems and applications. It is being developed by the Globus Alliance and
many others all over the world. A growing number of projects and companies are
using the Globus Toolkit to unlock the potential of grids for their cause.

The %{name}-devel package contains:
Globus XIO GridFTP Driver Development Files

%description doc
The Globus Toolkit is an open source software toolkit used for building Grid
systems and applications. It is being developed by the Globus Alliance and
many others all over the world. A growing number of projects and companies are
using the Globus Toolkit to unlock the potential of grids for their cause.

The %{name}-doc package contains:
Globus XIO GridFTP Driver Documentation Files

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

find $RPM_BUILD_ROOT%{_libdir} -name 'lib*.la' -exec rm -vf '{}' \;

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
%{_libdir}/libglobus*.so*

%files devel
%defattr(-,root,root,-)
%{_includedir}/globus/*
%{_libdir}/pkgconfig/%{name}.pc

%files doc
%defattr(-,root,root,-)
%dir %{_docdir}/%{name}-%{version}/html
%{_docdir}/%{name}-%{version}/html/*
%{_mandir}/man3/*

%changelog
* Thu Apr 29 2021 Globus Toolkit <support@globus.org> - 2.20~rc1-1
- Allow prerelease version tags

* Fri Mar 26 2021 Globus Toolkit <support@globus.org> - 2.19-1
- Use prebuilt doxyxgen if available

* Wed Nov 27 2019 Globus Toolkit <support@globus.org> - 2.18-3
- Packaging update to ensure priority of Globus packages

* Fri Aug 24 2018 Globus Toolkit <support@globus.org> - 2.18-1
- use 2048 bit keys to support openssl 1.1.1

* Thu Sep 08 2016 Globus Toolkit <support@globus.org> - 2.17-2
- Update for el.5 openssl101e, replace docbook with asciidoc

* Mon Aug 29 2016 Globus Toolkit <support@globus.org> - 2.16-2
- Updates for SLES 12

* Fri Aug 19 2016 Globus Toolkit <support@globus.org> - 2.16-1
- don't use no-fork with root-run tests

* Thu Aug 18 2016 Globus Toolkit <support@globus.org> - 2.15-1
- Makefile fix

* Tue Aug 16 2016 Globus Toolkit <support@globus.org> - 2.14-1
- Updates for OpenSSL 1.1.0

* Tue Apr 19 2016 Globus Toolkit <support@globus.org> - 2.13-1
- Add dlpreopen force

* Thu Aug 06 2015 Globus Toolkit <support@globus.org> - 2.12-2
- Add vendor

* Tue Jul 28 2015 Globus Toolkit <support@globus.org> - 2.12-1
- use SIGINT to terminating test server for gcov

* Tue Jul 14 2015 Globus Toolkit <support@globus.org> - 2.11-1
- Fix missing va_arg in attr_cntl
- Fix memory leak

* Thu Apr 16 2015 Globus Toolkit <support@globus.org> - 2.10-2
- Add openssl build dependency

* Mon Jan 12 2015 Globus Toolkit <support@globus.org> - 2.10-1
- Fix tests on static builds

* Mon Jan 12 2015 Globus Toolkit <support@globus.org> - 2.9-2
- Fix tests on static builds

* Tue Sep 30 2014 Globus Toolkit <support@globus.org> - 2.8-1
- Metadata version out of sync

* Tue Sep 23 2014 Globus Toolkit <support@globus.org> - 2.7-2
- Doxygen markup fixes
- Fix typos and clarify some documentation
- Replace some old GLOBUS_NULL values with NULL

* Fri Aug 22 2014 Globus Toolkit <support@globus.org> - 2.7-1
- Merge fixes from ellert-globus_6_branch

* Wed Aug 20 2014 Globus Toolkit <support@globus.org> - 2.6-2
- Fix Source path

* Mon Jun 09 2014 Globus Toolkit <support@globus.org> - 2.6-1
- Merge changes from Mattias Ellert

* Tue May 06 2014 Globus Toolkit <support@globus.org> - 2.5-1
- Don't version dynamic module

* Thu Apr 24 2014 Globus Toolkit <support@globus.org> - 2.4-2
- Filelist fix for unversioned .so

* Fri Apr 18 2014 Globus Toolkit <support@globus.org> - 2.4-1
- Version bump for consistency

* Thu Feb 20 2014 Globus Toolkit <support@globus.org> - 2.3-1
- GLOBUS_USAGE_OPTOUT tests

* Wed Feb 19 2014 Globus Toolkit <support@globus.org> - 2.2-1
- Packaging fixes

* Tue Feb 18 2014 Globus Toolkit <support@globus.org> - 2.1-1
- Packaging fixes

* Wed Jan 22 2014 Globus Toolkit <support@globus.org> - 2.0-1
- Repackage for GT6 without GPT

* Wed Jun 26 2013 Globus Toolkit <support@globus.org> - 1.2-2
- GT-424: New Fedora Packaging Guideline - no %_isa in BuildRequires

* Wed Jun 19 2013 Globus Toolkit <support@globus.org> - 1.2-1
- add GLOBUS_LICENSE

* Tue Jun 18 2013 Globus Toolkit <support@globus.org> - 1.1-1
- Initial rpm
