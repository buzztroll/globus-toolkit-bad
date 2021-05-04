Name:		globus-authz
%global soname 0
%global _name %(tr - _ <<< %{name})
Epoch:          1
Version:	3.18
Release:	1%{?dist}
Vendor:	Globus Support
Summary:	Globus Toolkit - Globus authz library

Group:		System Environment/Libraries
License:	ASL 2.0
URL:           https://www.globus.org/
Source:        https://downloads.globus.org/toolkit/gt6/packages/%{_name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:	globus-authz-callout-error-devel >= 2
BuildRequires:	globus-callout-devel >= 2
BuildRequires:	globus-gssapi-gsi-devel >= 9
BuildRequires:	globus-common-devel >= 14
BuildRequires:  automake >= 1.11
BuildRequires:  autoconf >= 2.60
BuildRequires:  libtool >= 2.2
BuildRequires:  pkgconfig
BuildRequires: libtool-ltdl-devel

%package devel
Summary:	Globus Toolkit - Globus authz library Development Files
Group:		Development/Libraries
Requires:	%{name}%{?_isa} = %{epoch}:%{version}-%{release}
Requires:	globus-authz-callout-error-devel%{?_isa}
Requires:	globus-callout-devel%{?_isa}
Requires:	globus-gssapi-gsi-devel%{?_isa} >= 9

%package doc
Summary:	Globus Toolkit - Globus authz library Documentation Files
Group:		Documentation
BuildArch:	noarch
Requires:	%{name} = %{epoch}:%{version}-%{release}

%description
The Globus Toolkit is an open source software toolkit used for building Grid
systems and applications. It is being developed by the Globus Alliance and
many others all over the world. A growing number of projects and companies are
using the Globus Toolkit to unlock the potential of grids for their cause.

The %{name} package contains:
Globus authz library

%description devel
The Globus Toolkit is an open source software toolkit used for building Grid
systems and applications. It is being developed by the Globus Alliance and
many others all over the world. A growing number of projects and companies are
using the Globus Toolkit to unlock the potential of grids for their cause.

The %{name}-devel package contains:
Globus authz library Development Files

%description doc
The Globus Toolkit is an open source software toolkit used for building Grid
systems and applications. It is being developed by the Globus Alliance and
many others all over the world. A growing number of projects and companies are
using the Globus Toolkit to unlock the potential of grids for their cause.

The %{name}-doc package contains:
Globus authz library Documentation Files

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
%{_libdir}/libglobus_*so.*

%files devel
%defattr(-,root,root,-)
%{_includedir}/globus/*
%{_libdir}/libglobus_*.so
%{_libdir}/pkgconfig/%{name}.pc

%files doc
%defattr(-,root,root,-)
%dir %{_docdir}/%{name}-%{version}/html
%{_docdir}/%{name}-%{version}/html/*
%{_mandir}/*/*

%changelog
* Thu Apr 29 2021 Globus Toolkit <support@globus.org> - 3.18-1
- Allow prerelease version tags

* Fri Mar 26 2021 Globus Toolkit <support@globus.org> - 3.17-1
- Use prebuilt doxyxgen if available

* Wed Nov 27 2019 Globus Toolkit <support@globus.org> - 3.16-3
- Packaging update to ensure priority of Globus packages

* Fri Aug 24 2018 Globus Toolkit <support@globus.org> - 3.16-1
- use 2048 bit keys to support openssl 1.1.1

* Thu Sep 08 2016 Globus Toolkit <support@globus.org> - 3.15-1
- Update for el.5 openssl101e

* Thu Aug 25 2016 Globus Toolkit <support@globus.org> - 3.14-2
- Updates for SLES 12

* Thu Aug 18 2016 Globus Toolkit <support@globus.org> - 3.14-1
- Makefile fix

* Tue Aug 16 2016 Globus Toolkit <support@globus.org> - 3.13-1
- Updates for OpenSSL 1.1.0

