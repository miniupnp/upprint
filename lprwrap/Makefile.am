## Copyright (C) 2001-2012 Peter Selinger.
## This file is part of the upprint package. It is free software and
## is distributed under the terms of the GNU general public license.
## See the file COPYING for details.

## Process this file with automake to produce Makefile.in

bin_SCRIPTS = lprwrap
CLEANFILES = $(bin_SCRIPTS)

# for backward compatibility: if lprwrap-cups exists, replace it by a
# symbolic link to lprwrap.

install-exec-hook:
	if [ -f $(DESTDIR)$(bindir)/lprwrap-cups ]; then \
	 rm -f $(DESTDIR)$(bindir)/lprwrap-cups; \
	 ln -s lprwrap $(DESTDIR)$(bindir)/lprwrap-cups; \
	fi

uninstall-local:
	rm -f $(DESTDIR)$(bindir)/lprwrap-cups
