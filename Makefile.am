SUBDIRS = src tools debian

EXTRA_DIST = NEWS README TODO AUTHORS ChangeLog autogen.sh simili.pc.in \
	     simili.list.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = simili.pc

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

epm-deb: simili.list.in all
	epm -f deb --output-dir . simili
