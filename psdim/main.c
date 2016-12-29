/* Copyright (C) 2001-2012 Peter Selinger.
   This file is part of the upprint package. It is free software and
   is distributed under the terms of the GNU general public license.
   See the file COPYING for details. */

/* $Id: main.c 100 2012-03-27 23:02:39Z selinger $ */

/* psdim: a small utility to be used in conjunction with pstops.  It
   uses GhostScript to determine the "actual" size of the printed
   pages in a document. It then figures out the optimal arrangement of
   pages in a grid for 2up printing or other such printing formats.
   The output is a format specification which can be passed to
   pstops. */

/* Requirements: ghostscript ("gs") must be installed. */

#ifdef HAVE_CONFIG_H
 #include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

#include "main.h"
#include "psdim.h"
#include "format.h"

info_t info;

/* define a set of error conditions, in the style of errno. */

int merrno;  /* for passing error conditions */

char *mstrerror[] = {
  /* 0 */    NULL,
  /* 1 */    "Out of memory", 
  /* 2 */    "Could not invoke ghostscript",
  /* 3 */    "Unexpected end of file from ghostscript",
  /* 4 */    "Postscript error",
  /* 5 */    "I/O error",  /* errno will be set */
};

/* dimensions of the various page formats, in postscript points */
pageformat_t pageformat[] = {
  { "a4",	 595,  842 },
  { "a3",        842, 1191 },
  { "a5",        421,  595 },
  { "b5",        516,  729 },
  { "letter",    612,  792 },
  { "legal",     612, 1008 },
  { "tabloid",   792, 1224 },
  { "statement", 396,  612 },
  { "executive", 540,  720 },
  { "folio",     612,  936 },
  { "quarto",    610,  780 },
  { "10x14",     720, 1008 },
  { NULL, 0, 0 },
};

#define MASK_LEFT   0x1
#define MASK_RIGHT  0x2
#define MASK_BOTTOM 0x4
#define MASK_TOP    0x8

int license(FILE *f) {
  fprintf(f, 
"This program is free software; you can redistribute it and/or modify\n"
"it under the terms of the GNU General Public License as published by\n"
"the Free Software Foundation; either version 2 of the License, or\n"
"(at your option) any later version.\n"
"\n"
"This program is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"GNU General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU General Public License\n"
"along with this program; if not, write to the Free Software\n"
"Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.\n"
);

  return 0;

}

