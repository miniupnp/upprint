/* Copyright (C) 2001-2012 Peter Selinger.
   This file is part of the upprint package. It is free software and
   is distributed under the terms of the GNU general public license.
   See the file COPYING for details. */

/* $Id: psdim.c 100 2012-03-27 23:02:39Z selinger $ */

#ifdef HAVE_CONFIG_H
 #include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "main.h"
#include "psdim.h"

/* x1list[n] is the index of the leftmost bit in the binary
   representation of n. x2list[n] is the index of the rightmost
   bit. */

int x1list[] = { 8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 
		 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
		 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
		 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
		 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }; 

int x2list[] = { 0, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8, 
		 4, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8, 
		 3, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8, 
		 4, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8, 
		 2, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8, 
		 4, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8, 
		 3, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8, 
		 4, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8, 
		 1, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8, 
		 4, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8, 
		 3, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8, 
		 4, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8, 
		 2, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8, 
		 4, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8, 
		 3, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8, 
		 4, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8, };

char *strcat_safe(char *dest, const char *src) {

  dest = realloc(dest, strlen(dest)+strlen(src)+1);
  if (dest) {
    strcat(dest, src);
  }
  return dest;
}

/* skip whitespace and comments, then read a non-negative decimal
   number from a stream. Return -1 on EOF. Tolerate other errors (skip
   bad characters). Also read the immediately following character, or
   to the end of the line if next character is a comment character. */

int readnum(FILE *f) {
  int c;
  int acc;

  /* skip whitespace and comments */
  while (1) {
    c = fgetc(f);
    if (c=='#') {
      while (1) {
	c = fgetc(f);
	if (c=='\n' || c==EOF)
	  break;
      }
    }
    if (c==EOF)
      return -1;
    if (c>='0' && c<='9') 
      break;
  }

  /* first digit is already in c */
  acc = c-'0';
  while (1) {
    c = fgetc(f);
    if (c=='#') {
      while (1) {
	c = fgetc(f);
	if (c=='\n' || c==EOF)
	  break;
      }
    }
    if (c<'0' || c>'9')
      break;
    acc *= 10;
    acc += c-'0';
  }
  return acc;
}

/* read zero or more portable bitmaps from GS and figure out the
   dimension of their printed area. This information is collected for
   all pages modulo n, e.g., if n=2, the info is summarized separately
   for even and odd pages. Return 0 on success, or -1 with merrno
   set. */

