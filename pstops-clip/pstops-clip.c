/* pstops.c
 * Copyright (C) Angus J. C. Duggan 1991-1995
 * Copyright (C) Peter Selinger 2006-2012
 * See file LICENSE for details.
 *
 * rearrange pages in conforming PS file for printing in signatures
 *
 * Usage:
 *       pstops [-q] [-b] [-d] [-w<dim>] [-h<dim>] [-ppaper] <pagespecs> [infile [outfile]]
 */

#ifdef HAVE_CONFIG_H
 #include "config.h"
#endif

#include <string.h>
#include <stdio.h>
#include <libgen.h>

#include "psutil.h"
#include "psspec.h"
#include "pserror.h"

char *program ;
int pages ;
int verbose ;
FILE *infile ;
FILE *outfile ;
char pagelabel[BUFSIZ] ;
int pageno ;

static void license(FILE *f) {
   fprintf(f, 
"%s "VERSION".\n"
"Copyright (C) 1991-1995 Angus J. C. Duggan.\n"
"Copyright (C) 2006-2012 Peter Selinger.\n"
"\n"
"This program may be copied and used for any purpose (including\n"
"distribution as part of a for-profit product), provided:\n"
"\n"
"1) The original attribution of the programs is clearly displayed in\n"
"   the product and/or documentation, even if the programs are modified\n"
"   and/or renamed as part of the product.\n"
"\n"
"2) The original source code of the programs is provided free of charge\n"
"   (except for reasonable distribution costs). For a definition of\n"
"   reasonable distribution costs, see the Gnu General Public License\n"
"   or Larry Wall's Artistic License (provided with the Perl 4 kit). The\n"
"   GPL and Artistic License in NO WAY affect this license; they are merely\n"
"   used as examples of the spirit in which it is intended.\n"
"\n"
"3) These programs are provided \"as-is\". No warranty or guarantee of\n"
"   their fitness for any particular task is provided. Use of these\n"
"   programs is completely at your own risk.\n"
	   , program);
}

static void usage(FILE *f) {
   fprintf(f, 
"pstops-clip "VERSION". Rearranges pages from a PostScript document.\n"
"\n"
"Usage: %s [options] <pagespecs> [infile [outfile]]\n"
"Options:\n"
" --help               - print this help message and exit\n"
" --version, -v        - print version info and exit\n"
" --license            - print license info and exit\n"
" -q                   - don't print page numbers to stderr\n"
" -b                   - prevent PostScript 'bind' operators from binding\n"
" -w<width>            - set page width\n"
" -h<height>           - set page height\n"
" -p<paper>            - set paper size\n"
" -d[linewidth]        - draw a box around each page\n"
"\n"
"Paper sizes:\n"
" a3, a4, a5, b5, letter, legal, tabloid, statement, executive, folio,\n"
" quarto, 10x14, _glibc\n"
"\n"
"Page specification:\n"
" <pagespecs>  = [modulo:]<spec>[,spec|+spec]...\n"
" <spec>       = [-]pageno[@scale][L|R|U][(xoff,yoff)][{x0,y0,x1,y1}]\n"
" modulo:      the number of pages in each block (1 or more; default 1)\n"
" spec:        the page specification for one page in each block\n"
" pageno:      between 0 and modulo-1\n"
" scale:       scaling factor\n"
" L,R,U:       rotate left, right, upside-down\n"
" xoff,yoff:   page origin in PostScript points, or units of cm, in, w, h\n"
" x0,y1,x1,y1: page clip path in PostScript points, or units of cm, in, w, h\n"
"\n"
	   , program);
}

void version(FILE *f)
{
   fprintf(f, 
"%s "VERSION".\n"
"Copyright (C) Angus J. C. Duggan, 1991-1995. See --license for details.\n"
"Copyright (C) 2006-2012 Peter Selinger.\n"
	   , program);
}

/* this can be used as an error handling callback function; does not return */
void shortusage(void)
{
   fprintf(stderr, "Illegal command line. Try --help for more info.\n");
   fflush(stderr);
   exit(1);
}

static int modulo = 1;
static int pagesperspec = 1;

