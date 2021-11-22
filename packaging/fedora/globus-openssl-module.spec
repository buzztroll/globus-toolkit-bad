Name:		globus-openssl-module
%global _name %(tr - _ <<< %{name})
Epoch:          1
Version:	4.11~a1
Release:	1%{?dist}
Vendor:	Globus Support
Summary:	Globus Toolkit - Globus OpenSSL Module Wrapper

Group:		System Environment/Libraries
License:	Globus Connect Community Source Code License Agreement
URL:           https://www.globus.org/
Source:        https://downloads.globus.org/toolkit/gt6/packages/%{_name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:	globus-gsi-proxy-ssl-devel >= 4
BuildRequires:	globus-common-devel >= 14
BuildRequires:	globus-gsi-openssl-error-devel >= 2

BuildRequires:  openssl
BuildRequires:  openssl-devel

BuildRequires:	automake >= 1.11
BuildRequires:	autoconf >= 2.60
BuildRequires:	libtool >= 2.2
BuildRequires:  pkgconfig

%package devel
Summary:	Globus Toolkit - Globus OpenSSL Module Wrapper Development Files
Group:		Development/Libraries
Requires:	%{name}%{?_isa} = %{epoch}:%{version}-%{release}
Requires:	globus-gsi-proxy-ssl-devel%{?_isa} >= 4
Requires:	globus-common-devel%{?_isa} >= 14
Requires:	globus-gsi-openssl-error-devel%{?_isa} >= 2

Requires:  openssl
Requires:  openssl-devel

%package doc
Summary:	Globus Toolkit - Globus OpenSSL Module Wrapper Documentation Files
Group:		Documentation
BuildArch:	noarch
Requires:	%{name} = %{epoch}:%{version}-%{release}

%description
The Globus Toolkit is an open source software toolkit used for building Grid
systems and applications. It is being developed by the Globus Alliance and
many others all over the world. A growing number of projects and companies are
using the Globus Toolkit to unlock the potential of grids for their cause.

The %{name} package contains:
Globus OpenSSL Module Wrapper

%description devel
The Globus Toolkit is an open source software toolkit used for building Grid
systems and applications. It is being developed by the Globus Alliance and
many others all over the world. A growing number of projects and companies are
using the Globus Toolkit to unlock the potential of grids for their cause.

The %{name}-devel package contains:
Globus OpenSSL Module Wrapper Development Files

%description doc
The Globus Toolkit is an open source software toolkit used for building Grid
systems and applications. It is being developed by the Globus Alliance and
many others all over the world. A growing number of projects and companies are
using the Globus Toolkit to unlock the potential of grids for their cause.

The %{name}-doc package contains:
Globus OpenSSL Module Wrapper Documentation Files

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

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%dir %{_docdir}/%{name}-%{version}
%doc %{_docdir}/%{name}-%{version}/GLOBUS_LICENSE
%{_libdir}/libglobus*.so.*

