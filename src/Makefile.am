AM_CPPFLAGS =						\
	$(GS_PRIVATE_CFLAGS)				\
	$(GTKSPELL_CFLAGS)				\
	$(GNOME_DESKTOP_CFLAGS)				\
	-I$(top_srcdir)/lib				\
	-DG_LOG_DOMAIN=\"Gs\"				\
	-DLIBDIR=\"$(libdir)\"				\
	-DBINDIR=\"$(bindir)\"				\
	-DDATADIR=\"$(datadir)\"			\
	-DVERSION="\"$(VERSION)\""			\
	-DLOCALEDIR=\""$(localedir)"\"			\
	-DLOCALSTATEDIR=\""$(localstatedir)"\"		\
	-DLIBEXECDIR=\"$(libexecdir)\"			\
	-DTESTDATADIR=\""$(top_srcdir)/data"\"		\
	-DLOCALPLUGINDIR=\""$(top_builddir)/plugins/.libs"\"

%.desktop: %.desktop.in
	$(AM_V_GEN) msgfmt --desktop -d $(top_srcdir)/po --template $< -o $@

desktopdir = $(datadir)/applications
desktop_in_files =					\
	gnome-software-local-file.desktop.in		\
	org.gnome.Software.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

UI_FILES =						\
	gnome-software.ui				\
	gs-app-addon-row.ui				\
	gs-app-folder-dialog.ui				\
	gs-app-row.ui					\
	gs-app-tile.ui					\
	gs-auth-dialog.ui				\
	gs-category-page.ui				\
	gs-category-tile.ui				\
	gs-details-page.ui				\
	gs-extras-page.ui				\
	gs-feature-tile.ui				\
	gs-first-run-dialog.ui				\
	gs-history-dialog.ui				\
	gs-image-tile.ui				\
	gs-info-bar.ui					\
	gs-installed-page.ui				\
	gs-loading-page.ui				\
	gs-menus.ui					\
	gs-moderate-page.ui				\
	gs-overview-page.ui				\
	gs-popular-tile.ui				\
	gs-removal-dialog.ui				\
	gs-review-dialog.ui				\
	gs-review-histogram.ui				\
	gs-review-row.ui				\
	gs-screenshot-image.ui				\
	gs-search-page.ui				\
	gs-side-filter-row.ui				\
	gs-sources-dialog-row.ui			\
	gs-sources-dialog.ui				\
	gs-star-widget.ui				\
	gs-summary-tile.ui				\
	gs-update-dialog.ui				\
	gs-updates-page.ui				\
	gs-upgrade-banner.ui

dbusservicedir = $(datadir)/dbus-1/services
dbusservice_DATA =					\
	org.gnome.Software.service

if HAVE_PACKAGEKIT
dbusservice_DATA += 					\
	org.freedesktop.PackageKit.service

org.freedesktop.PackageKit.service: org.freedesktop.PackageKit.service.in Makefile
	$(AM_V_GEN) sed -e "s|\@bindir\@|$(bindir)|" $<> $@
endif

org.gnome.Software.service: org.gnome.Software.service.in Makefile
	$(AM_V_GEN) sed -e "s|\@bindir\@|$(bindir)|" $<> $@

searchproviderdir = $(datadir)/gnome-shell/search-providers
searchprovider_DATA = org.gnome.Software-search-provider.ini

discoveryfeedcontentproviderdir = $(datadir)/eos-discovery-feed/content-providers
discoveryfeedcontentprovider_DATA = org.gnome.Software-discovery-feed-content-provider.ini

autostartdir = $(sysconfdir)/xdg/autostart
autostart_DATA = gnome-software-service.desktop

gnome-software-service.desktop: gnome-software-service.desktop.in Makefile
	$(AM_V_GEN) sed -e "s|\@bindir\@|$(bindir)|" $<> $@

libexec_PROGRAMS =					\
	gnome-software-restarter

bin_PROGRAMS =						\
	gnome-software

