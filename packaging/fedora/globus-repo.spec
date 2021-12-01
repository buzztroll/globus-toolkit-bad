Name:           globus-repo
Version:        6.0.23
Release:        1
Summary:        Globus Repository Configuration
Group:          System Environment/Base
License:        ASL 2.0
URL:           https://www.globus.org/
Source0:        globus-repo_%{version}.tar.xz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildArch:      noarch
Provides:       globus-connect-server-repo
Obsoletes:      globus-repo
Requires:	system-release

%description
This package installs the Globus yum repository configuration and GPG key for
Globus Toolkit 6 and Globus Connect Server 5.

%prep
%setup -q -n %{name}

%build
./globus-generate-repo -r

%install
rm -rf $RPM_BUILD_ROOT

# gpg
install -Dpm 644 RPM-GPG-KEY-Globus \
  $RPM_BUILD_ROOT%{_sysconfdir}/pki/rpm-gpg/RPM-GPG-KEY-Globus

install -dm 755 $RPM_BUILD_ROOT%{_datadir}/globus/repo
for repo in *.repo; do
    install -pm 644 $repo $RPM_BUILD_ROOT%{_datadir}/globus/repo
done

%clean
rm -rf $RPM_BUILD_ROOT

%posttrans
rpm --import %{_sysconfdir}/pki/rpm-gpg/RPM-GPG-KEY-Globus 2>/dev/null

. /etc/os-release

case $ID:$VERSION_ID:$ID_LIKE in
    rhel:7*:* | amzn:2:* | centos:7*:* | ol:7*:* | scientific:7*:*)
        repo=el7
        ;;
    rhel:8*:* | almalinux:8*:* | centos:8*:* | ol:8*:* | rocky:8*:*)
        repo=el8
        ;;
    *:7*:*rhel* | *:7*:*centos* )
        repo=el7
        ;;
    *:8*:*rhel* | *:8*:*centos* )
        repo=el8
        ;;
    fedora:*:*)
        repo=fedora
        ;;
    *)
	echo "Unsupported repo" 1>&2
	exit 1
        ;;
esac