static PageSpec *parsespecs(char *str)
{
   PageSpec *head, *tail;
   int other = 0;
   int num = -1;

   head = tail = newspec();
   while (*str) {
      if (isdigit(*str)) {
	 num = parseint(&str, shortusage);
      } else {
	 switch (*str++) {
	 case ':':
	    if (other || head != tail || num < 1) shortusage();
	    modulo = num;
	    num = -1;
	    break;
	 case '-':
	    tail->reversed = !tail->reversed;
	    break;
	 case '@':
	    if (num < 0) shortusage();
	    tail->scale *= parsedouble(&str, shortusage);
	    tail->flags |= SCALE;
	    break;
	 case 'l': case 'L':
	    tail->rotate += 90;
	    tail->flags |= ROTATE;
	    break;
	 case 'r': case 'R':
	    tail->rotate -= 90;
	    tail->flags |= ROTATE;
	    break;
	 case 'u': case 'U':
	    tail->rotate += 180;
	    tail->flags |= ROTATE;
	    break;
	 case '(':
	    tail->xoff += parsedimen(&str, shortusage);
	    if (*str++ != ',') shortusage();
	    tail->yoff += parsedimen(&str, shortusage);
	    if (*str++ != ')') shortusage();
	    tail->flags |= OFFSET;
	    break;
	 case '{':
	    tail->x0 = parsedimen(&str, shortusage);
	    if (*str++ != ',') shortusage();
	    tail->y0 = parsedimen(&str, shortusage);
	    if (*str++ != ',') shortusage();
	    tail->x1 = parsedimen(&str, shortusage);
	    if (*str++ != ',') shortusage();
	    tail->y1 = parsedimen(&str, shortusage);
	    if (*str++ != '}') shortusage();
	    tail->flags |= CLIP;
	    break;
	 case '+':
	    tail->flags |= ADD_NEXT;
	 case ',':
	    if (num < 0 || num >= modulo) shortusage();
	    if ((tail->flags & ADD_NEXT) == 0)
	       pagesperspec++;
	    tail->pageno = num;
	    tail->next = newspec();
	    tail = tail->next;
	    num = -1;
	    break;
	 default:
	    shortusage();
	 }
	 other = 1;
      }
   }
   if (num >= modulo)
      shortusage();
   else if (num >= 0)
      tail->pageno = num;
   return (head);
}

int main(int argc, char *argv[])
{
   PageSpec *specs = NULL;
   int nobinding = 0;
   double draw = 0;
   Paper *paper;

#ifdef PAPER
   if ( (paper = findpaper(PAPER)) != (Paper *)0 ) {
      width = (double)PaperWidth(paper);
      height = (double)PaperHeight(paper);
   }
#endif

   infile = stdin;
   outfile = stdout;
   verbose = 1;
   for (program = basename(*argv++); --argc; argv++) {
      if (strcmp(argv[0], "--help") == 0) {
	 usage(stdout);
	 exit(1);
      } else if (strcmp(argv[0], "--version") == 0) {
	 version(stdout);
	 exit(1);
      } else if (strcmp(argv[0], "--license") == 0) {
	 license(stdout);
	 exit(1);
      } else if (argv[0][0] == '-') {
	 switch (argv[0][1]) {
	 case 'q':	/* quiet */
	    verbose = 0;
	    break;
	 case 'd':	/* draw borders */
	    if (argv[0][2])
	       draw = singledimen(*argv+2, shortusage, shortusage);
	    else
	       draw = 1;
	    break;
	 case 'b':	/* no bind operator */
	    nobinding = 1;
	    break;
	 case 'w':	/* page width */
	    width = singledimen(*argv+2, shortusage, shortusage);
	    break;
	 case 'h':	/* page height */
	    height = singledimen(*argv+2, shortusage, shortusage);
	    break;
	 case 'p':	/* paper type */
	    if ( (paper = findpaper(*argv+2)) != (Paper *)0 ) {
	       width = (double)PaperWidth(paper);
	       height = (double)PaperHeight(paper);
	    } else
	      message(FATAL, "paper size '%s' not recognised\n", *argv+2);
	    break;
	 case 'v':	/* version */
	    version(stdout);
	    exit(1);
	 default:
	    if (specs == NULL)
	       specs = parsespecs(*argv);
	    else
	       shortusage();
	 }
      } else if (specs == NULL)
	 specs = parsespecs(*argv);
      else if (infile == stdin) {
	 if ((infile = fopen(*argv, "r")) == NULL)
	    message(FATAL, "can't open input file %s\n", *argv);
      } else if (outfile == stdout) {
	 if ((outfile = fopen(*argv, "w")) == NULL)
	    message(FATAL, "can't open output file %s\n", *argv);
      } else shortusage();
   }
   if (specs == NULL)
      shortusage();
#if defined(MSDOS) || defined(WINNT)
   if ( infile == stdin ) {
      int fd = fileno(stdin) ;
      if ( setmode(fd, O_BINARY) < 0 )
         message(FATAL, "can't open input file %s\n", argv[4]);
    }
   if ( outfile == stdout ) {
      int fd = fileno(stdout) ;
      if ( setmode(fd, O_BINARY) < 0 )
         message(FATAL, "can't reset stdout to binary mode\n");
    }
#endif
   if ((infile=seekable(infile))==NULL)
      message(FATAL, "can't seek input\n");

   pstops(modulo, pagesperspec, nobinding, specs, draw);

   exit(0);
}