gnome_software_SOURCES =				\
	gs-app-addon-row.c				\
	gs-app-addon-row.h				\
	gs-app-folder-dialog.c				\
	gs-app-folder-dialog.h				\
	gs-application.c				\
	gs-application.h				\
	gs-app-row.c					\
	gs-app-row.h					\
	gs-app-tile.c					\
	gs-app-tile.h					\
	gs-auth-dialog.c				\
	gs-auth-dialog.h				\
	gs-category-page.c				\
	gs-category-page.h				\
	gs-category-tile.c				\
	gs-category-tile.h				\
	gs-common.c					\
	gs-common.h					\
	gs-content-rating.c				\
	gs-content-rating.h				\
	gs-details-page.c				\
	gs-details-page.h				\
	gs-discovery-feed-content-provider.c		\
	gs-discovery-feed-content-provider.h		\
	gs-extras-page.c				\
	gs-extras-page.h				\
	gs-feature-tile.c				\
	gs-feature-tile.h				\
	gs-first-run-dialog.c				\
	gs-first-run-dialog.h				\
	gs-fixed-size-bin.c				\
	gs-fixed-size-bin.h				\
	gs-folders.c					\
	gs-folders.h					\
	gs-hiding-box.c					\
	gs-hiding-box.h					\
	gs-history-dialog.c				\
	gs-history-dialog.h				\
	gs-image-tile.c					\
	gs-image-tile.h					\
	gs-info-bar.c					\
	gs-info-bar.h					\
	gs-installed-page.c				\
	gs-installed-page.h				\
	gs-language.c					\
	gs-language.h					\
	gs-loading-page.c				\
	gs-loading-page.h				\
	gs-main.c					\
	gs-moderate-page.c				\
	gs-moderate-page.h				\
	gs-overview-page.c				\
	gs-overview-page.h				\
	gs-page.c					\
	gs-page.h					\
	gs-popular-tile.c				\
	gs-popular-tile.h				\
	gs-progress-button.c				\
	gs-progress-button.h				\
	gs-removal-dialog.c				\
	gs-removal-dialog.h				\
	gs-review-bar.c					\
	gs-review-bar.h					\
	gs-review-dialog.c				\
	gs-review-dialog.h				\
	gs-review-histogram.c				\
	gs-review-histogram.h				\
	gs-review-row.c					\
	gs-review-row.h					\
	gs-screenshot-image.c				\
	gs-screenshot-image.h				\
	gs-search-page.c				\
	gs-search-page.h				\
	gs-side-filter-row.c				\
	gs-side-filter-row.h				\
	gs-shell.c					\
	gs-shell.h					\
	gs-shell-search-provider.c			\
	gs-shell-search-provider.h			\
	gs-sources-dialog.c				\
	gs-sources-dialog.h				\
	gs-sources-dialog-row.c				\
	gs-sources-dialog-row.h				\
	gs-star-widget.c				\
	gs-star-widget.h				\
	gs-summary-tile.c				\
	gs-summary-tile.h				\
	gs-update-dialog.c				\
	gs-update-dialog.h				\
	gs-update-list.c				\
	gs-update-list.h				\
	gs-update-monitor.c				\
	gs-update-monitor.h				\
	gs-updates-page.c				\
	gs-updates-page.h				\
	gs-upgrade-banner.c				\
	gs-upgrade-banner.h				\
	gs-vendor.c					\
	gs-vendor.h

nodist_gnome_software_SOURCES =						\
	gs-discovery-feed-content-provider-generated.c	\
	gs-discovery-feed-content-provider-generated.h	\
	gs-resources.c									\
	gs-resources.h									\
	gs-shell-search-provider-generated.c			\
	gs-shell-search-provider-generated.h

gnome_software_LDADD =					\
	$(top_builddir)/lib/libgnomesoftware.a		\
	$(GS_PRIVATE_LIBS)				\
	$(GTKSPELL_LIBS)				\
	$(PACKAGEKIT_LIBS)				\
	$(GNOME_DESKTOP_LIBS)

gnome_software_CFLAGS =					\
	$(GS_PRIVATE_CFLAGS)

gnome_software_LDFLAGS =				\
	$(PIE_LDFLAGS)					\
	$(RELRO_LDFLAGS)

if HAVE_PACKAGEKIT
gnome_software_SOURCES +=				\
	gs-dbus-helper.c				\
	gs-dbus-helper.h

gnome_software_CFLAGS +=				\
	-DI_KNOW_THE_PACKAGEKIT_GLIB2_API_IS_SUBJECT_TO_CHANGE \
	 $(PACKAGEKIT_CFLAGS)

nodist_gnome_software_SOURCES +=			\
	$(packagekit_built_sources)			\
	$(packagekit_modify2_built_sources)
endif

gnome_software_restarter_SOURCES =			\
	gs-restarter.c

gnome_software_restarter_LDADD =			\
	$(GLIB_LIBS)

gnome_software_restarter_CFLAGS =			\
	-DLIBEXECDIR=\"$(libexecdir)\"			\
	$(WARN_CFLAGS)

gnome_software_restarter_LDFLAGS =			\
	$(PIE_LDFLAGS)					\
	$(RELRO_LDFLAGS)

packagekit_built_sources = gs-packagekit-generated.c gs-packagekit-generated.h
$(packagekit_built_sources): Makefile.am org.freedesktop.PackageKit.xml
	$(AM_V_GEN) gdbus-codegen				\
		--interface-prefix org.freedesktop.		\
		--c-namespace Gs				\
		--generate-c-code gs-packagekit-generated	\
		$(srcdir)/org.freedesktop.PackageKit.xml

