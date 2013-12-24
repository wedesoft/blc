SUFFIXES = .c .h .tmp .y .l

AM_YFLAGS = -d

bin_PROGRAMS = x

x_SOURCES = x.c
x_LDFLAGS =
x_LDADD =

test: x.tmp

x.tmp: x$(EXEEXT)
	./x$(EXEEXT) && $(TOUCH) $@

CLEANFILES = *~ *.tmp

DISTCLEANFILES = .*.un~ .*.swp

MAINTAINERCLEANFILES = Makefile.in
