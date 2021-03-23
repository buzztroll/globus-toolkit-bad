ACLOCAL_AMFLAGS=-I m4
pkgconfdir = $(libdir)/pkgconfig

SUBDIRS = library test

doc_DATA = GLOBUS_LICENSE
pkgconf_DATA = globus-gsi-proxy-ssl.pc

EXTRA_DIST = $(doc_DATA)

distuninstallcheck:
	@:

doc:
	$(MAKE) $(AM_MAKEFLAGS) -C library doc