int psdim(char *infile, int n, bbox_t *bboxes, percentile_t *percentile) {
  FILE *f;
  char *cmd;
  int magic[2];
  int y, i, j, b, p, c, k;
  int w, h;          /* width, height in pixels */
  int bpr;           /* bytes per row */
  int rowcount[n][1008];
  int colcount[n][1008];
  int px0, px1, py0, py1;
  int count;
  int fdin, r;
  struct stat st;
  int top, bot;

  if (infile) {
    /* open infile */
    fdin = open(infile, O_RDONLY);
    if (fdin == -1) {
      merrno = ME_IO;
      return -1;
    }
    /* check that it's not a directory */
    fstat(fdin, &st);
    if (S_ISDIR(st.st_mode)) {
      errno = EISDIR;
      merrno = ME_IO;
      return -1;
    }
    /* connect infile to stdin */
    r = dup2(fdin, 0);
    if (r == -1) {
      merrno = ME_IO;
      return -1;
    }
  }

  cmd = strdup(""GS" -q -dNOPAUSE -sDEVICE=pbmraw -g1008x1008 -sOutputFile=- -");

  f = popen(cmd, "r");

  free(cmd);

  if (!f) {
    merrno = ME_GSNOTFOUND;
    return -1;
  }

  memset(rowcount, 0, sizeof(rowcount));
  memset(colcount, 0, sizeof(rowcount));

  j = 0;  /* page counter mod n */
  p = 1;  /* page counter */
  c = 0;  /* column of stderr output */

  while(1) {
    
    magic[0] = fgetc(f);
    if (magic[0] == EOF) {
      goto eof;
    }    

    if (magic[0] != 'P') {
      goto format_error;
    }    

    magic[1] = fgetc(f);
    if (magic[1] != '4') {
      goto format_error;
    }
    
    w = readnum(f);
    if (w<0) {
      goto format_error;
    }    

    h = readnum(f);
    if (h<0) {
      goto format_error;
    }
    
    /* read P4 format */
    
    bpr = 1+(w-1)/8;
    
    for (y=h-1; y>=0; y--) {
      for (i=0; i<bpr; i++) {
	b = fgetc(f);
	if (b==EOF) {
	  goto eof_error;
	}
	if (b==0) {
	  continue;
	}
	if (y < 1008 && 8*i < 1008) {
	  for (k=0; k<8; k++) {
	    if (b & (0x80 >> k)) {
	      rowcount[j][y]++;
	      colcount[j][8*i+k]++;
	    }
	  }
	}
      }
    }

    if (!info.quiet) {
      c += fprintf(stderr, "[%d] ", p);
      if (c >= 75) {
	fprintf(stderr, "\n");
	c = 0;
      }
    }

    j = (j+1) % n;
    p++;
  } /* while(1) */

 eof_error:
  pclose(f);
  if (!info.quiet && c!=0) {
    fprintf(stderr, "\n");
  }
  merrno = ME_EOF;
  return -1;

 format_error:
  pclose(f);
  if (!info.quiet && c!=0) {
    fprintf(stderr, "\n");
  }
  merrno = ME_POSTSCRIPT;
  return -1;
  
 eof:
  if (!info.quiet && c!=0) {
    fprintf(stderr, "\n");
  }
  pclose(f);
  /* note: we now ignore the return value of pclose, as this was
     sometimes -1 (with errno=ECHILD) although everything worked fine. */

  /* figure out bounding boxes from row/column counts for each page set */

  for (j=0; j<n; j++) {
    count = 0;   /* total pixels this page */
    for (i=0; i<1008; i++) {
      count += rowcount[j][i];
    }
    if (count == 0) { /* no pixels */
      bboxes[j].x0 = 1008;
      bboxes[j].y0 = 1008;
      bboxes[j].x1 = 0;
      bboxes[j].y1 = 0;
    } else {
      /* percentiles */
      px0 = (int)floor(count * percentile->x0);
      px1 = (int)ceil(count * percentile->x1);
      py0 = (int)floor(count * percentile->y0);
      py1 = (int)ceil(count * percentile->y1);

      count = 0;
      bboxes[j].x0 = 0;
      bboxes[j].x1 = 0;
      for (i=0; i<1008; i++) {
	count += colcount[j][i];
	if (count <= px0) {
	  bboxes[j].x0 = i+1;
	}
	if (count < px1) {
	  bboxes[j].x1 = i+2;
	}
      }
      count = 0;
      bboxes[j].y0 = 0;
      bboxes[j].y1 = 0;
      for (i=0; i<1008; i++) {
	count += rowcount[j][i];
	if (count <= py0) {
	  bboxes[j].y0 = i+1;
	}
	if (count < py1) {
	  bboxes[j].y1 = i+2;
	}
      }
    }
  }

  /* handle the special case of certain PostScript files produced by
     Acrobat Reader that anchor the page in the top left corner of the
     canvas, instead of the bottom left corner. The only way to detect
     this for sure would be to render the image twice. Here we use a
     heuristic. */
  top = 0;
  bot = 1008;
  for (i=0; i<1008; i++) {
    for (j=0; j<n; j++) {
      if (rowcount[j][i] != 0) {
	top = i;
	if (bot == 1008) {
	  bot = i;
	}
      }
    }
  }
  if (top >= 842 && bot > 1008-842) {
    for (j=0; j<n; j++) {
      bboxes[j].y0 -= 1008 - 792;
      bboxes[j].y1 -= 1008 - 792;
    }
  }
  
  return 0;
}

/* like psdim, except use colored bitmaps. Return 0 on error, else -1
   with merrno set. NOTE: this calculates the right dimensions;
   however, pstops does not work well on colored backgrounds due to
   stupid cropping. */

