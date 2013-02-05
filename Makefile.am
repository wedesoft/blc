SUFFIXES = .c .h .tmp

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = spec_blc spec_lambda blc lambda

spec_blc_SOURCES = spec_blc.c blc.c
spec_blc_LDFLAGS =
spec_blc_LDADD =

spec_lambda_SOURCES = spec_lambda.c lambda.c blc.c
spec_lambda_LDFLAGS =
spec_lambda_LDADD =

blc_SOURCES = blc_repl.c blc.c
blc_LDFLAGS = 
blc_LDADD =

lambda_SOURCES = lambda_repl.c lambda.c blc.c
lambda_LDFLAGS = 
lambda_LDADD =

noinst_HEADERS = blc.h lambda.h

BUILT_SOURCES =

EXTRA_DIST = Makefile.dist configure.ac README.md COPYING TODO run.sh

test: spec_blc.tmp spec_lambda.tmp

spec_blc.tmp: spec_blc$(EXEEXT)
	./spec_blc$(EXEEXT) && $(TOUCH) $@

spec_lambda.tmp: spec_lambda$(EXEEXT)
	./spec_lambda$(EXEEXT) && $(TOUCH) $@

CLEANFILES = *~ *.tmp

MAINTAINERCLEANFILES = aclocal.m4 config.guess config.sub configure \
	install-sh ltmain.sh Makefile.in missing mkinstalldirs stamp-h.in \
	libtool config.cache config.h config.h.in acinclude.m4 depcomp

maintainer-clean-local:
	-rm -rf m4

