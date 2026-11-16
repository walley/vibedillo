AM_CPPFLAGS = \
	-I$(top_srcdir)

bookmarksdir = $(libdir)/vibedillo/dpi/bookmarks
downloadsdir = $(libdir)/vibedillo/dpi/downloads
ftpdir = $(libdir)/vibedillo/dpi/ftp
hellodir = $(libdir)/vibedillo/dpi/hello
vsourcedir = $(libdir)/vibedillo/dpi/vsource
filedir = $(libdir)/vibedillo/dpi/file
cookiesdir = $(libdir)/vibedillo/dpi/cookies
datauridir = $(libdir)/vibedillo/dpi/datauri
bookmarks_PROGRAMS = bookmarks.dpi
downloads_PROGRAMS = downloads.dpi
ftp_PROGRAMS = ftp.filter.dpi
hello_PROGRAMS = hello.filter.dpi
vsource_PROGRAMS = vsource.filter.dpi
file_PROGRAMS = file.dpi
cookies_PROGRAMS = cookies.dpi
datauri_PROGRAMS = datauri.filter.dpi

bookmarks_dpi_LDADD = \
	$(top_builddir)/dpip/libDpip.a \
	$(top_builddir)/dlib/libDlib.a
downloads_dpi_LDADD = @FLTK_LIBS@ \
	$(top_builddir)/dpip/libDpip.a \
	$(top_builddir)/dlib/libDlib.a
ftp_filter_dpi_LDADD = \
	$(top_builddir)/dpip/libDpip.a \
	$(top_builddir)/dlib/libDlib.a
hello_filter_dpi_LDADD = \
	$(top_builddir)/dpip/libDpip.a \
	$(top_builddir)/dlib/libDlib.a
vsource_filter_dpi_LDADD = \
	$(top_builddir)/dpip/libDpip.a \
	$(top_builddir)/dlib/libDlib.a
file_dpi_LDADD = \
	$(top_builddir)/dpip/libDpip.a \
	$(top_builddir)/dlib/libDlib.a
cookies_dpi_LDADD = \
	$(top_builddir)/dpip/libDpip.a \
	$(top_builddir)/dlib/libDlib.a
datauri_filter_dpi_LDADD = \
	$(top_builddir)/dpip/libDpip.a \
	$(top_builddir)/dlib/libDlib.a

downloads_dpi_CXXFLAGS = @FLTK_CXXFLAGS@

bookmarks_dpi_SOURCES = bookmarks.c dpiutil.c dpiutil.h
downloads_dpi_SOURCES = downloads.cc dpiutil.c dpiutil.h
ftp_filter_dpi_SOURCES = ftp.c dpiutil.c dpiutil.h
hello_filter_dpi_SOURCES = hello.c dpiutil.c dpiutil.h
vsource_filter_dpi_SOURCES = vsource.c dpiutil.c dpiutil.h
file_dpi_SOURCES = file.c dpiutil.c dpiutil.h
cookies_dpi_SOURCES = cookies.c dpiutil.c dpiutil.h
datauri_filter_dpi_SOURCES = datauri.c dpiutil.c dpiutil.h