int usage(FILE *f) {
  int i, j;

  fprintf(f, "Usage: "PSDIM" [options] [file]\n\n");
  fprintf(f, "Options:\n");
  fprintf(f, " -h, --help               - print this help message and exit\n");
  fprintf(f, " -v, --version            - print version info and exit\n");
  fprintf(f, " -l, --license            - print license info and exit\n");
  fprintf(f, " -q, --quiet              - suppress progress info on stderr\n");
  fprintf(f, " -x, --width <dim>        - output page width\n");
  fprintf(f, " -y, --height <dim>       - output page height\n");
  fprintf(f, " -p, --page <format>      - select output page size (e.g. letter, a4)\n");
  fprintf(f, " -m, --margin <dim>       - margin around page\n");
  fprintf(f, " -n, --hmargin <dim>      - horizontal margin (horizontal space)\n");
  fprintf(f, " -o, --vmargin <dim>      - vertical margin (vertical space)\n");
  fprintf(f, " -s, --sep <dim>          - separation between pages\n");
  fprintf(f, " -t, --hsep <dim>         - horizontal separation\n");
  fprintf(f, " -u, --vsep <dim>         - vertical separation\n");
  fprintf(f, " -L, --landscape          - landscape orientation\n");
  fprintf(f, " -R, --seascape           - seascape orientation\n");
  fprintf(f, " -U, --upside-down        - upside down orientation\n");
  fprintf(f, " -P, --portrait           - portrait orientation\n");
  fprintf(f, " -f, --format <n>x<m>     - arrange pages in n rows and m columns\n");
  fprintf(f, " -a, --hpolicy <n>        - horizontal alignment policy\n");
  fprintf(f, " -b, --vpolicy <n>        - vertical alignment policy\n");
  fprintf(f, " -c, --columnmode         - sort pages in columns\n");
  fprintf(f, " -d, --righttoleft        - page numbers increase right to left\n");
  fprintf(f, " -e, --bottomtotop        - page numbers increase bottom to top\n");
  fprintf(f, " -C, --color              - handle non-white backgrounds\n");
  fprintf(f, " -i, --clip               - output page clipping instructions for pstops-clip\n");
  fprintf(f, " -F, --fudge [L|R|T|B]<n> - percentage of pixels allowed out of bounds\n");
  fprintf(f, " -H, --ladjust <dim>      - adjust left edge of input's bounding box\n");
  fprintf(f, " -I, --radjust <dim>      - adjust right edge of input's bounding box\n");
  fprintf(f, " -J, --tadjust <dim>      - adjust top edge of input's bounding box\n");
  fprintf(f, " -K, --badjust <dim>      - adjust bottom edge of input's bounding box\n");
  fprintf(f, " -S, --shrink             - only shrink, never enlarge page\n");
  fprintf(f, " -1, --1up                - fit to size mode\n");
  fprintf(f, " -2, --2up                - 1x2 landscape mode\n");
  fprintf(f, " -4, --4up                - 2x2 portrait mode\n");
  fprintf(f, " -8, --8up                - 2x4 landscape mode\n");
  fprintf(f, " -9, --9up                - 3x3 portrait mode\n");
  fprintf(f, " -6, --16up               - 4x4 portrait mode\n");
  fprintf(f, "\n");
  fprintf(f, "Dimensions can have optional units, e.g. 6.5in, 15cm (default: "DEFAULT_UNIT_NAME").\n\n");

  j = fprintf(f, "Valid page formats are: ");
  for (i=0; pageformat[i].name!=NULL; i++) {
    if (j + strlen(pageformat[i].name) > 75) {
      fprintf(f, "\n");
      j = 0;
    }
    j += fprintf(f, "%s, ", pageformat[i].name);
  }
  fprintf(f, "(default: "
#ifdef USE_A4
	  "a4"
#else
	  "letter"
#endif
	  ").\n");
  
  return 0;
}

/* parse a dimension of the kind "1.5in", "7cm", etc. Return result in
   postscript points (=1/72 in). If endptr!=NULL, store pointer to
   next character in *endptr in the manner of strtod(3). */
double parse_dimension(char *s, char **endptr) {
  char *p;
  double x;

  x = strtod(s, &p);
  if (!*p) {
    x *= DEFAULT_UNIT_POINTS;
  } else if (!strcasecmp(p, "in")) {
    x *= 72;
    p += 2;
  } else if (!strcasecmp(p, "cm")) {
    x *= 72 / 2.54;
    p += 2;
  } else if (!strcasecmp(p, "mm")) {
    x *= 72 / 25.4;
    p += 2;
  } else if (!strcasecmp(p, "pt")) {
    p += 2;
  }
  if (endptr!=NULL) {
    *endptr = p;
  }
  return x;
}

static struct option longopts[] = {
  {"help",         0, 0, 'h'},
  {"version",      0, 0, 'v'},
  {"license",      0, 0, 'l'},
  {"quiet",        0, 0, 'q'},
  {"width",        1, 0, 'x'},
  {"height",       1, 0, 'y'},
  {"page",         1, 0, 'p'},
  {"margin",       1, 0, 'm'},
  {"hmargin",      1, 0, 'n'},
  {"vmargin",      1, 0, 'o'},
  {"sep",          1, 0, 's'},
  {"hsep",         1, 0, 't'},
  {"vsep",         1, 0, 'u'},
  {"ladjust",      1, 0, 'H'},
  {"radjust",      1, 0, 'I'},
  {"tadjust",      1, 0, 'J'},
  {"badjust",      1, 0, 'K'},
  {"landscape",    0, 0, 'L'},
  {"seascape",     0, 0, 'R'},
  {"upside-down",  0, 0, 'U'},
  {"portrait",     0, 0, 'P'},
  {"format",       1, 0, 'f'},
  {"hpolicy",      1, 0, 'a'},
  {"vpolicy",      1, 0, 'b'},
  {"columnmode",   0, 0, 'c'},
  {"righttoleft",  0, 0, 'd'},
  {"bottomtotop",  0, 0, 'e'},
  {"color",        0, 0, 'C'},
  {"clip",         0, 0, 'i'},
  {"fudge",        1, 0, 'F'},
  {"shrink",       0, 0, 'S'},
  {"1up",          0, 0, '1'},
  {"2up",          0, 0, '2'},
  {"4up",          0, 0, '4'},
  {"8up",          0, 0, '8'},
  {"9up",          0, 0, '9'},
  {"16up",         0, 0, '6'},
  {0, 0, 0, 0}
};

