SUFFIXES = .c .h .tmp

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = tokenize miniracket

tokenize_SOURCES = tokenize.c
tokenize_LDFLAGS =
tokenize_LDADD =

miniracket_SOURCES = miniracket.c
miniracket_LDFLAGS =
miniracket_LDADD =

BUILT_SOURCES =

EXTRA_DIST = Makefile.dist configure.ac README.md COPYING \
	test_tokenize_in.txt test_tokenize_out.txt \
	test_miniracket_in.txt test_miniracket_out.txt

test: test_tokenize.tmp test_miniracket.tmp

test_tokenize.tmp: tokenize$(EXEEXT)
	$(CAT) test_tokenize_in.txt | ./tokenize$(EXEEXT) > out.tmp
	$(DIFF) test_tokenize_out.txt out.tmp && $(TOUCH) $@

test_miniracket.tmp: tokenize$(EXEEXT) miniracket$(EXEEXT)
	$(CAT) test_miniracket_in.txt | ./tokenize $(EXEEXT) | ./miniracket$(EXEEXT) > out.tmp
	$(DIFF) test_miniracket_out.txt out.tmp && $(TOUCH) $@

CLEANFILES = *~ *.tmp

MAINTAINERCLEANFILES = aclocal.m4 config.guess config.sub configure \
	install-sh ltmain.sh Makefile.in missing mkinstalldirs stamp-h.in \
	libtool config.cache config.h config.h.in acinclude.m4 depcomp

