## Copyright (C) 2001-2012 Peter Selinger.
## This file is part of the upprint package. It is free software and
## is distributed under the terms of the GNU general public license.
## See the file COPYING for details.

s/</\&lt;/g
s/>/\&gt;/g
s/^\.TH \([^ ]*\) \([^ ]*\) "\([^"]*\)" .*$/<html><body bgcolor=#ffffff><title>Man page for \1(\2)<\/title><h1>Man page for \1(\2)<\/h1>/
s/^\.SH \(.*\)$/<h3>\1<\/h3>/
s/^\.SS \(.*\)$/<p><b>\1<\/b><p>/
s/^\.nf/<pre>/
s/^\.br/<br>/
s/^\.sp/<br><br>/
s/^\.fi/<\/pre><p>/
s/^\.TP \([0-9]*\)/<dl><dt>__sed_table__/
s/^\.TP$/<p><dt>__sed_table__/
s/^\.ce/__sed_center__/
/__sed_table__/N
/__sed_center__/N
s/^\.PD$/<\/dl>/
s/^\.PP$/<p>/
s/^\.RS \([0-9]*\)/\.RS/
s/^\.RS$/<dl><dt>\&nbsp;<\/dt>/
s/^\.RE$/<\/dl>/
s/^\.IP$/<table border=0><tr><td width=30><\/td><td>/
s/^\.LP$/<\/td><\/tr><\/table>/
s/^\.B \(.*\)$/<b>\1<\/b>/
s/__sed_table__\n\.B \(.*\)$/<b>\1<\/b><dd>/
s/__sed_table__\n\.I \(.*\)$/<i>\1<\/i><dd>/
s/__sed_table__\n\(.*\)$/\1<dd>/
s/__sed_center__\n\(.*\)$/<center>\1<\/center>/
s/__sed_table__/<!oops>/
s/__sed_center__/<!oops>/
s/^\.I \(.*\)$/<i>\1<\/i>/
s/^\.\\" \(.*\)/<!--\1-->/
s/^$/<p>/
s/\\-/-/g
s/\\fI\([^\\]*\)\\fP/<i>\1<\/i>/g
s/\\fB\([^\\]*\)\\fP/<b>\1<\/b>/g
s/\\fI\([^\\]*\)\\fR/<i>\1<\/i>/g
s/\\fB\([^\\]*\)\\fR/<b>\1<\/b>/g
s/\\fR//g
s/\\fP//g
s/\\"/__sed_quote__/g
s/"//g
s/__sed_quote__/"/g
s/\\'/'/g
s/\\-/-/g
s/\\+/+/g
s/\\(cq/'/g
s/\\(dq/"/g