if command -v dnf > /dev/null; then
    for repofile in %{_datadir}/globus/repo/*-${repo}.repo; do
        dnf config-manager --add-repo file://$repofile
    done
elif command -v yum-config-manager > /dev/null; then
    for repofile in %{_datadir}/globus/repo/*-${repo}.repo; do
        yum-config-manager --add-repo file://$repofile
    done
elif [ -d %{_sysconfdir}/yum.repos.d ] ; then
    cp %{_datadir}/globus/repo/*-${repo}.repo %{_sysconfdir}/yum.repos.d
else
    echo "Copy the Globus Repository Definition from %{_datadir}/globus/repo/ to your system's repo configuration"
fi

%preun
if [ "$1" != 0 ]; then
    exit 0
fi

. /etc/os-release

case $ID:$VERSION_ID:$ID_LIKE in
    rhel:7*:* | amzn:2:* | centos:7*:* | ol:7*:* | scientific:7*:*)
        repo=el7
        ;;
    rhel:8*:* | almalinux:8*:* | centos:8*:* | ol:8*:* | rocky:8*:*)
        repo=el8
        ;;
    *:7*:rhel | *:7*:centos )
        repo=el7
        ;;
    *:8*:rhel | *:8*:centos )
        repo=el8
        ;;
    fedora:*:*)
        repo=fedora
        ;;
    *)
	echo "Unsupported repo" 1>&2
	exit 1
        ;;
esac

if [ -d %{_sysconfdir}/yum.repos.d ]; then
    for repofile in %{_datadir}/globus/repo/*-${repo}.repo; do
        rm -f %{_sysconfdir}/yum.repos.d/$(basename $repofile)
    done
else
    echo "Remove the Globus Repository defintion from your system configuration"
fi

%files
%defattr(-,root,root,-)
%{_sysconfdir}/pki/rpm-gpg/*
%{_datadir}/globus/repo/*

%changelog
* Wed Oct 27 2021 Globus Toolkit <support@globus.org> - 6.0.23-1
- (deb) add bullseye and impish
- (deb) remove groovy
- Refactor version detection in repo install scripts

* Fri Apr 23 2021 Globus Toolkit <support@globus.org> - 6.0.22-1
- (deb) add groovy and hirsute
- (deb) remove jessie, stretch, xenial, cosmic, disco, eoan
- (rpm) remove el.6, sles.12

* Fri May 22 2020 Globus Toolkit <support@globus.org> - 6.0.21-1
- (deb) add focal

* Mon Nov 18 2019 Globus Toolkit <support@globus.org> - 6.0.20-1
- (rpm) add el8
- (deb) add buster and eoan

* Tue May  7 2019 Globus Toolkit <support@globus.org> - 6.0.17-1
- (deb) Add packaging pinning to our repo

* Fri Apr 26 2019 Globus Toolkit <support@globus.org> - 6.0.16-1
- (deb) Add disco
- (deb) remove trusty

* Thu Dec  6 2018 Globus Toolkit <support@globus.org> - 6.0.15-1
- (deb) Add cosmic
- (deb) remove wheezy and artful

* Fri Apr  6 2018 Globus Toolkit <support@globus.org> - 6.0.14-1
- (deb) Add bionic
- (deb) remove yakkety and zesty

* Mon Feb 19 2018 Globus Toolkit <support@globus.org> - 6.0.13-1
- (rpm) Fix yum install when yum-config-tools is not present

* Thu Jun 22 2017 Globus Toolkit <support@globus.org> - 6.0.12-1
- (deb) Fix GCSv5 repo install

* Wed May 31 2017 Globus Toolkit <support@globus.org> - 6.0.11-1
- (rpm) Fix fedora repo path

* Fri May 25 2017 Globus Toolkit <support@globus.org> - 6.0.10-1
- (debian) Add dependency on apt-transport-https

* Mon May 22 2017 Globus Toolkit <support@globus.org> - 6.0.9-1
- Move repos to downloads.globus.org
- Combine with equivalent deb package

* Thu May  4 2017 Globus Toolkit <support@globus.org> - 6-24
- Remove el.5
- Move repos to s3 hosted https
- Add GCS 5 repo

* Wed Aug 31 2016 Globus Toolkit <support@globus.org> - 6-23
- Updates for SLES 12

* Mon Aug 1 2016 Globus Toolkit <support@globus.org> - 6-22
- ignore key import errors

* Mon Aug 1 2016 Globus Toolkit <support@globus.org> - 6-21
- Use dnf config-manager to install repo when available

* Thu May 19 2016 Globus Toolkit <support@globus.org> - 6-20
- Add springdale (rhel-compatible)

* Mon Apr 06 2015 Globus Toolkit <support@globus.org> - 6-18
- Handle update from broken versions

* Wed Apr 01 2015 Globus Toolkit <support@globus.org> - 6-17
- Don't require lsb

* Tue Mar 31 2015 Globus Toolkit <support@globus.org> - 6-16
- Don't preun when upgrading

* Mon Mar 30 2015 Globus Toolkit <support@globus.org> - 6-15
- Rename www.globus.org -> toolkit.globus.org

* Thu Nov 13 2014 Globus Toolkit <support@globus.org> - 6-14
- Don't use zypper from postinstall on SUSE

* Thu Nov 06 2014 Globus Toolkit <support@globus.org> - 6-13
- Import key on non-SUSE

* Thu Nov 06 2014 Globus Toolkit <support@globus.org> - 6-12
- Fix sed substitution command line

* Tue Sep 16 2014 Globus Toolkit <support@globus.org> - 6-11
- Fix sed substitution to enable stable when yum-utils are not present

* Mon Sep 15 2014 Globus Toolkit <support@globus.org> - 6-10
- Add Provides alias for globus-connect-server-repo

* Fri Aug 22 2014 Globus Toolkit <support@globus.org> - 6-9
- Add pre/post dependency on lsb

* Fri Aug 15 2014 Globus Toolkit <support@globus.org> - 6-8
- Move unstable repo to www.globus.org

* Wed Aug 13 2014 Globus Toolkit <support@globus.org> - 6-7
- Add priority to repositories

* Mon Aug 11 2014 Globus Toolkit <support@globus.org> - 6-6
- Fix "type" line

* Mon Aug 11 2014 Globus Toolkit <support@globus.org> - 6-5
- Avoid rpm import deadlock

* Mon Aug 11 2014 Globus Toolkit <support@globus.org> - 6-4
- Don't require yum-utils

* Mon Aug 11 2014 Globus Toolkit <support@globus.org> - 6-3
- Add SLES11 to repo list

* Tue Jul 22 2014 Globus Toolkit <support@globus.org> - 6-2
- Add EL7 to repo list
