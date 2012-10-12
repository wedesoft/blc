SUFFIXES = .c .h .tmp

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = tokenize boot-eval

tokenize_SOURCES = tokenize.c tokenizer.c
tokenize_LDFLAGS =
tokenize_LDADD =

boot_eval_SOURCES = boot-eval.c tokenizer.c
boot_eval_LDFLAGS = 
boot_eval_LDADD =

noinst_HEADERS = tokenizer.h

BUILT_SOURCES =

EXTRA_DIST = Makefile.dist configure.ac README.md COPYING \
	test_tokenize_in.txt test_tokenize_out.txt \
	test_boot_in.txt test_boot_out.txt

test: test_tokenize.tmp test_boot.tmp

test_tokenize.tmp: tokenize$(EXEEXT) test_tokenize_in.txt test_tokenize_out.txt
	$(CAT) test_tokenize_in.txt | ./tokenize$(EXEEXT) > out.tmp
	$(DIFF) test_tokenize_out.txt out.tmp && $(TOUCH) $@

test_boot.tmp: boot-eval$(EXEEXT) test_boot_in.txt test_boot_out.txt
	$(CAT) test_boot_in.txt | ./boot-eval$(EXEEXT) > out.tmp
	$(DIFF) test_boot_out.txt out.tmp && $(TOUCH) $@

CLEANFILES = *~ *.tmp

MAINTAINERCLEANFILES = aclocal.m4 config.guess config.sub configure \
	install-sh ltmain.sh Makefile.in missing mkinstalldirs stamp-h.in \
	libtool config.cache config.h config.h.in acinclude.m4 depcomp

