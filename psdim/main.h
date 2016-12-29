/* Copyright (C) 2001-2012 Peter Selinger.
   This file is part of the upprint package. It is free software and
   is distributed under the terms of the GNU general public license.
   See the file COPYING for details. */

/* $Id: main.h 100 2012-03-27 23:02:39Z selinger $ */

#ifndef MAIN_H
#define MAIN_H

struct bbox_s {
  int x0, x1;  /* x-min and x-max */
  int y0, y1;  /* y-min and y-max */
};
typedef struct bbox_s bbox_t;

struct percentile_s {
  double x0, x1;  /* x-min and x-max */
  double y0, y1;  /* y-min and y-max */
};
typedef struct percentile_s percentile_t;

struct pageformat_s {
  char *name;
  double w, h;
};
typedef struct pageformat_s pageformat_t;

/* define a set of error conditions, in the style of errno. */

extern int merrno;  /* for passing error conditions */

#define ME_MEM                 1
#define ME_GSNOTFOUND          2
#define ME_EOF                 3
#define ME_POSTSCRIPT          4
#define ME_IO                  5  /* errno will be set */

/* alignment policies are as follows (hpolicy): 
   0 - coordinate origins are aligned vertically and evenly spaced horizontally
   1 - coordinate origins are aligned vertically. Pages are evenly spaced h.
   2 - page groups are centered vertically, evenly spaced horizontally 
   3 - coordinate origins are aligned vertically. Pages are unevenly spaced h.
   4 - page groups are centered vertically, unevenly spaced horizontally.
*/

struct info_s {
  double w, h;        /* width and height of output page */
  double hmargin, hsep; /* desired (outside) margin and (inside) separation */
  double vmargin, vsep; /* desired (outside) margin and (inside) separation */
  int land;           /* 0 upright, 1 landscape, 2 upside down, 3 seascape */
  int cols, rows;     /* columns, rows */
  int columnmode;     /* page numbers go in columns? */
  int righttoleft;    /* page numbers increase right to left? */
  int bottomtotop;    /* page numbers increase bottom to top? */
  int hpolicy;        /* horizontal alignment policy */
  int vpolicy;        /* vertical alignment policy */
  char *infile;       /* NULL for stdin */
  int quiet;          /* suppress stderr progress info? */
  int color;          /* handle non-white background colors? */
  int clip;           /* output page clipping instructions? */
  int shrink;         /* only shrink, never enlarge? */
  percentile_t percentile; /* percentiles for calculating bounding boxes */
  double ladjust, radjust; /* additional bounding box adjustment left, right */
  double tadjust, badjust; /* additional bounding box adjustment top, bottom */
};
typedef struct info_s info_t;

extern info_t info;

int license(FILE *f);
int usage(FILE *f);
double parse_dimension(char *s, char **endptr);
int dopts(int ac, char *av[]);
  
int main(int ac, char *av[]);

#endif /* MAIN_H */
