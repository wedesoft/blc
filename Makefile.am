SUFFIXES = .c .h .tmp

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = tokenize minischeme

tokenize_SOURCES = tokenize.c
tokenize_LDFLAGS =
tokenize_LDADD =

minischeme_SOURCES = minischeme.c
minischeme_LDFLAGS =
minischeme_LDADD =

BUILT_SOURCES =

EXTRA_DIST = Makefile.dist configure.ac README.md COPYING \
	test_tokenize_in.txt test_tokenize_out.txt

test: test.tmp

test.tmp: tokenize$(EXEEXT)
	$(CAT) test_tokenize_in.txt | ./tokenize$(EXEEXT) > out.tmp
	$(DIFF) test_tokenize_out.txt out.tmp && $(TOUCH) $@

CLEANFILES = *~ *.tmp

MAINTAINERCLEANFILES = aclocal.m4 config.guess config.sub configure \
	install-sh ltmain.sh Makefile.in missing mkinstalldirs stamp-h.in \
	libtool config.cache config.h config.h.in acinclude.m4 depcomp