packagekit_modify2_built_sources = gs-packagekit-modify2-generated.c gs-packagekit-modify2-generated.h
$(packagekit_modify2_built_sources): Makefile.am org.freedesktop.PackageKit.Modify2.xml
	$(AM_V_GEN) gdbus-codegen					\
		--interface-prefix org.freedesktop.			\
		--c-namespace Gs					\
		--generate-c-code gs-packagekit-modify2-generated	\
		$(srcdir)/org.freedesktop.PackageKit.Modify2.xml

gs-shell-search-provider-generated.h gs-shell-search-provider-generated.c: Makefile.am $(srcdir)/shell-search-provider-dbus-interfaces.xml
	$(AM_V_GEN) gdbus-codegen \
	 	--interface-prefix org.gnome. \
		--c-namespace Gs \
		--generate-c-code gs-shell-search-provider-generated \
		$(srcdir)/shell-search-provider-dbus-interfaces.xml

gs-discovery-feed-content-provider-generated.h gs-discovery-feed-content-provider-generated.c: Makefile.am $(srcdir)/discovery-feed-content-provider-dbus-interfaces.xml
	$(AM_V_GEN) gdbus-codegen \
	 	--interface-prefix com.endlessm. \
		--c-namespace Gs \
		--generate-c-code gs-discovery-feed-content-provider-generated \
		$(srcdir)/discovery-feed-content-provider-dbus-interfaces.xml

gs-resources.c: gnome-software.gresource.xml $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir) --sourcedir="$(top_srcdir)/data/icons/categories" --generate-dependencies $(srcdir)/gnome-software.gresource.xml)
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir) --sourcedir="$(top_srcdir)/data/icons/categories" --target=$@ --generate-source --c-name gs $(srcdir)/gnome-software.gresource.xml

gs-resources.h: gnome-software.gresource.xml $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir) --sourcedir="$(top_srcdir)/data/icons/categories" --generate-dependencies $(srcdir)/gnome-software.gresource.xml)
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir) --sourcedir="$(top_srcdir)/data/icons/categories" --target=$@ --generate-header --c-name gs $(srcdir)/gnome-software.gresource.xml

BUILT_SOURCES =						\
	$(packagekit_built_sources)			\
	$(packagekit_modify2_built_sources)		\
	gs-discovery-feed-content-provider-generated.h	\
	gs-discovery-feed-content-provider-generated.c	\
	gs-shell-search-provider-generated.c		\
	gs-shell-search-provider-generated.h		\
	gs-resources.c					\
	gs-resources.h

EXTRA_DIST =						\
	shell-search-provider-dbus-interfaces.xml	\
	discovery-feed-content-provider-dbus-interfaces.xml \
	org.freedesktop.PackageKit.xml			\
	org.freedesktop.PackageKit.Modify2.xml		\
	gnome-software.gresource.xml			\
	gnome-software.xml				\
	gtk-style.css					\
	gtk-style-hc.css				\
	$(desktop_in_files)				\
	org.freedesktop.PackageKit.service.in		\
	org.gnome.Software.service.in			\
	gnome-software-service.desktop.in		\
	$(searchprovider_DATA)				\
	$(discoveryfeedcontentprovider_DATA) \
	$(UI_FILES)

man_MANS =
if ENABLE_MAN
man_MANS += gnome-software.1
endif

XSLTPROC_FLAGS = \
	--nonet \
	--stringparam man.output.quietly 1 \
	--stringparam funcsynopsis.style ansi \
	--stringparam man.th.extra1.suppress 1 \
	--stringparam man.authors.section.enabled 0 \
	--stringparam man.copyright.section.enabled 0

gnome-software.1: gnome-software.xml
	$(AM_V_GEN) $(XSLTPROC) $(XSLTPROC_FLAGS) http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl $<

clean-local:
	rm -f *~
	rm -f *.1
	rm -f manpage.*

massif:
	make && \
	valgrind --massif-out-file=massif.out --threshold=0.1 --tool=massif \
		./gnome-software-cmd search gimp --refine-flags=icon --show-results && \
	ms_print massif.out > massif.txt && \
	head -n 30 massif.txt

CLEANFILES =						\
	$(BUILT_SOURCES)				\
	gnome-software.1				\
	manpage.*

MAINTAINERCLEANFILES =					\
	*~						\
	Makefile.in

DISTCLEANFILES =					\
	gnome-software-local-file.desktop		\
	gnome-software-service.desktop			\
	org.gnome.Software.desktop			\
	org.freedesktop.PackageKit.service		\
	org.gnome.Software.service

-include $(top_srcdir)/git.mk
