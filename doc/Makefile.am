doc_DATA = user_help.html
man_MANS = vibedillo.1
EXTRA_DIST = \
	README \
	Cookies.txt \
	vibedillo.1.in \
	user_help.in.html \
	install.md

vibedillo.1: $(srcdir)/vibedillo.1.in Makefile
	sed 's%/usr/local%${prefix}%g' < $(srcdir)/vibedillo.1.in > vibedillo.1

# Use .in.html instead of .html.in so it is recognized as HTML.
user_help.html: $(srcdir)/user_help.in.html Makefile
	sed 's/__VERSION__/${VERSION}/g' $(srcdir)/user_help.in.html > $@

DISTCLEANFILES = vibedillo.1 user_help.html
