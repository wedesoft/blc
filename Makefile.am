SUFFIXES = .c .h .tmp .y .l

SUBDIRS = src logo

ACLOCAL_AMFLAGS = -I m4

noinst_DATA = README.html TODO.html

EXTRA_DIST = Makefile.dist configure.ac README.md TODO.md COPYING

test:
	test src = . || ($(am__cd) src && $(MAKE) $(AM_MAKEFLAGS) test)

.md.html:
	$(PANDOC) -o $@ $< || echo "Install Pandoc to compile Markdown to HTML"

CLEANFILES = *~ *.html

DISTCLEANFILES = .*.un~ .*.swp

MAINTAINERCLEANFILES = aclocal.m4 config.guess config.sub configure \
	install-sh ltmain.sh Makefile.in missing mkinstalldirs stamp-h.in \
	libtool config.cache config.h config.h.in acinclude.m4 depcomp

maintainer-clean-local:
	-rm -rf m4
