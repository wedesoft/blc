SUFFIXES = .c .h .tmp .y .l

ACLOCAL_AMFLAGS = -I m4

AM_YFLAGS = -d

bin_PROGRAMS = spec_blc blc lambda spec_lambda

blc_SOURCES = blc_repl.c blc.c
blc_LDFLAGS = 
blc_LDADD =

spec_blc_SOURCES = spec_blc.c blc.c
spec_blc_LDFLAGS =
spec_blc_LDADD =

lambda_SOURCES = lambda_repl.c lambda.c lambda_bison.y lambda_flex.l blc.c
lambda_LDFLAGS =
lambda_LDADD =

spec_lambda_SOURCES = spec_lambda.c lambda.c lambda_bison.y lambda_flex.l blc.c
spec_lambda_LDFLAGS =
spec_lambda_LDADD =

noinst_HEADERS = blc.h lambda.h

noinst_DATA = README.html TODO.html

BUILT_SOURCES = lambda_bison.h

EXTRA_DIST = Makefile.dist configure.ac README.md TODO.md COPYING TODO run.sh

test: spec_blc.tmp spec_lambda.tmp

spec_blc.tmp: spec_blc$(EXEEXT)
	./spec_blc$(EXEEXT) && $(TOUCH) $@

spec_lambda.tmp: spec_lambda$(EXEEXT)
	./spec_lambda$(EXEEXT) && $(TOUCH) $@

.md.html:
	$(PANDOC) -o $@ $< || echo "Install Pandoc to compile Markdown to HTML"

CLEANFILES = *~ *.tmp *.html

MAINTAINERCLEANFILES = aclocal.m4 config.guess config.sub configure \
	install-sh ltmain.sh Makefile.in missing mkinstalldirs stamp-h.in \
	libtool config.cache config.h config.h.in acinclude.m4 depcomp \
	ylwrap

maintainer-clean-local:
	-rm -rf m4

