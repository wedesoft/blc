SUFFIXES = .c .h .tmp

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = spec_bracket boot-eval

spec_bracket_SOURCES = spec_bracket.c boot-eval.c
spec_bracket_LDFLAGS =
spec_bracket_LDADD =

boot_eval_SOURCES = repl.c boot-eval.c
boot_eval_LDFLAGS = 
boot_eval_LDADD =

noinst_HEADERS = boot-eval.h

BUILT_SOURCES =

EXTRA_DIST = Makefile.dist configure.ac README.md COPYING TODO boot.rkt run.sh

test: spec_bracket.tmp

spec_bracket.tmp: spec_bracket$(EXEEXT)
	./spec_bracket$(EXEEXT) && $(TOUCH) $@

CLEANFILES = *~ *.tmp

MAINTAINERCLEANFILES = aclocal.m4 config.guess config.sub configure \
	install-sh ltmain.sh Makefile.in missing mkinstalldirs stamp-h.in \
	libtool config.cache config.h config.h.in acinclude.m4 depcomp

maintainer-clean-local:
	-rm -rf m4