* Wed Apr 06 2016 Globus Toolkit <support@globus.org> - 3.12-1
- Fix authz callout load in test on mac

* Fri Nov 20 2015 Globus Toolkit <support@globus.org> - 3.11-1
- Fix paths for mingw tests

* Thu Aug 06 2015 Globus Toolkit <support@globus.org> - 3.10-2
- Add vendor

* Wed Sep 24 2014 Globus Toolkit <support@globus.org> - 3.10-1
- Doxygen markup fixes
- Include more manpages for API
- Fix typos and clarify some documentation

* Fri Aug 22 2014 Globus Toolkit <support@globus.org> - 3.9-1
- Merge fixes from ellert-globus_6_branch

* Wed Aug 20 2014 Globus Toolkit <support@globus.org> - 3.8-2
- Fix Source path

* Mon Jun 09 2014 Globus Toolkit <support@globus.org> - 3.8-1
- Merge changes from Mattias Ellert

* Mon Apr 21 2014 Globus Toolkit <support@globus.org> - 3.7-1
- Test fixes

* Mon Apr 21 2014 Globus Toolkit <support@globus.org> - 3.6-1
- Test fixes

* Fri Apr 18 2014 Globus Toolkit <support@globus.org> - 3.5-1
- Version bump for consistency

* Thu Feb 13 2014 Globus Toolkit <support@globus.org> - 3.4-1
- Test Fixes

* Thu Feb 13 2014 Globus Toolkit <support@globus.org> - 3.3-1
- Test fixes

* Thu Feb 13 2014 Globus Toolkit <support@globus.org> - 3.1-1
- Test fixes

* Tue Jan 21 2014 Globus Toolkit <support@globus.org> - 3.0-1
- Repackage for GT6 without GPT

* Wed Jun 26 2013 Globus Toolkit <support@globus.org> - 2.2-9
- GT-424: New Fedora Packaging Guideline - no %%_isa in BuildRequires

* Wed Mar 06 2013 Globus Toolkit <support@globus.org> - 2.2-8
- add missing build dependencies

* Wed Feb 20 2013 Globus Toolkit <support@globus.org> - 2.2-7
- Workaround missing F18 doxygen/latex dependency

* Mon Nov 26 2012 Globus Toolkit <support@globus.org> - 2.2-6
- 5.2.3

* Mon Jul 16 2012 Joseph Bester <bester@mcs.anl.gov> - 2.2-5
- GT 5.2.2 final

* Fri Jun 29 2012 Joseph Bester <bester@mcs.anl.gov> - 2.2-4
- GT 5.2.2 Release

* Wed May 09 2012 Joseph Bester <bester@mcs.anl.gov> - 2.2-3
- RHEL 4 patches

* Fri May 04 2012 Joseph Bester <bester@mcs.anl.gov> - 2.2-2
- SLES 11 patches

* Tue Feb 14 2012 Joseph Bester <bester@mcs.anl.gov> - 2.2-1
- RIC-224: Eliminate some doxygen warnings

* Mon Dec 05 2011 Joseph Bester <bester@mcs.anl.gov> - 2.1-4
- Update for 5.2.0 release

* Mon Dec 05 2011 Joseph Bester <bester@mcs.anl.gov> - 2.1-3
- Last sync prior to 5.2.0

* Tue Oct 11 2011 Joseph Bester <bester@mcs.anl.gov> - 2.1-2
- Add explicit dependencies on >= 5.2 libraries

* Thu Oct 06 2011 Joseph Bester <bester@mcs.anl.gov> - 2.1-1
- Add backward-compatibility aging

* Thu Sep 01 2011 Joseph Bester <bester@mcs.anl.gov> - 2.0-2
- Update for 5.1.2 release

* Sat Jan 23 2010 Mattias Ellert <mattias.ellert@fysast.uu.se> - 0.7-2
- Update to Globus Toolkit 5.0.0

* Tue Jul 28 2009 Mattias Ellert <mattias.ellert@fysast.uu.se> - 0.7-1
- Autogenerated
