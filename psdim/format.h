/* Copyright (C) 2001-2012 Peter Selinger.
   This file is part of the upprint package. It is free software and
   is distributed under the terms of the GNU general public license.
   See the file COPYING for details. */

/* $Id: format.h 100 2012-03-27 23:02:39Z selinger $ */

#ifndef FORMAT_H
#define FORMAT_H

#include "main.h"

/* holds a linear transformation a la pstops */
struct transform_s {
  char *rot;   /* rotation, "", "L", "R", "U" */
  double sf;   /* scaling factor              */
  double dx;   /* x shift, in points          */
  double dy;   /* y shift, in points          */
  double x0p, y0p, x1p, y1p; /* clip path, in points */
};

typedef struct transform_s transform_t;

int format(info_t info, int n, bbox_t *bboxes);

#endif /* FORMAT_H */

