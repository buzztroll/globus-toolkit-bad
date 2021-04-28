Name:		globus-gridmap-callout-error
%global soname 0
%global _name %(tr - _ <<< %{name})
Epoch:          1
Version:	2.7
Release:	1%{?dist}
Vendor:	Globus Support
Summary:	Globus Toolkit - Globus Gridmap Callout Errors

Group:		System Environment/Libraries
License:	ASL 2.0
URL:           https://www.globus.org/
Source:        https://downloads.globus.org/toolkit/gt6/packages/%{_name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:	globus-common-devel >= 14
BuildRequires:	globus-gssapi-error-devel >= 4

BuildRequires:  automake >= 1.11
BuildRequires:  autoconf >= 2.60
BuildRequires:  libtool >= 2.2
BuildRequires:  pkgconfig

%package devel
Summary:	Globus Toolkit - Globus Gridmap Callout Errors Development Files
Group:		Development/Libraries
Requires:	%{name}%{?_isa} = %{epoch}:%{version}-%{release}
Requires:	globus-common-devel%{?_isa} >= 14
Requires:	globus-gssapi-error-devel%{?_isa} >= 4

%package doc
Summary:	Globus Toolkit - Globus Gridmap Callout Errors Documentation Files
Group:		Documentation
BuildArch:	noarch
Requires:	%{name} = %{epoch}:%{version}-%{release}

%description
The Globus Toolkit is an open source software toolkit used for building Grid
systems and applications. It is being developed by the Globus Alliance and
many others all over the world. A growing number of projects and companies are
using the Globus Toolkit to unlock the potential of grids for their cause.

The %{name} package contains:
Globus Gridmap Callout Errors

%description devel
The Globus Toolkit is an open source software toolkit used for building Grid
systems and applications. It is being developed by the Globus Alliance and
many others all over the world. A growing number of projects and companies are
using the Globus Toolkit to unlock the potential of grids for their cause.

The %{name}-devel package contains:
Globus Gridmap Callout Errors Development Files

%description doc
The Globus Toolkit is an open source software toolkit used for building Grid
systems and applications. It is being developed by the Globus Alliance and
many others all over the world. A growing number of projects and companies are
using the Globus Toolkit to unlock the potential of grids for their cause.

The %{name}-doc package contains:
Globus Gridmap Callout Errors Documentation Files

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

%clean
rm -rf $RPM_BUILD_ROOT

%post %{?nmainpkg} -p /sbin/ldconfig

%postun %{?nmainpkg} -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%dir %{_docdir}/%{name}-%{version}
%{_docdir}/%{name}-%{version}/GLOBUS_LICENSE
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
* Fri Apr 23 2021 Globus Toolkit <support@globus.org> - 2.7-1
- Allow prerelease version tags

* Fri Mar 26 2021 Globus Toolkit <support@globus.org> - 2.6-1
- Use prebuilt doxyxgen if available

* Wed Nov 27 2019 Globus Toolkit <support@globus.org> - 2.5-6
- Packaging update to ensure priority of Globus packages

* Thu Sep 08 2016 Globus Toolkit <support@globus.org> - 2.5-4
- Rebuild after changes for el.5 with openssl101e

* Mon Aug 29 2016 Globus Toolkit <support@globus.org> - 2.5-3
- Updates for SLES 12

* Sat Aug 20 2016 Globus Toolkit <support@globus.org> - 2.5-1
- Update bug report URL

* Thu Aug 06 2015 Globus Toolkit <support@globus.org> - 2.4-2
- Add vendor

* Wed Sep 24 2014 Globus Toolkit <support@globus.org> - 2.4-1
- Include more manpages for API
- Doxygen markup fixes
- Quiet some autoconf/automake warnings

* Fri Aug 22 2014 Globus Toolkit <support@globus.org> - 2.3-3
- Merge fixes from ellert-globus_6_branch

* Wed Aug 20 2014 Globus Toolkit <support@globus.org> - 2.3-2
- Fix Source path

* Mon Jun 09 2014 Globus Toolkit <support@globus.org> - 2.3-1
- Merge changes from Mattias Ellert

* Fri Apr 18 2014 Globus Toolkit <support@globus.org> - 2.1-1
- Version bump for consistency

* Wed Jan 22 2014 Globus Toolkit <support@globus.org> - 2.0-1
- Repackage for GT6 without GPT

* Wed Jun 26 2013 Globus Toolkit <support@globus.org> - 1.2-10
- GT-424: New Fedora Packaging Guideline - no %%_isa in BuildRequires

* Wed Feb 20 2013 Globus Toolkit <support@globus.org> - 1.2-9
- Workaround missing F18 doxygen/latex dependency

* Mon Nov 26 2012 Globus Toolkit <support@globus.org> - 1.2-8
- 5.2.3

* Mon Jul 16 2012 Joseph Bester <bester@mcs.anl.gov> - 1.2-7
- GT 5.2.2 final

* Fri Jun 29 2012 Joseph Bester <bester@mcs.anl.gov> - 1.2-6
- GT 5.2.2 Release

* Wed May 09 2012 Joseph Bester <bester@mcs.anl.gov> - 1.2-5
- RHEL 4 patches

* Fri May 04 2012 Joseph Bester <bester@mcs.anl.gov> - 1.2-4
- SLES 11 patches

* Tue Feb 14 2012 Joseph Bester <bester@mcs.anl.gov> - 1.2-3
- Updated version numbers

* Mon Dec 05 2011 Joseph Bester <bester@mcs.anl.gov> - 1.2-2
- Update for 5.2.0 release

* Thu Oct 06 2011 Joseph Bester <bester@mcs.anl.gov> - 1.2-1
- Add backward-compatibility aging

* Tue Sep 13 2011 Joseph Bester <bester@mcs.anl.gov> - 1.0-1
- Update for Globus Toolkit 5.1.2