int psdim_color(char *infile, int n, bbox_t *bboxes, percentile_t *percentile) {
  FILE *f;
  char *cmd;
  int magic[2];
  int y, x, i, j, b, p, c;
  int maxval, bpp;
  int w, h;          /* width, height in pixels */
  int ink, first;
  unsigned char pix[6];
  int rowcount[n][1008];
  int colcount[n][1008];
  int px0, px1, py0, py1;
  int count;
  int fdin, r;
  int top, bot;
  struct stat st;

  if (infile) {
    /* open infile */
    fdin = open(infile, O_RDONLY);
    if (fdin == -1) {
      merrno = ME_IO;
      return -1;
    }
    /* check that it's not a directory */
    fstat(fdin, &st);
    if (S_ISDIR(st.st_mode)) {
      errno = EISDIR;
      merrno = ME_IO;
      return -1;
    }
    /* connect infile to stdin */
    r = dup2(fdin, 0);
    if (r == -1) {
      merrno = ME_IO;
      return -1;
    }
  }

  cmd = strdup(""GS" -q -dNOPAUSE -sDEVICE=ppmraw -g1008x1008 -sOutputFile=- -");

  f = popen(cmd, "r");

  free(cmd);

  if (!f) {
    merrno = ME_GSNOTFOUND;
    return -1;
  }

  memset(rowcount, 0, sizeof(rowcount));
  memset(colcount, 0, sizeof(rowcount));

  j = 0;  /* page counter mod n */
  p = 1;  /* page counter */
  c = 0;  /* column of stderr output */

  while(1) {
    
    magic[0] = fgetc(f);
    if (magic[0] == EOF)
      goto eof;
    
    if (magic[0] != 'P')
      goto format_error;
    
    magic[1] = fgetc(f);
    if (magic[1] != '6')
      goto format_error;
    
    w = readnum(f);
    if (w<0)
      goto format_error;
    
    h = readnum(f);
    if (h<0)
      goto format_error;

    maxval = readnum(f);
    if (maxval<1 || maxval>=65536)
      goto format_error;

    bpp = (maxval >= 256) ? 6 : 3;   /* bytes per pixel */
    
    /* read P6 format */
    
    first = 1;
    for (y=h-1; y>=0; y--) {
      for (x=0; x<w; x++) {
	if (first) {
	  for (i=0; i<bpp; i++) {
	    b = fgetc(f);
	    if (b==EOF)
	      goto eof_error;
	    pix[i] = b;
	  }
	  first = 0;
	} else {
	  ink = 0;
	  for (i=0; i<bpp; i++) {
	    b = fgetc(f);
	    if (b==EOF)
	      goto eof_error;
	    if (b != pix[i]) {
	      ink = 1;
	    }
	  }
	  if (ink) {
	    if (y < 1008 && x < 1008) {
	      rowcount[j][y]++;
	      colcount[j][x]++;
	    }
	  }
	}
      }
    }

    if (!info.quiet) {
      c += fprintf(stderr, "[%d] ", p);
      if (c >= 75) {
	fprintf(stderr, "\n");
	c = 0;
      }
    }

    j = (j+1) % n;
    p++;
  } /* while(1) */

 eof_error:
  pclose(f);
  if (!info.quiet && c!=0) {
    fprintf(stderr, "\n");
  }
  merrno = ME_EOF;
  return -1;

 format_error:
  pclose(f);
  if (!info.quiet && c!=0) {
    fprintf(stderr, "\n");
  }
  merrno = ME_POSTSCRIPT;
  return -1;
  
 eof:
  if (!info.quiet && c!=0) {
    fprintf(stderr, "\n");
  }
  pclose(f);

  /* figure out bounding boxes from row/column counts for each page set */

  for (j=0; j<n; j++) {
    count = 0;   /* total pixels this page */
    for (i=0; i<1008; i++) {
      count += rowcount[j][i];
    }
    if (count == 0) { /* no pixels */
      bboxes[j].x0 = 1008;
      bboxes[j].y0 = 1008;
      bboxes[j].x1 = 0;
      bboxes[j].y1 = 0;
    } else {
      /* percentiles */
      px0 = (int)floor(count * percentile->x0);
      px1 = (int)ceil(count * percentile->x1);
      py0 = (int)floor(count * percentile->y0);
      py1 = (int)ceil(count * percentile->y1);

      count = 0;
      bboxes[j].x0 = 0;
      bboxes[j].x1 = 0;
      for (i=0; i<1008; i++) {
	count += colcount[j][i];
	if (count <= px0) {
	  bboxes[j].x0 = i+1;
	}
	if (count < px1) {
	  bboxes[j].x1 = i+2;
	}
      }
      count = 0;
      bboxes[j].y0 = 0;
      bboxes[j].y1 = 0;
      for (i=0; i<1008; i++) {
	count += rowcount[j][i];
	if (count <= py0) {
	  bboxes[j].y0 = i+1;
	}
	if (count < py1) {
	  bboxes[j].y1 = i+2;
	}
      }
    }
  }

  /* handle the special case of certain PostScript files produced by
     Acrobat Reader that anchor the page in the top left corner of the
     canvas, instead of the bottom left corner. The only way to detect
     this for sure would be to render the image twice. Here we use a
     heuristic. */
  top = 0;
  bot = 1008;
  for (i=0; i<1008; i++) {
    for (j=0; j<n; j++) {
      if (rowcount[j][i] != 0) {
        top = i;
        if (bot == 1008) {
          bot = i;
        }
      }
    }
  }
  if (top >= 842 && bot > 1008-842) {
    for (j=0; j<n; j++) {
      bboxes[j].y0 -= 1008 - 792;
      bboxes[j].y1 -= 1008 - 792;
    }
  }
  return 0;
}

