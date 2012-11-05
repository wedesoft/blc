SUFFIXES = .c .h .tmp

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = tokenizer_spec bracket_spec boot-eval

tokenizer_spec_SOURCES = tokenizer_spec.c tokenizer.c
tokenizer_spec_LDFLAGS =
tokenizer_spec_LDADD =

bracket_spec_SOURCES = bracket_spec.c boot-eval.c tokenizer.c
bracket_spec_LDFLAGS =
bracket_spec_LDADD =

boot_eval_SOURCES = repl.c boot-eval.c tokenizer.c
boot_eval_LDFLAGS = 
boot_eval_LDADD =

noinst_HEADERS = tokenizer.h boot-eval.h

BUILT_SOURCES =

EXTRA_DIST = Makefile.dist configure.ac README.md COPYING TODO \
	test_tokenize_in.txt test_tokenize_out.txt \
	test_boot_in.txt test_boot_out.txt

test: tokenizer_spec.tmp bracket_spec.tmp test_boot.tmp

tokenizer_spec.tmp: tokenizer_spec$(EXEEXT)
	./tokenizer_spec$(EXEEXT) && $(TOUCH) $@

bracket_spec.tmp: bracket_spec$(EXEEXT)
	./bracket_spec$(EXEEXT) && $(TOUCH) $@

test_boot.tmp: boot-eval$(EXEEXT) boot.rkt test_boot_in.txt test_boot_out.txt
	$(CAT) test_boot_in.txt | ./boot-eval$(EXEEXT) > out.tmp
	$(DIFF) test_boot_out.txt out.tmp && $(TOUCH) $@

CLEANFILES = *~ *.tmp

MAINTAINERCLEANFILES = aclocal.m4 config.guess config.sub configure \
	install-sh ltmain.sh Makefile.in missing mkinstalldirs stamp-h.in \
	libtool config.cache config.h config.h.in acinclude.m4 depcomp

