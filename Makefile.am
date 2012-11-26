SUFFIXES = .c .h .tmp

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = spec_blc blc

spec_blc_SOURCES = spec_blc.c blc.c
spec_blc_LDFLAGS =
spec_blc_LDADD =

blc_SOURCES = repl.c blc.c
blc_LDFLAGS = 
blc_LDADD =

noinst_HEADERS = blc.h

BUILT_SOURCES =

EXTRA_DIST = Makefile.dist configure.ac README.md COPYING TODO run.sh

test: spec_blc.tmp

spec_blc.tmp: spec_blc$(EXEEXT)
	./spec_blc$(EXEEXT) && $(TOUCH) $@

CLEANFILES = *~ *.tmp

MAINTAINERCLEANFILES = aclocal.m4 config.guess config.sub configure \
	install-sh ltmain.sh Makefile.in missing mkinstalldirs stamp-h.in \
	libtool config.cache config.h config.h.in acinclude.m4 depcomp

maintainer-clean-local:
	-rm -rf m4

