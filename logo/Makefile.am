SUFFIXES = .tmp .ini .pov .png .gif

ACLOCAL_AMFLAGS = -I m4

LFRAME = 99

EXTRA_DIST = blc.ini blcanim.ini blc.pov opensans.ttf

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

CLEANFILES = *~ *.gif *.png *~ *.tmp

DISTCLEANFILES = .*.un~ .*.swp

MAINTAINERCLEANFILES = Makefile.in
