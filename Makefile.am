SUFFIXES = .c .h .tmp .y .l .ini .pov .png .gif

ACLOCAL_AMFLAGS = -I m4

LFRAME = 99

AM_YFLAGS = -d

bin_PROGRAMS = x

x_SOURCES = x.c
x_LDFLAGS =
x_LDADD =

noinst_DATA = README.html TODO.html

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

test: x.tmp

x.tmp: x$(EXEEXT)
	./x$(EXEEXT) && $(TOUCH) $@

.md.html:
	$(PANDOC) -o $@ $< || echo "Install Pandoc to compile Markdown to HTML"

CLEANFILES = *.gif *.png *~ *.tmp *.html

DISTCLEANFILES = .*.un~ .*.swp

MAINTAINERCLEANFILES = aclocal.m4 config.guess config.sub configure \
	install-sh ltmain.sh Makefile.in missing mkinstalldirs stamp-h.in \
	libtool config.cache config.h config.h.in acinclude.m4 depcomp

maintainer-clean-local:
	-rm -rf m4

