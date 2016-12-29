## Copyright (C) 2001-2012 Peter Selinger.
## This file is part of the upprint package. It is free software and
## is distributed under the terms of the GNU general public license.
## See the file COPYING for details.

man_MANS = psdim.1 pstops-clip.1 lprwrap.1
HTMLDOC = psdim.html pstops-clip.html lprwrap.html
DISTCLEANFILES = $(HTMLDOC)
EXTRA_DIST = man2html.sed

html-doc: $(HTMLDOC)

psdim.html: psdim.1 $(srcdir)/man2html.sed
	cat "$<" | sed -f $(srcdir)/man2html.sed > "$@"

pstops-clip.html: pstops-clip.1 $(srcdir)/man2html.sed
	cat "$<" | sed -f $(srcdir)/man2html.sed > "$@"

lprwrap.html: lprwrap.1 $(srcdir)/man2html.sed
	cat "$<" | sed -f $(srcdir)/man2html.sed > "$@"

