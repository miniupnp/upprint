## Copyright (C) 2001-2012 Peter Selinger.
## This file is part of the upprint package. It is free software and
## is distributed under the terms of the GNU general public license.
## See the file COPYING for details.

## Process this file with automake to produce Makefile.in

bin_PROGRAMS = psdim
EXTRA_DIST = getopt.c getopt1.c getopt.h

psdim_SOURCES = main.c main.h psdim.c psdim.h format.c format.h

psdim_LDADD = @EXTRA_OBJS@
psdim_DEPENDENCIES = @EXTRA_OBJS@
