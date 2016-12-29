/* Copyright (C) 2001-2012 Peter Selinger.
   This file is part of the upprint package. It is free software and
   is distributed under the terms of the GNU general public license.
   See the file COPYING for details. */

/* $Id: format.c 100 2012-03-27 23:02:39Z selinger $ */

#ifdef HAVE_CONFIG_H
 #include "config.h"
#endif

#include <stdio.h>
#include <math.h>

#include "main.h"
#include "format.h"

#define min(a,b) (a<b ? a : b)
#define max(a,b) (a>b ? a : b)

#define INFTY 10000

int format(info_t info, int n, bbox_t *bboxes) {
  int xmin[info.cols], xmax[info.cols], dxmax[info.cols];
  int ymin[info.rows], ymax[info.rows], dymax[info.rows];
  int xmint, xmaxt, dxmaxt, dxmaxt2, dxmaxs, dxmaxs2;
  int ymint, ymaxt, dymaxt, dymaxt2, dymaxs, dymaxs2;
  double xoff[info.cols+1], yoff[info.rows+1];
  int totalw, totalh;
  transform_t tr[n];

  bbox_t *b;
  int i,j,k;
  double w, h;             /* effective width, height */
  double sf1, sf2, sf;     /* scaling factor */
  double padx, pady;

  int origin;              /* page on upper left corner */
  int incx, incy;          /* page increment x and y direction */

  double logical_x[n], logical_y[n];

  /* figure out page numbering */
  
  if (info.columnmode) {
    incx = info.righttoleft ? -info.rows : info.rows;
    incy = info.bottomtotop ? -1 : 1;
    origin = info.rows*(info.righttoleft ? info.cols : 1) 
      -(info.bottomtotop ? 1 : info.rows);
  } else {
    incx = info.righttoleft ? -1 : 1;
    incy = info.bottomtotop ? -info.cols : info.cols;
    origin = info.cols*(info.bottomtotop ? info.rows : 1)
      -(info.righttoleft ? 1 : info.cols);
  }

  /* first calculate aggregate widths and heights of rows and columns,
     according to various alignment policies */

  for (i=0; i<info.rows; i++) {
    ymin[i] = INFTY;
    ymax[i] = 0;
    dymax[i] = -1;
  }
  for (j=0; j<info.cols; j++) {
    xmin[j] = INFTY;
    xmax[j] = 0;
    dxmax[j] = -1;
  }

  for (i=0; i<info.rows; i++) {
    for (j=0; j<info.cols; j++) {

      b = &bboxes[origin+i*incy+j*incx];

      ymin[i]  = min(ymin[i],  b->y0);
      ymax[i]  = max(ymax[i],  b->y1);
      dymax[i] = max(dymax[i], b->y1 - b->y0);
      xmin[j]  = min(xmin[j],  b->x0);
      xmax[j]  = max(xmax[j],  b->x1);
      dxmax[j] = max(dxmax[j], b->x1 - b->x0);

#ifdef DEBUG
      fprintf(stderr, "### bbox(%d,%d): x0=%d x1=%d y0=%d y1=%d dx=%d dy=%d\n",
	      i, j, b->x0, b->x1, b->y0, b->y1, b->x1-b->x0, b->y1-b->y0);
#endif

    }
  }

  ymint   = xmint   = INFTY;
  ymaxt   = xmaxt   = 0;
  dymaxt  = dxmaxt  = -1;
  dymaxt2 = dxmaxt2 = -1;
  dymaxs  = dxmaxs  = 0;
  dymaxs2 = dxmaxs2 = 0;

  for (i=0; i<info.rows; i++) {
    ymint    = min(ymint,   ymin[i]);
    ymaxt    = max(ymaxt,   ymax[i]);
    dymaxt   = max(dymaxt,  ymax[i]-ymin[i]);
    dymaxt2  = max(dymaxt2, dymax[i]);
    dymaxs  += max(0, ymax[i]-ymin[i]);
    dymaxs2 += max(0, dymax[i]);

#ifdef DEBUG
      fprintf(stderr, "### row(%d): y0=%d y1=%d dy=%d, dymax=%d\n",
	      i, ymin[i], ymax[i], ymax[i]-ymin[i], dymax[i]);
#endif

  }    
  for (j=0; j<info.cols; j++) {
    xmint    = min(xmint,   xmin[j]);
    xmaxt    = max(xmaxt,   xmax[j]);
    dxmaxt   = max(dxmaxt,  xmax[j]-xmin[j]);
    dxmaxt2  = max(dxmaxt2, dxmax[j]);
    dxmaxs  += max(0, xmax[j]-xmin[j]);
    dxmaxs2 += max(0, dxmax[j]);

#ifdef DEBUG
      fprintf(stderr, "### col(%d): x0=%d x1=%d dx=%d, dxmax=%d\n",
	      j, xmin[j], xmax[j], xmax[j]-xmin[j], dxmax[j]);
#endif

  }    

#ifdef DEBUG
  fprintf(stderr, "### total: x0=%d x1=%d dx=%d dxx=%d dxxx=%d dxs=%d dxxs=%d\n", xmint, xmaxt, xmaxt-xmint, dxmaxt, dxmaxt2, dxmaxs, dxmaxs2);
  fprintf(stderr, "### total: y0=%d y1=%d dy=%d dyx=%d dyxx=%d dys=%d dyxs=%d\n", ymint, ymaxt, ymaxt-ymint, dymaxt, dymaxt2, dymaxs, dymaxs2);
#endif

  switch(info.hpolicy) {
  case 0: default:
    totalw = info.cols * (xmaxt-xmint);
    break;
  case 1:
    totalw = info.cols * dxmaxt;
    break;
  case 2:
    totalw = info.cols * dxmaxt2;
    break;
  case 3:
    totalw = dxmaxs;
    break;
  case 4:
    totalw = dxmaxs2;
    break;
  }

  switch(info.vpolicy) {
  case 0: default:
    totalh = info.rows * (ymaxt-ymint);
    break;
  case 1:
    totalh = info.rows * dymaxt;
    break;
  case 2:
    totalh = info.rows * dymaxt2;
    break;
  case 3:
    totalh = dymaxs;
    break;
  case 4:
    totalh = dymaxs2;
    break;
  }

  /* next, calculate the optimal scaling factor */

  w = (info.land & 1) ? info.h : info.w;
  h = (info.land & 1) ? info.w : info.h;

  sf1 = (w - 2*info.hmargin - (info.cols-1)*info.hsep) / totalw;
  sf2 = (h - 2*info.vmargin - (info.rows-1)*info.vsep) / totalh;
  sf = min(sf1, sf2);
  sf = floor(1000.0 * sf) / 1000.0;   /* round down to multiple of 1/1000 */

  if (info.shrink) {
    sf = min(sf, 1.0);
  }

  /* calculate the extra padding between cells */
  padx = (w - sf*totalw - 2*info.hmargin - (info.cols-1)*info.hsep) / (info.cols+1);
  pady = (h - sf*totalh - 2*info.vmargin - (info.rows-1)*info.vsep) / (info.rows+1);

#ifdef DEBUG
  fprintf(stderr, "### padx=%f pady=%f\n", padx, pady);
#endif

  /* calculate the offset of each row and column */

  xoff[0] = info.hmargin + padx;
  for (j=0; j<info.cols; j++) {
    switch(info.hpolicy) {
    case 0: default:
      xoff[j+1] = xoff[j] + sf*(xmaxt-xmint);
      break;
    case 1: 
      xoff[j+1] = xoff[j] + sf*dxmaxt;
      break;
    case 2: 
      xoff[j+1] = xoff[j] + sf*dxmaxt2;
      break;
    case 3: 
      xoff[j+1] = xoff[j] + sf*max(0, xmax[j]-xmin[j]);
      break;
    case 4: 
      xoff[j+1] = xoff[j] + sf*max(0, dxmax[j]);
      break;
    }      
    xoff[j+1] +=  info.hsep + padx;
  }
  
  yoff[0] = h - info.vmargin - pady;
  for (i=0; i<info.rows; i++) {
    switch(info.vpolicy) {
    case 0: default:
      yoff[i+1] = yoff[i] - sf*(ymaxt-ymint);
      break;
    case 1: 
      yoff[i+1] = yoff[i] - sf*dymaxt;
      break;
    case 2: 
      yoff[i+1] = yoff[i] - sf*dymaxt2;
      break;
    case 3: 
      yoff[i+1] = yoff[i] - sf*max(0, ymax[i]-ymin[i]);
      break;
    case 4: 
      yoff[i+1] = yoff[i] - sf*max(0, dymax[i]);
      break;
    }      
    yoff[i+1] -=  info.vsep + pady;
  }
  
  /* now calculate the optimal placement of each cell */
  for (i=0; i<info.rows; i++) {
    for (j=0; j<info.cols; j++) {
      int x0, x1, y0, y1;
      double x, y;

      k = origin+i*incy+j*incx;

      b = &bboxes[k];
      switch(info.hpolicy) {
      case 0: default:
	x0 = xmint;
	x1 = xmaxt;
	break;
      case 1: case 3:
	x0 = xmax[j];
	x1 = xmin[j];
	break;
      case 2: case 4:
	x0 = b->x0;
	x1 = b->x1;
	break;
      }
      switch(info.vpolicy) {
      case 0: default:
	y0 = ymint;
	y1 = ymaxt;
	break;
      case 1: case 3:
	y0 = ymax[i];
	y1 = ymin[i];
	break;
      case 2: case 4:
	y0 = b->y0;
	y1 = b->y1;
	break;
      }	
      x = (xoff[j]+xoff[j+1]-info.hsep-padx)/2-sf*(x0+x1)/2;
      y = (yoff[i]+yoff[i+1]+info.vsep+pady)/2-sf*(y0+y1)/2;

      logical_x[k] = x;
      logical_y[k] = y;  /* remember these values for later */
      switch(info.land) {
      case 0: default:
	tr[k].rot = "";
	tr[k].sf = sf;
	tr[k].dx = x;
	tr[k].dy = y;
	break;
      case 1:
	tr[k].rot = "L";
	tr[k].sf = sf;
	tr[k].dx = -y+info.w;
	tr[k].dy = x;
	break;
      case 2:
	tr[k].rot = "U";
	tr[k].sf = sf;
	tr[k].dx = -x+info.w;
	tr[k].dy = -y+info.h;
	break;
      case 3:
	tr[k].rot = "R";
	tr[k].sf = sf;
	tr[k].dx = y;
	tr[k].dy = -x+info.h;
	break;
      }
    }
  }

  /* calculate the clippath for each page */
  if (info.clip) {
    double sf_inv;  /* inverse scale factor */

    if (sf > 0) {
      sf_inv = 1.0 / sf;
    } else {
      sf_inv = 0.0;
    }
    
    for (i=0; i<info.rows; i++) {
      for (j=0; j<info.cols; j++) {
	k = origin+i*incy+j*incx;
	if (j==0) {
	  tr[k].x0p = (-logical_x[k]) * sf_inv;
	} else {
	  tr[k].x0p = (xoff[j]-(info.hsep+padx)/2.0-logical_x[k]) * sf_inv;
	}
	if (j==info.cols-1) {
	  tr[k].x1p = (w-logical_x[k]) * sf_inv;
	} else {
	  tr[k].x1p = (xoff[j+1]-(info.hsep+padx)/2.0-logical_x[k]) * sf_inv;
	}
	if (i==info.rows-1) {
	  tr[k].y0p = (-logical_y[k]) * sf_inv;
	} else {
	  tr[k].y0p = (yoff[i+1]+(info.vsep+pady)/2.0-logical_y[k]) * sf_inv;
	}
	if (i==0) {
	  tr[k].y1p = (h-logical_y[k]) * sf_inv;
	} else {
	  tr[k].y1p = (yoff[i]+(info.vsep+pady)/2.0-logical_y[k]) * sf_inv;
	}
      }
    }
  }

  /* output the results in order */

  printf("%d:", n);
  for (k=0; k<n; k++) {
    printf("%s%d@%0.3f%s(%0.3f"DEFAULT_UNIT_NAME",%0.3f"DEFAULT_UNIT_NAME")", k ? "+" : "", k, tr[k].sf,
	   tr[k].rot, tr[k].dx/DEFAULT_UNIT_POINTS, tr[k].dy/DEFAULT_UNIT_POINTS);
    if (info.clip) {
      printf("{%0.3f"DEFAULT_UNIT_NAME",%0.3f"DEFAULT_UNIT_NAME",%0.3f"DEFAULT_UNIT_NAME",%0.3f"DEFAULT_UNIT_NAME"}", tr[k].x0p/DEFAULT_UNIT_POINTS, tr[k].y0p/DEFAULT_UNIT_POINTS, tr[k].x1p/DEFAULT_UNIT_POINTS, tr[k].y1p/DEFAULT_UNIT_POINTS);
    }
  }
  printf("\n");

  return 0;
}			
