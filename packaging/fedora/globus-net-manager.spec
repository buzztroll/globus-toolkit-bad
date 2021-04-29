Name:		globus-net-manager
%global soname 0
%global _name %(tr - _ <<< %{name})
Epoch:          1
Version:	0.21~rc1
Release:	1%{?dist}
Vendor:	Globus Support
Summary:	Globus Toolkit - Net Manager Library

Group:		System Environment/Libraries
License:	ASL 2.0
URL:           https://www.globus.org/
Source:        https://downloads.globus.org/toolkit/gt6/packages/%{_name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:	globus-common-devel >= 15.27
BuildRequires:	globus-xio-devel >= 5
BuildRequires:  automake >= 1.11
BuildRequires:  autoconf >= 2.60
BuildRequires:  libtool >= 2.2
BuildRequires:  pkgconfig
%if %{?rhel}%{!?rhel:0} >= 8 || %{?fedora}%{!?fedora:0} >= 31
BuildRequires:  python2-devel
%else
BuildRequires:  python-devel
%endif

%package devel
Summary:	Globus Toolkit - Net Manager Library Development Files
Group:		Development/Libraries
Requires:	%{name}%{?_isa} = %{epoch}:%{version}-%{release}
Requires:	globus-common-devel%{?_isa} >= 15.27
Requires:	globus-xio-devel%{?_isa} >= 5

%package -n globus-xio-net-manager-driver
Summary:	Globus Toolkit - Net Manager Library XIO Driver
Group:		System Environment/Libraries
Requires:	%{name}%{?_isa} = %{epoch}:%{version}-%{release}
Requires:	globus-common-devel%{?_isa} >= 15.27
Requires:	globus-xio-devel%{?_isa} >= 5
Provides:       globus-net-manager-xio-driver

%package doc
Summary:	Globus Toolkit - Net Manager Library Documentation Files
Group:		Documentation
BuildArch:	noarch
Requires:	%{name} = %{epoch}:%{version}-%{release}

%description
The Globus Toolkit is an open source software toolkit used for building Grid
systems and applications. It is being developed by the Globus Alliance and
many others all over the world. A growing number of projects and companies are
using the Globus Toolkit to unlock the potential of grids for their cause.

The %{name} package contains:
Net Manager Library

%description devel
The Globus Toolkit is an open source software toolkit used for building Grid
systems and applications. It is being developed by the Globus Alliance and
many others all over the world. A growing number of projects and companies are
using the Globus Toolkit to unlock the potential of grids for their cause.

The %{name}-devel package contains:
Net Manager Library Development Files

%description -n globus-xio-net-manager-driver
The Globus Toolkit is an open source software toolkit used for building Grid
systems and applications. It is being developed by the Globus Alliance and
many others all over the world. A growing number of projects and companies are
using the Globus Toolkit to unlock the potential of grids for their cause.

The %{name}-devel package contains:
Net Manager Library XIO Driver

%description doc
The Globus Toolkit is an open source software toolkit used for building Grid
systems and applications. It is being developed by the Globus Alliance and
many others all over the world. A growing number of projects and companies are
using the Globus Toolkit to unlock the potential of grids for their cause.

The %{name}-doc package contains:
Net Manager Library Documentation Files

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
           --libexecdir=%{_datadir}/globus \
           --enable-python 

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
%{_libdir}/lib%{_name}.so.*

