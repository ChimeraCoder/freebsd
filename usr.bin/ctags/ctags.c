
/*
 * You may redistribute this program and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef lint
static const char copyright[] =
"@(#) Copyright (c) 1987, 1993, 1994, 1995\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif

#if 0
#ifndef lint
static char sccsid[] = "@(#)ctags.c	8.4 (Berkeley) 2/7/95";
#endif
#endif

#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/wait.h>
__FBSDID("$FreeBSD$");

#include <err.h>
#include <limits.h>
#include <locale.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ctags.h"

/*
 * ctags: create a tags file
 */

NODE	*head;			/* head of the sorted binary tree */

				/* boolean "func" (see init()) */
bool	_wht[256], _etk[256], _itk[256], _btk[256], _gd[256];

FILE	*inf;			/* ioptr for current input file */
FILE	*outf;			/* ioptr for tags file */

long	lineftell;		/* ftell after getc( inf ) == '\n' */

int	lineno;			/* line number of current line */
int	dflag;			/* -d: non-macro defines */
int	tflag;			/* -t: create tags for typedefs */
int	vflag;			/* -v: vgrind style index output */
int	wflag;			/* -w: suppress warnings */
int	xflag;			/* -x: cxref style output */

char	*curfile;		/* current input file name */
char	searchar = '/';		/* use /.../ searches by default */
char	lbuf[LINE_MAX];

void	init(void);
void	find_entries(char *);
static void usage(void);

int
main(int argc, char **argv)
{
	static const char	*outfile = "tags";	/* output file */
	int	aflag;				/* -a: append to tags */
	int	uflag;				/* -u: update tags */
	int	exit_val;			/* exit value */
	int	step;				/* step through args */
	int	ch;				/* getopts char */

	setlocale(LC_ALL, "");

	aflag = uflag = NO;
	tflag = YES;
	while ((ch = getopt(argc, argv, "BFTadf:tuwvx")) != -1)
		switch(ch) {
		case 'B':
			searchar = '?';
			break;
		case 'F':
			searchar = '/';
			break;
		case 'T':
			tflag = NO;
			break;
		case 'a':
			aflag++;
			break;
		case 'd':
			dflag++;
			break;
		case 'f':
			outfile = optarg;
			break;
		case 't':
			tflag = YES;
			break;
		case 'u':
			uflag++;
			break;
		case 'w':
			wflag++;
			break;
		case 'v':
			vflag++;
		case 'x':
			xflag++;
			break;
		case '?':
		default:
			usage();
		}
	argv += optind;
	argc -= optind;
	if (!argc)
		usage();

	if (!xflag)
		setlocale(LC_COLLATE, "C");

	init();

	for (exit_val = step = 0; step < argc; ++step)
		if (!(inf = fopen(argv[step], "r"))) {
			warn("%s", argv[step]);
			exit_val = 1;
		}
		else {
			curfile = argv[step];
			find_entries(argv[step]);
			(void)fclose(inf);
		}

	if (head) {
		if (xflag)
			put_entries(head);
		else {
			if (uflag) {
				FILE *oldf;
				regex_t *regx;

				if ((oldf = fopen(outfile, "r")) == NULL)
					err(1, "opening %s", outfile);
				if (unlink(outfile))
					err(1, "unlinking %s", outfile);
				if ((outf = fopen(outfile, "w")) == NULL)
					err(1, "recreating %s", outfile);
				if ((regx = calloc(argc, sizeof(regex_t))) == NULL)
					err(1, "RE alloc");
				for (step = 0; step < argc; step++) {
					(void)strcpy(lbuf, "\t");
					(void)strlcat(lbuf, argv[step], LINE_MAX);
					(void)strlcat(lbuf, "\t", LINE_MAX);
					if (regcomp(regx + step, lbuf,
					    REG_NOSPEC))
						warn("RE compilation failed");
				}
nextline:
				while (fgets(lbuf, LINE_MAX, oldf)) {
					for (step = 0; step < argc; step++)
						if (regexec(regx + step,
						    lbuf, 0, NULL, 0) == 0)
							goto nextline;
					fputs(lbuf, outf);
				}
				for (step = 0; step < argc; step++)
					regfree(regx + step);
				free(regx);
				fclose(oldf);
				fclose(outf);
				++aflag;
			}
			if (!(outf = fopen(outfile, aflag ? "a" : "w")))
				err(1, "%s", outfile);
			put_entries(head);
			(void)fclose(outf);
			if (uflag) {
				pid_t pid;

				if ((pid = fork()) == -1)
					err(1, "fork failed");
				else if (pid == 0) {
					execlp("sort", "sort", "-o", outfile,
					    outfile, NULL);
					err(1, "exec of sort failed");
				}
				/* Just assume the sort went OK. The old code
				   did not do any checks either. */
				(void)wait(NULL);
			}
		}
	}
	exit(exit_val);
}

