SUFFIXES = .c .h .tmp .y .l .ini .pov .png .gif

ACLOCAL_AMFLAGS = -I m4

LFRAME = 99

AM_YFLAGS = -d

bin_PROGRAMS = x #blc spec_blc lambda spec_lambda spec_dsl

x_SOURCES = x.c
x_LDFLAGS =
x_LDADD =

# blc_SOURCES = blc_repl.c blc.c
# blc_LDFLAGS = 
# blc_LDADD =
# 
# spec_blc_SOURCES = spec_blc.c blc.c
# spec_blc_LDFLAGS =
# spec_blc_LDADD =
# 
# lambda_SOURCES = lambda_repl.c lambda.c lambda_bison.y lambda_flex.l blc.c
# lambda_LDFLAGS =
# lambda_LDADD =
# 
# spec_lambda_SOURCES = spec_lambda.c lambda.c lambda_bison.y lambda_flex.l blc.c
# spec_lambda_LDFLAGS =
# spec_lambda_LDADD =
# 
# spec_dsl_SOURCES = spec_dsl.c blc.c
# spec_dsl_LDFLAGS =
# spec_dsl_LDADD =

# noinst_HEADERS = blc.h lambda.h

noinst_DATA = README.html TODO.html

# BUILT_SOURCES = lambda_bison.h

EXTRA_DIST = Makefile.dist configure.ac README.md TODO.md COPYING run.sh blc.ini blcanim.ini blc.pov opensans.ttf

logo: blc.png blc.gif

blca.png: blc.ini blc.pov opensans.ttf
	$(POVRAY) blc

blca$(LFRAME).png: blc.ini blc.pov opensans.ttf
	$(POVRAY) blcanim

blc.png: blca.png
	$(CONVERT) blca.png -background white -flatten blc.png

blc$(LFRAME).png: blca$(LFRAME).png
	for i in $(shell seq -f '%02g' 0 $(LFRAME)); do \
		$(CONVERT) blca$$i.png -background white -flatten blc$$i.png; \
	done

blc.gif: blc$(LFRAME).png
	$(CONVERT) blc??.png -delay 4 -colors 32 +dither $@

# test: spec_blc.tmp spec_lambda.tmp spec_dsl.tmp
# test: spec_blc.tmp
test: x.tmp

x.tmp: x
	./x$(EXEEXT) && $(TOUCH) $@

spec_blc.tmp: spec_blc$(EXEEXT)
	./spec_blc$(EXEEXT) && $(TOUCH) $@

spec_lambda.tmp: spec_lambda$(EXEEXT)
	./spec_lambda$(EXEEXT) && $(TOUCH) $@

spec_dsl.tmp: spec_dsl$(EXEEXT)
	./spec_dsl$(EXEEXT) && $(TOUCH) $@

.md.html:
	$(PANDOC) -o $@ $< || echo "Install Pandoc to compile Markdown to HTML"

CLEANFILES = *.gif *.png *~ *.tmp *.html

MAINTAINERCLEANFILES = aclocal.m4 config.guess config.sub configure \
	install-sh ltmain.sh Makefile.in missing mkinstalldirs stamp-h.in \
	libtool config.cache config.h config.h.in acinclude.m4 depcomp \
	ylwrap

maintainer-clean-local:
	-rm -rf m4