%files devel
%defattr(-,root,root,-)
%{_includedir}/globus/*.h
%{_libdir}/libglobus_net_manager*.so
%{_libdir}/pkgconfig/%{name}.pc
%{_libdir}/pkgconfig/globus-xio-net-manager-driver.pc

%files -n globus-xio-net-manager-driver
%defattr(-,root,root,-)
%{_libdir}/libglobus_xio_net_manager_driver.so

%files doc
%defattr(-,root,root,-)
%dir %{_docdir}/%{name}-%{version}/html
%{_docdir}/%{name}-%{version}/html/*
%{_mandir}/man3/*

%changelog
* Thu Apr 29 2021 Globus Toolkit <support@globus.org> - 0.21~rc1-1
- Allow prerelease version tags

* Fri Mar 26 2021 Globus Toolkit <support@globus.org> - 0.20-1
- Use prebuilt doxyxgen if available

* Wed Nov 27 2019 Globus Toolkit <support@globus.org> - 0.19-3
- Packaging update to ensure priority of Globus packages

* Fri Nov 22 2019 Globus Toolkit <support@globus.org> - 0.19-1
- support el8

* Tue May 01 2018 Globus Toolkit <support@globus.org> - 0.18-2
- fix pre-connect not using changed remote contact

* Tue Apr 04 2017 Globus Toolkit <support@globus.org> - 0.17-1
- Fix .pc typo

* Thu Sep 08 2016 Globus Toolkit <support@globus.org> - 0.16-2
- Rebuild after changes for el.5 with openssl101e

* Thu Sep 08 2016 Globus Toolkit <support@globus.org> - 0.16-1
- exclude tests from doc

* Fri Aug 26 2016 Globus Toolkit <support@globus.org> - 0.15-5
- Updates for SLES 12

* Sat Aug 20 2016 Globus Toolkit <support@globus.org> - 0.15-2
- Update bug report URL

* Mon Apr 18 2016 Globus Toolkit <support@globus.org> - 0.15-1
- Use prelinks for tests so that they run on El Capitan

* Fri Dec 18 2015 Globus Toolkit <support@globus.org> - 0.14-1
- pre_connect return attrs get set on attr, not handle

* Thu Oct 29 2015 Globus Toolkit <support@globus.org> - 0.13-1
- Remove unused code

* Thu Aug 06 2015 Globus Toolkit <support@globus.org> - 0.12-2
- Add vendor

* Tue Jul 14 2015 Globus Toolkit <support@globus.org> - 0.12-1
- Fix linkage on Mac with libtool 2.4.6

* Tue Jul 14 2015 Globus Toolkit <support@globus.org> - 0.11-1
- Fix memory leaks, NULL pointer derefs, and dead assignments

* Wed Jul 01 2015 Globus Toolkit <support@globus.org> - 0.10-1
- Fix uninitialized value
- Remove unused variables

* Wed Jun 17 2015 Globus Toolkit <support@globus.org> - 0.9-1
- Fix missing documentation
- Clarify python invocation
- Fix error handling
- Add test for end_listen in python
- Allow running tests with valgrind

* Mon Jun 01 2015 Globus Toolkit <support@globus.org> - 0.8-2
- Rename xio driver package

* Mon Apr 13 2015 Globus Toolkit <support@globus.org> - 0.8-1
- fix for attr not being used on connect()

* Fri Mar 27 2015 Globus Toolkit <support@globus.org> - 0.7-1
- add file paramter to logging driver to set a file to log to.  use manager=logging;file=/path/to/file;.

* Fri Jan 09 2015 Globus Toolkit <support@globus.org> - 0.6-1
- Fix conflicts with globus-common-doc and globus-xio-doc

* Thu Jan 08 2015 Globus Toolkit <support@globus.org> - 0.5-1
- Fix test link on recent debians

* Wed Jan 07 2015 Globus Toolkit <support@globus.org> - 0.4-1
- Link in ltdl for tests

* Mon Jan 05 2015 Globus Toolkit <support@globus.org> - 0.3-1
- Tests run with static build

* Mon Dec 22 2014 Globus Toolkit <support@globus.org> - 0.2-1
- Fix missing skip test

* Fri Dec 19 2014 Globus Toolkit <support@globus.org> - 0.1-1
- check for python2.6-config

* Wed Dec 17 2014 Globus Toolkit <support@globus.org> - 0.0-1
- Initial package
