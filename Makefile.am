SUFFIXES = .c .h .tmp

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = spec_bracket boot-eval

spec_bracket_SOURCES = spec_bracket.c boot-eval.c tokenizer.c
spec_bracket_LDFLAGS =
spec_bracket_LDADD =

boot_eval_SOURCES = repl.c boot-eval.c tokenizer.c
boot_eval_LDFLAGS = 
boot_eval_LDADD =

noinst_HEADERS = tokenizer.h boot-eval.h

BUILT_SOURCES =

EXTRA_DIST = Makefile.dist configure.ac README.md COPYING TODO \
	test_tokenize_in.txt test_tokenize_out.txt \
	test_boot_in.txt test_boot_out.txt

test: spec_bracket.tmp test_boot.tmp

spec_bracket.tmp: spec_bracket$(EXEEXT)
	./spec_bracket$(EXEEXT) && $(TOUCH) $@

test_boot.tmp: boot-eval$(EXEEXT) boot.rkt test_boot_in.txt test_boot_out.txt
	$(CAT) test_boot_in.txt | ./boot-eval$(EXEEXT) > out.tmp
	$(DIFF) test_boot_out.txt out.tmp && $(TOUCH) $@

CLEANFILES = *~ *.tmp

MAINTAINERCLEANFILES = aclocal.m4 config.guess config.sub configure \
	install-sh ltmain.sh Makefile.in missing mkinstalldirs stamp-h.in \
	libtool config.cache config.h config.h.in acinclude.m4 depcomp

maintainer-clean-local:
	-rm -rf m4

