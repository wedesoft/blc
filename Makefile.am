SUFFIXES = .c .h .tmp .y .l

SUBDIRS = logo

ACLOCAL_AMFLAGS = -I m4

AM_YFLAGS = -d

bin_PROGRAMS = x

x_SOURCES = x.c
x_LDFLAGS =
x_LDADD =

noinst_DATA = README.html TODO.html

EXTRA_DIST = Makefile.dist configure.ac README.md TODO.md COPYING run.sh

test: x.tmp

x.tmp: x$(EXEEXT)
	./x$(EXEEXT) && $(TOUCH) $@

.md.html:
	$(PANDOC) -o $@ $< || echo "Install Pandoc to compile Markdown to HTML"

CLEANFILES = *~ *.html

DISTCLEANFILES = .*.un~ .*.swp

MAINTAINERCLEANFILES = aclocal.m4 config.guess config.sub configure \
	install-sh ltmain.sh Makefile.in missing mkinstalldirs stamp-h.in \
	libtool config.cache config.h config.h.in acinclude.m4 depcomp

maintainer-clean-local:
	-rm -rf m4

