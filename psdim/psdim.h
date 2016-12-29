/* Copyright (C) 2001-2012 Peter Selinger.
   This file is part of the upprint package. It is free software and
   is distributed under the terms of the GNU general public license.
   See the file COPYING for details. */

/* $Id: psdim.h 100 2012-03-27 23:02:39Z selinger $ */

#ifndef PSDIM_H
#define PSDIM_H

#include "main.h"

int readnum(FILE *f);
int psdim(char *infile, int n, bbox_t *bboxes, percentile_t *percentile);
int psdim_color(char *infile, int n, bbox_t *bboxes, percentile_t *percentile);

#endif /* PSDIM_H */