static void
usage(void)
{
	(void)fprintf(stderr, "usage: ctags [-BFTaduwvx] [-f tagsfile] file ...\n");
	exit(1);
}

/*
 * init --
 *	this routine sets up the boolean pseudo-functions which work by
 *	setting boolean flags dependent upon the corresponding character.
 *	Every char which is NOT in that string is false with respect to
 *	the pseudo-function.  Therefore, all of the array "_wht" is NO
 *	by default and then the elements subscripted by the chars in
 *	CWHITE are set to YES.  Thus, "_wht" of a char is YES if it is in
 *	the string CWHITE, else NO.
 */
void
init(void)
{
	int		i;
	const unsigned char	*sp;

	for (i = 0; i < 256; i++) {
		_wht[i] = _etk[i] = _itk[i] = _btk[i] = NO;
		_gd[i] = YES;
	}
#define	CWHITE	" \f\t\n"
	for (sp = CWHITE; *sp; sp++)	/* white space chars */
		_wht[*sp] = YES;
#define	CTOKEN	" \t\n\"'#()[]{}=-+%*/&|^~!<>;,.:?"
	for (sp = CTOKEN; *sp; sp++)	/* token ending chars */
		_etk[*sp] = YES;
#define	CINTOK	"ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz0123456789"
	for (sp = CINTOK; *sp; sp++)	/* valid in-token chars */
		_itk[*sp] = YES;
#define	CBEGIN	"ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz"
	for (sp = CBEGIN; *sp; sp++)	/* token starting chars */
		_btk[*sp] = YES;
#define	CNOTGD	",;"
	for (sp = CNOTGD; *sp; sp++)	/* invalid after-function chars */
		_gd[*sp] = NO;
}

/*
 * find_entries --
 *	this routine opens the specified file and calls the function
 *	which searches the file.
 */
void
find_entries(char *file)
{
	char	*cp;

	lineno = 0;				/* should be 1 ?? KB */
	if ((cp = strrchr(file, '.'))) {
		if (cp[1] == 'l' && !cp[2]) {
			int	c;

			for (;;) {
				if (GETC(==, EOF))
					return;
				if (!iswhite(c)) {
					rewind(inf);
					break;
				}
			}
#define	LISPCHR	";(["
/* lisp */		if (strchr(LISPCHR, c)) {
				l_entries();
				return;
			}
/* lex */		else {
				/*
				 * we search all 3 parts of a lex file
				 * for C references.  This may be wrong.
				 */
				toss_yysec();
				(void)strcpy(lbuf, "%%$");
				pfnote("yylex", lineno);
				rewind(inf);
			}
		}
/* yacc */	else if (cp[1] == 'y' && !cp[2]) {
			/*
			 * we search only the 3rd part of a yacc file
			 * for C references.  This may be wrong.
			 */
			toss_yysec();
			(void)strcpy(lbuf, "%%$");
			pfnote("yyparse", lineno);
			y_entries();
		}
/* fortran */	else if ((cp[1] != 'c' && cp[1] != 'h') && !cp[2]) {
			if (PF_funcs())
				return;
			rewind(inf);
		}
	}
/* C */	c_entries();
}