static char *shortopts = "hvlqx:y:p:m:n:o:s:t:u:LRUPf:a:b:cdeCiF:S124896H:I:J:K:";

int dopts(int ac, char *av[]) {
  int c, i, j;
  char *p;
  double fudge;
  int mask;

  /* defaults */
#ifdef USE_A4
  info.w = 595;       /* default page is a4 */
  info.h = 842;
#else
  info.w = 612;       /* default page is letter */
  info.h = 792;
#endif
  info.hmargin = 36;  /* default margin 0.5in */
  info.vmargin = 36;  /* default margin 0.5in */
  info.hsep = 36;     /* default sep 0.5in */
  info.vsep = 36;     /* default sep 0.5in */
  info.ladjust = 0;   /* default adjustment 0in */
  info.radjust = 0;   /* default adjustment 0in */
  info.tadjust = 0;   /* default adjustment 0in */
  info.badjust = 0;   /* default adjustment 0in */

  info.land = 1;      /* default: 2up */
  info.cols = 2;
  info.rows = 1;
  info.columnmode = 0;
  info.righttoleft = 0; 
  info.bottomtotop = 0; 
  info.hpolicy = 1;
  info.vpolicy = 1;
  info.quiet = 0;
  info.infile = NULL;
  info.percentile.x0 = 0.0;
  info.percentile.x1 = 1.0;
  info.percentile.y0 = 0.0;
  info.percentile.y1 = 1.0;

  info.color = 0;
  info.clip = 0;

  while ((c = getopt_long(ac, av, shortopts, longopts, NULL)) != -1) {
    switch (c) {
    case 'h':
      fprintf(stdout, ""PSDIM" "VERSION". Determines optimal page format for pstops.\n\n");
      usage(stdout);
      exit(0);
      break;
    case 'v':
      fprintf(stdout, ""PSDIM" "VERSION". Copyright (C) 2001-2012 Peter Selinger.\n");
      exit(0);
      break;
    case 'l':
      fprintf(stdout, ""PSDIM" "VERSION". Copyright (C) 2001-2012 Peter Selinger.\n\n");
      license(stdout);
      exit(0);
      break;
    case 'q':
      info.quiet = 1;
      break;
    case 'x':
      info.w = parse_dimension(optarg, &p);
      if (*p) {
	fprintf(stderr, ""PSDIM": invalid dimension -- %s\n", optarg);
	exit(1);
      }
      break;
    case 'y':
      info.h = parse_dimension(optarg, &p);
      if (*p) {
	fprintf(stderr, ""PSDIM": invalid dimension -- %s\n", optarg);
	exit(1);
      }
      break;
    case 'p':
      for (i=0; pageformat[i].name!=NULL; i++) {
	if (strcasecmp(pageformat[i].name, optarg)==0) 
	  break;
      }
      if (!pageformat[i].name) {
	fprintf(stderr, ""PSDIM": unrecognized page format -- %s\n", optarg);
	j = fprintf(stderr, "Use one of: ");
	for (i=0; pageformat[i].name!=NULL; i++) {
	  if (j + strlen(pageformat[i].name) > 75) {
	    fprintf(stderr, "\n");
	    j = 0;
	  }
	  j += fprintf(stderr, "%s, ", pageformat[i].name);
	}
	fprintf(stderr, "or use -x and -y options.\n");
	exit(1);
      }  
      info.w = pageformat[i].w;
      info.h = pageformat[i].h;
      break;
    case 'm':
      info.hmargin = info.vmargin = parse_dimension(optarg, &p);
      if (*p) {
	fprintf(stderr, ""PSDIM": invalid dimension -- %s\n", optarg);
	exit(1);
      }
      break;
    case 'n':
      info.hmargin = parse_dimension(optarg, &p);
      if (*p) {
	fprintf(stderr, ""PSDIM": invalid dimension -- %s\n", optarg);
	exit(1);
      }
      break;
    case 'o':
      info.vmargin = parse_dimension(optarg, &p);
      if (*p) {
	fprintf(stderr, ""PSDIM": invalid dimension -- %s\n", optarg);
	exit(1);
      }
      break;
    case 's':
      info.hsep = info.vsep = parse_dimension(optarg, &p);
      if (*p) {
	fprintf(stderr, ""PSDIM": invalid dimension -- %s\n", optarg);
	exit(1);
      }
      break;
    case 't':
      info.hsep = parse_dimension(optarg, &p);
      if (*p) {
	fprintf(stderr, ""PSDIM": invalid dimension -- %s\n", optarg);
	exit(1);
      }
      break;
    case 'u':
      info.vsep = parse_dimension(optarg, &p);
      if (*p) {
	fprintf(stderr, ""PSDIM": invalid dimension -- %s\n", optarg);
	exit(1);
      }
      break;
    case 'H':
      info.ladjust = parse_dimension(optarg, &p);
      if (*p) {
	fprintf(stderr, ""PSDIM": invalid dimension -- %s\n", optarg);
	exit(1);
      }
      break;
    case 'I':
      info.radjust = parse_dimension(optarg, &p);
      if (*p) {
	fprintf(stderr, ""PSDIM": invalid dimension -- %s\n", optarg);
	exit(1);
      }
      break;
    case 'J':
      info.tadjust = parse_dimension(optarg, &p);
      if (*p) {
	fprintf(stderr, ""PSDIM": invalid dimension -- %s\n", optarg);
	exit(1);
      }
      break;
    case 'K':
      info.badjust = parse_dimension(optarg, &p);
      if (*p) {
	fprintf(stderr, ""PSDIM": invalid dimension -- %s\n", optarg);
	exit(1);
      }
      break;
    case 'L':
      info.land = 1;
      break;
    case 'R':
      info.land = 3;
      break;
    case 'U':
      info.land = 2;
      break;
    case 'P':
      info.land = 0;
      break;
    case 'f':
      info.rows = strtol(optarg, &p, 10);
      if (*p == 'x' && info.rows>0) {
	info.cols = strtol(p+1, &p, 10);
	if (*p == '\0' && info.cols>0) {
	  break;
	}
      }
      fprintf(stderr, ""PSDIM": invalid format -- %s\n", optarg);
      fprintf(stderr, "Use <n>x<m>.\n");
      exit(1);
    case 'a':
      info.hpolicy = atoi(optarg);
      if (info.hpolicy<0 || info.hpolicy>4) {
	fprintf(stderr, ""PSDIM": invalid horizontal alignment policy -- %d\n", info.hpolicy);
	fprintf(stderr, "Valid range is 0--4.\n");
	exit(1);
      }
      break;
    case 'b':
      info.vpolicy = atoi(optarg);
      if (info.vpolicy<0 || info.vpolicy>4) {
	fprintf(stderr, ""PSDIM": invalid vertical alignment policy -- %d\n", info.vpolicy);
	fprintf(stderr, "Valid range is 0--4.\n");
	exit(1);
      }
      break;
    case 'c':
      info.columnmode = 1;
      break;
    case 'd':
      info.righttoleft = 1;
      break;
    case 'e':
      info.bottomtotop = 1;
      break;
    case 'C':
      info.color = 1;
      break;
    case 'i':
      info.clip = 1;
      break;
    case 'F': {
      p = optarg;
      mask = 0;
      while (1) {
	if (*p == 'l' || *p == 'L') {
	  mask |= MASK_LEFT;
	} else if (*p == 'r' || *p == 'R') {
	  mask |= MASK_RIGHT;
	} else if (*p == 'b' || *p == 'B' || *p == 'd' || *p == 'D') {
	  mask |= MASK_BOTTOM;
	} else if (*p == 't' || *p == 'T' || *p == 'u' || *p == 'U') {
	  mask |= MASK_TOP;
	} else {
	  break;
	}
	p += 1;
      }
      fudge = strtod(p, &p);
      if (*p || fudge < 0.0 || fudge > 100.0) {
	fprintf(stderr, ""PSDIM": invalid percentage -- %s\n", optarg);
	exit(1);
      }
      if (mask == 0 || mask & MASK_LEFT) {
	info.percentile.x0 = fudge * 0.01;
      }
      if (mask == 0 || mask & MASK_RIGHT) {
	info.percentile.x1 = 1.0 - fudge * 0.01;
      }
      if (mask == 0 || mask & MASK_BOTTOM) {
	info.percentile.y0 = fudge * 0.01;
      }
      if (mask == 0 || mask & MASK_TOP) {
	info.percentile.y1 = 1.0 - fudge * 0.01;
      }
      break;
    }
    case 'S':
      info.shrink = 1;
      break;
    case '1':
      info.land = 0;
      info.cols = 1;
      info.rows = 1;
      break;
    case '2':
      info.land = 1;
      info.cols = 2;
      info.rows = 1;
      break;
    case '4':
      info.land = 0;
      info.cols = 2;
      info.rows = 2;
      break;
    case '8':
      info.land = 1;
      info.cols = 4;
      info.rows = 2;
      break;
    case '9':
      info.land = 0;
      info.cols = 3;
      info.rows = 3;
      break;
    case '6':
      info.land = 0;
      info.cols = 4;
      info.rows = 4;
      break;
    case '?':
      fprintf(stderr, "Try --help for more info\n");
      exit(1);
      break;
    default:
      fprintf(stderr, ""PSDIM": Invalid option -- %c\n", c);
      exit(1);
    }
  }

  if (optind < ac) {
    info.infile = av[optind];
    optind++;
  }
  if (optind < ac) {
    fprintf(stderr, ""PSDIM": Too many non-option arguments\n");
    fprintf(stderr, "Try --help for more info\n");
    exit(1);
  }

  return 0;
}

