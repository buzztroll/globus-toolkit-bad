Name:           globus-python
Version:        3.9.16
Release:        1%{?dist}
Summary:        Python 3.9.16
Group:          System Environment/Libraries
License:        Python
URL:            https://www.python.org/downloads/release/python-3916/
Source0:        https://www.python.org/ftp/python/3.9.16/Python-3.9.16.tgz
Patch00378:     https://src.fedoraproject.org/rpms/python2.7/raw/rawhide/f/00378-support-expat-2-4-5.patch

# Don't bother with debuginfo package
%global         debug_package           %{nil}

BuildRoot:      %{_tmppath}/python-%{version}-%{release}-build
BuildRequires:  openssl-devel, jansson-devel, check-devel, doxygen
BuildRequires: autoconf
BuildRequires: bluez-libs-devel
BuildRequires: bzip2
BuildRequires: bzip2-devel
BuildRequires: expat-devel
BuildRequires: findutils
BuildRequires: gcc-c++
BuildRequires: gdbm-devel
BuildRequires: glibc-devel
BuildRequires: gmp-devel
BuildRequires: libappstream-glib
BuildRequires: libffi-devel
BuildRequires: libtirpc-devel
BuildRequires: libGL-devel
BuildRequires: libuuid-devel
BuildRequires: libX11-devel
BuildRequires: ncurses-devel

BuildRequires: openssl-devel
BuildRequires: pkgconfig
BuildRequires: readline-devel
BuildRequires: redhat-rpm-config
BuildRequires: sqlite-devel
BuildRequires: gdb

BuildRequires: tar
BuildRequires: tcl-devel
BuildRequires: tix-devel
BuildRequires: tk-devel
BuildRequires: xz-devel
BuildRequires: zlib-devel
BuildRequires: net-tools

# Turn off the brp-python-bytecompile script. This version of python may include
# features not supported in system __python
%global __os_install_post %(echo '%{__os_install_post}' | sed -e 's!/usr/lib[^[:space:]]*/brp-python-bytecompile[[:space:]].*$!!g')
# Turn off the brp-python-bytecompile automagic
%global _python_bytecompile_extra 0
%global _python_root /opt/globus-python

%description
Python %{version} installed into %_python_root

%prep
%setup -q -n Python-%{version}
%patch00378 -p1

%build

./configure --prefix=%_python_root \
  --enable-ipv6 \
  --with-computed-gotos=yes \
  --with-dbmliborder=gdbm:ndbm:bdb \
  --with-system-expat \
  --with-system-ffi \
  --enable-loadable-sqlite-extensions \
  --with-lto \
  --with-ssl-default-suites=openssl \
  --without-ensurepip \
  --enable-optimizations
%make_build

%install
rm -rf $PRM_BUILD_ROOT

%make_install

sed -i -e "/#! \/usr\/local\/bin\/python/d" $RPM_BUILD_ROOT%{_python_root}/lib/python3.9/cgi.py

%check
make %{?_smp_mflags} test

%clean
rm -rf $PRM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%dir %_python_root
%_python_root/*

%changelog
* Wed Jan 4 2023 Globus Toolkit<support@globus.org> - 3.9.16-1
- Initial package of python 3.9 for GCS