%files devel
%defattr(-,root,root,-)
%{_includedir}/globus/*
%{_libdir}/pkgconfig/%{name}.pc
%{_libdir}/libglobus*.so

%files doc
%defattr(-,root,root,-)
%dir %{_docdir}/%{name}-%{version}/html
%{_docdir}/%{name}-%{version}/html/*
%{_mandir}/man3/*

%changelog
* Thu Nov 18 2021 Globus Toolkit <support@globus.org> - 4.11~a1-1
- Prerelease build

* Thu Apr 29 2021 Globus Toolkit <support@globus.org> - 4.10-1
- Allow prerelease version tags

* Fri Mar 26 2021 Globus Toolkit <support@globus.org> - 4.9-1
- Use prebuilt doxyxgen if available

* Wed Mar 24 2021 Globus Toolkit <support@globus.org> - 4.8-4
- Remove conditional builds for unsupported OSes

* Wed Nov 27 2019 Globus Toolkit <support@globus.org> - 4.8-3
- Packaging update to ensure priority of Globus packages

* Thu Sep 08 2016 Globus Toolkit <support@globus.org> - 4.8-1
- Update for el.5 openssl101e

* Thu Aug 25 2016 Globus Toolkit <support@globus.org> - 4.7-2
- Updates for SLES 12

* Tue Aug 16 2016 Globus Toolkit <support@globus.org> - 4.7-1
- Updates for OpenSSL 1.1.0

* Thu Aug 06 2015 Globus Toolkit <support@globus.org> - 4.6-2
- Add vendor

* Thu Sep 25 2014 Globus Toolkit <support@globus.org> - 4.6-1
- Include more manpages for API
- Use consistent PREDEFINED in all Doxyfiles
- Doxygen markup fixes
- Quiet some autoconf/automake warnings

* Fri Aug 22 2014 Globus Toolkit <support@globus.org> - 4.5-1
- Merge fixes from ellert-globus_6_branch

* Wed Aug 20 2014 Globus Toolkit <support@globus.org> - 4.4-2
- Fix Source path

* Mon Jun 09 2014 Globus Toolkit <support@globus.org> - 4.4-1
- Merge changes from Mattias Ellert

* Tue May 27 2014 Globus Toolkit <support@globus.org> - 4.3-1
- Don't require initializer

* Fri Apr 18 2014 Globus Toolkit <support@globus.org> - 4.2-1
- Version bump for consistency

* Mon Jan 27 2014 Globus Toolkit <support@globus.org> - 4.1-1
- Add missing dependency

* Tue Jan 21 2014 Globus Toolkit <support@globus.org> - 4.0-1
- Repackage for GT6 without GPT

* Mon Jul 08 2013 Globus Toolkit <support@globus.org> - 3.3-4
- openssl-libs for newer packages

* Wed Jun 26 2013 Globus Toolkit <support@globus.org> - 3.3-2
- GT-424: New Fedora Packaging Guideline - no %%_isa in BuildRequires

* Fri Jun 14 2013 Globus Toolkit <support@globus.org> - 3.3-1
- Be more tolerant if a symlink fails

* Wed Feb 20 2013 Globus Toolkit <support@globus.org> - 3.2-7
- Workaround missing F18 doxygen/latex dependency

* Mon Nov 26 2012 Globus Toolkit <support@globus.org> - 3.2-6
- 5.2.3

* Mon Jul 16 2012 Joseph Bester <bester@mcs.anl.gov> - 3.2-5
- GT 5.2.2 final

* Fri Jun 29 2012 Joseph Bester <bester@mcs.anl.gov> - 3.2-4
- GT 5.2.2 Release

* Wed May 09 2012 Joseph Bester <bester@mcs.anl.gov> - 3.2-3
- RHEL 4 patches

* Fri May 04 2012 Joseph Bester <bester@mcs.anl.gov> - 3.2-2
- SLES 11 patches

* Tue Feb 14 2012 Joseph Bester <bester@mcs.anl.gov> - 3.2-1
- RIC-233: Move admin tool globus-update-certificate-dir to sbindir

* Mon Dec 05 2011 Joseph Bester <bester@mcs.anl.gov> - 3.1-4
- Update for 5.2.0 release

* Mon Dec 05 2011 Joseph Bester <bester@mcs.anl.gov> - 3.1-3
- Last sync prior to 5.2.0

* Tue Oct 11 2011 Joseph Bester <bester@mcs.anl.gov> - 3.1-2
- Add explicit dependencies on >= 5.2 libraries

* Thu Oct 06 2011 Joseph Bester <bester@mcs.anl.gov> - 3.1-1
- Add backward-compatibility aging

* Thu Sep 01 2011 Joseph Bester <bester@mcs.anl.gov> - 3.0-2
- Update for 5.1.2 release

* Sat Jul 17 2010 Mattias Ellert <mattias.ellert@fysast.uu.se> - 1.3-1
- Update to Globus Toolkit 5.0.2
- Drop patch globus-openssl-module-oid.patch (fixed upstream)

* Mon May 31 2010 Mattias Ellert <mattias.ellert@fysast.uu.se> - 1.2-2
- Fix OID registration pollution

* Tue Apr 13 2010 Mattias Ellert <mattias.ellert@fysast.uu.se> - 1.2-1
- Update to Globus Toolkit 5.0.1
- Drop patch globus-openssl-module-sslinit.patch (fixed upstream)

* Fri Jan 22 2010 Mattias Ellert <mattias.ellert@fysast.uu.se> - 1.1-1
- Update to Globus Toolkit 5.0.0

* Fri Aug 21 2009 Tomas Mraz <tmraz@redhat.com> - 0.6-4
- rebuilt with new openssl

* Thu Jul 23 2009 Mattias Ellert <mattias.ellert@fysast.uu.se> - 0.6-3
- Add instruction set architecture (isa) tags
- Make doc subpackage noarch

* Wed Jun 03 2009 Mattias Ellert <mattias.ellert@fysast.uu.se> - 0.6-2
- Update to official Fedora Globus packaging guidelines

* Wed Apr 15 2009 Mattias Ellert <mattias.ellert@fysast.uu.se> - 0.6-1
- Make comment about source retrieval more explicit
- Change defines to globals
- Remove explicit requires on library packages
- Put GLOBUS_LICENSE file in extracted source tarball

* Sun Mar 15 2009 Mattias Ellert <mattias.ellert@fysast.uu.se> - 0.6-0.5
- Adapting to updated globus-core package

* Thu Feb 26 2009 Mattias Ellert <mattias.ellert@fysast.uu.se> - 0.6-0.4
- Add s390x to the list of 64 bit platforms

* Thu Jan 01 2009 Mattias Ellert <mattias.ellert@fysast.uu.se> - 0.6-0.3
- Adapt to updated GPT package

* Mon Oct 13 2008 Mattias Ellert <mattias.ellert@fysast.uu.se> - 0.6-0.2
- Update to Globus Toolkit 4.2.1

* Mon Jul 14 2008 Mattias Ellert <mattias.ellert@fysast.uu.se> - 0.6-0.1
- Autogenerated