void adjust(info_t info, int n, bbox_t *bboxes) {
  int i;

  for ( i=0; i<n; i++) {
#ifdef DEBUG
      fprintf(stderr, "### bbox(%d) was: x0=%d x1=%d y0=%d, y1=%d\n",
	      i, bboxes[i].x0, bboxes[i].x1, bboxes[i].y0, bboxes[i].y1 );
#endif
    bboxes[i].x0 += info.ladjust;
    bboxes[i].x1 -= info.radjust;
    bboxes[i].y0 += info.badjust;
    bboxes[i].y1 -= info.tadjust;
#ifdef DEBUG
      fprintf(stderr, "### bbox(%d) now: x0=%d x1=%d y0=%d, y1=%d\n",
	      i, bboxes[i].x0, bboxes[i].x1, bboxes[i].y0, bboxes[i].y1 );
#endif
  }
}
  
int main(int ac, char *av[]) {
  int n;
  int r;
  bbox_t *bboxes;

  /* read command line options */
  dopts(ac, av);
  
  n = info.rows * info.cols;

  /* allocate bounding boxes for n sets of pages */
  bboxes = (bbox_t *)malloc(n*sizeof(bbox_t));
  if (!bboxes) {
    fprintf(stderr, ""PSDIM": Out of memory\n");
    exit(1);
  }
  
  /* extract bounding boxes from file */
  if (info.color) {
    r = psdim_color(info.infile, n, bboxes, &info.percentile);
  } else {
    r = psdim(info.infile, n, bboxes, &info.percentile);
  }
  if (r == -1) {
    if (merrno != ME_IO) {
      fprintf(stderr, ""PSDIM": %s\n", mstrerror[merrno]);
    } else {
      fprintf(stderr, ""PSDIM": %s\n", strerror(errno));
    }      
    return merrno;
  }

  /* apply additional adjustment */
  adjust(info, n, bboxes);

  /* calculate and output best pstops format */
  format(info, n, bboxes);

  return 0;
}

