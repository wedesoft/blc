SUFFIXES = .c .h .tmp

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = mini

mini_SOURCES = mini.c
mini_LDFLAGS =
mini_LDADD =

BUILT_SOURCES =

EXTRA_DIST = Makefile.dist configure.ac README.md COPYING mini_spec.sh

test: test.tmp

test.tmp: mini$(EXEEXT)
	$(CAT) input.txt | ./mini$(EXEEXT) > output.tmp
	$(DIFF) output.txt output.tmp && $(TOUCH) $@

CLEANFILES = *~

MAINTAINERCLEANFILES = aclocal.m4 config.guess config.sub configure \
	install-sh ltmain.sh Makefile.in missing mkinstalldirs stamp-h.in \
	libtool config.cache config.h config.h.in acinclude.m4 depcomp

