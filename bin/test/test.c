
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
/*	$NetBSD: test.c,v 1.21 1999/04/05 09:48:38 kleink Exp $	*/
/*-
 * test(1); version 7-like  --  author Erik Baalbergen
 * modified by Eric Gisin to be used as built-in.
 * modified by Arnold Robbins to add SVR3 compatibility
 * (-x -c -b -p -u -g -k) plus Korn's -L -nt -ot -ef and new -S (socket).
 * modified by J.T. Conklin for NetBSD.
 *
 * This program is in the Public Domain.
 */
/*
 * Important: This file is used both as a standalone program /bin/test and
 * as a builtin for /bin/sh (#define SHELL).
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/stat.h>

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef SHELL
#define main testcmd
#include "bltin/bltin.h"
#else
#include <locale.h>

static void error(const char *, ...) __dead2 __printf0like(1, 2);

static void
error(const char *msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	verrx(2, msg, ap);
	/*NOTREACHED*/
	va_end(ap);
}
#endif

/* test(1) accepts the following grammar:
	oexpr	::= aexpr | aexpr "-o" oexpr ;
	aexpr	::= nexpr | nexpr "-a" aexpr ;
	nexpr	::= primary | "!" primary
	primary	::= unary-operator operand
		| operand binary-operator operand
		| operand
		| "(" oexpr ")"
		;
	unary-operator ::= "-r"|"-w"|"-x"|"-f"|"-d"|"-c"|"-b"|"-p"|
		"-u"|"-g"|"-k"|"-s"|"-t"|"-z"|"-n"|"-o"|"-O"|"-G"|"-L"|"-S";

	binary-operator ::= "="|"!="|"-eq"|"-ne"|"-ge"|"-gt"|"-le"|"-lt"|
			"-nt"|"-nt[abcm][abcm]"|"-ot"|"-ot[abcm][abcm])"|"-ef";
	operand ::= <any legal UNIX file name>
*/

enum token {
	EOI,
	FILRD,
	FILWR,
	FILEX,
	FILEXIST,
	FILREG,
	FILDIR,
	FILCDEV,
	FILBDEV,
	FILFIFO,
	FILSOCK,
	FILSYM,
	FILGZ,
	FILTT,
	FILSUID,
	FILSGID,
	FILSTCK,
	FILNTAA,
	FILNTAB,
	FILNTAC,
	FILNTAM,
	FILNTBA,
	FILNTBB,
	FILNTBC,
	FILNTBM,
	FILNTCA,
	FILNTCB,
	FILNTCC,
	FILNTCM,
	FILNTMA,
	FILNTMB,
	FILNTMC,
	FILNTMM,
	FILOTAA,
	FILOTAB,
	FILOTAC,
	FILOTAM,
	FILOTBA,
	FILOTBB,
	FILOTBC,
	FILOTBM,
	FILOTCA,
	FILOTCB,
	FILOTCC,
	FILOTCM,
	FILOTMA,
	FILOTMB,
	FILOTMC,
	FILOTMM,
	FILEQ,
	FILUID,
	FILGID,
	STREZ,
	STRNZ,
	STREQ,
	STRNE,
	STRLT,
	STRGT,
	INTEQ,
	INTNE,
	INTGE,
	INTGT,
	INTLE,
	INTLT,
	UNOT,
	BAND,
	BOR,
	LPAREN,
	RPAREN,
	OPERAND
};

enum token_types {
	UNOP,
	BINOP,
	BUNOP,
	BBINOP,
	PAREN
};

enum time_types {
	ATIME,
	BTIME,
	CTIME,
	MTIME
};

static struct t_op {
	char op_text[6];
	char op_num, op_type;
} const ops [] = {
	{"-r",	FILRD,	UNOP},
	{"-w",	FILWR,	UNOP},
	{"-x",	FILEX,	UNOP},
	{"-e",	FILEXIST,UNOP},
	{"-f",	FILREG,	UNOP},
	{"-d",	FILDIR,	UNOP},
	{"-c",	FILCDEV,UNOP},
	{"-b",	FILBDEV,UNOP},
	{"-p",	FILFIFO,UNOP},
	{"-u",	FILSUID,UNOP},
	{"-g",	FILSGID,UNOP},
	{"-k",	FILSTCK,UNOP},
	{"-s",	FILGZ,	UNOP},
	{"-t",	FILTT,	UNOP},
	{"-z",	STREZ,	UNOP},
	{"-n",	STRNZ,	UNOP},
	{"-h",	FILSYM,	UNOP},		/* for backwards compat */
	{"-O",	FILUID,	UNOP},
	{"-G",	FILGID,	UNOP},
	{"-L",	FILSYM,	UNOP},
	{"-S",	FILSOCK,UNOP},
	{"=",	STREQ,	BINOP},
	{"==",	STREQ,	BINOP},
	{"!=",	STRNE,	BINOP},
	{"<",	STRLT,	BINOP},
	{">",	STRGT,	BINOP},
	{"-eq",	INTEQ,	BINOP},
	{"-ne",	INTNE,	BINOP},
	{"-ge",	INTGE,	BINOP},
	{"-gt",	INTGT,	BINOP},
	{"-le",	INTLE,	BINOP},
	{"-lt",	INTLT,	BINOP},
	{"-nt",	FILNTMM,	BINOP},
	{"-ntaa",	FILNTAA,	BINOP},
	{"-ntab",	FILNTAB,	BINOP},
	{"-ntac",	FILNTAC,	BINOP},
	{"-ntam",	FILNTAM,	BINOP},
	{"-ntba",	FILNTBA,	BINOP},
	{"-ntbb",	FILNTBB,	BINOP},
	{"-ntbc",	FILNTBC,	BINOP},
	{"-ntbm",	FILNTBM,	BINOP},
	{"-ntca",	FILNTCA,	BINOP},
	{"-ntcb",	FILNTCB,	BINOP},
	{"-ntcc",	FILNTCC,	BINOP},
	{"-ntcm",	FILNTCM,	BINOP},
	{"-ntma",	FILNTMA,	BINOP},
	{"-ntmb",	FILNTMB,	BINOP},
	{"-ntmc",	FILNTMC,	BINOP},
	{"-ntmm",	FILNTMM,	BINOP},
	{"-ot",	FILOTMM,	BINOP},
	{"-otaa",	FILOTAA,	BINOP},
	{"-otab",	FILOTBB,	BINOP},
	{"-otac",	FILOTAC,	BINOP},
	{"-otam",	FILOTAM,	BINOP},
	{"-otba",	FILOTBA,	BINOP},
	{"-otbb",	FILOTBB,	BINOP},
	{"-otbc",	FILOTBC,	BINOP},
	{"-otbm",	FILOTBM,	BINOP},
	{"-otca",	FILOTCA,	BINOP},
	{"-otcb",	FILOTCB,	BINOP},
	{"-otcc",	FILOTCC,	BINOP},
	{"-otcm",	FILOTCM,	BINOP},
	{"-otma",	FILOTMA,	BINOP},
	{"-otmb",	FILOTMB,	BINOP},
	{"-otmc",	FILOTMC,	BINOP},
	{"-otmm",	FILOTMM,	BINOP},
	{"-ef",	FILEQ,	BINOP},
	{"!",	UNOT,	BUNOP},
	{"-a",	BAND,	BBINOP},
	{"-o",	BOR,	BBINOP},
	{"(",	LPAREN,	PAREN},
	{")",	RPAREN,	PAREN},
	{"",	0,	0}
};

static struct t_op const *t_wp_op;
static int nargc;
static char **t_wp;
static int parenlevel;

static int	aexpr(enum token);
static int	binop(void);
static int	equalf(const char *, const char *);
static int	filstat(char *, enum token);
static int	getn(const char *);
static intmax_t	getq(const char *);
static int	intcmp(const char *, const char *);
static int	isunopoperand(void);
static int	islparenoperand(void);
static int	isrparenoperand(void);
static int	newerf(const char *, const char *, enum time_types,
		       enum time_types);
static int	nexpr(enum token);
static int	oexpr(enum token);
static int	primary(enum token);
static void	syntax(const char *, const char *);
static enum	token t_lex(char *);

int
main(int argc, char **argv)
{
	int	res;
	char	*p;

	if ((p = strrchr(argv[0], '/')) == NULL)
		p = argv[0];
	else
		p++;
	if (strcmp(p, "[") == 0) {
		if (strcmp(argv[--argc], "]") != 0)
			error("missing ]");
		argv[argc] = NULL;
	}

	/* no expression => false */
	if (--argc <= 0)
		return 1;

#ifndef SHELL
	(void)setlocale(LC_CTYPE, "");
#endif
	nargc = argc;
	t_wp = &argv[1];
	parenlevel = 0;
	if (nargc == 4 && strcmp(*t_wp, "!") == 0) {
		/* Things like ! "" -o x do not fit in the normal grammar. */
		--nargc;
		++t_wp;
		res = oexpr(t_lex(*t_wp));
	} else
		res = !oexpr(t_lex(*t_wp));

	if (--nargc > 0)
		syntax(*t_wp, "unexpected operator");

	return res;
}

static void
syntax(const char *op, const char *msg)
{

	if (op && *op)
		error("%s: %s", op, msg);
	else
		error("%s", msg);
}

static int
oexpr(enum token n)
{
	int res;

	res = aexpr(n);
	if (t_lex(nargc > 0 ? (--nargc, *++t_wp) : NULL) == BOR)
		return oexpr(t_lex(nargc > 0 ? (--nargc, *++t_wp) : NULL)) ||
		    res;
	t_wp--;
	nargc++;
	return res;
}

static int
aexpr(enum token n)
{
	int res;

	res = nexpr(n);
	if (t_lex(nargc > 0 ? (--nargc, *++t_wp) : NULL) == BAND)
		return aexpr(t_lex(nargc > 0 ? (--nargc, *++t_wp) : NULL)) &&
		    res;
	t_wp--;
	nargc++;
	return res;
}

static int
nexpr(enum token n)
{
	if (n == UNOT)
		return !nexpr(t_lex(nargc > 0 ? (--nargc, *++t_wp) : NULL));
	return primary(n);
}

static int
primary(enum token n)
{
	enum token nn;
	int res;

	if (n == EOI)
		return 0;		/* missing expression */
	if (n == LPAREN) {
		parenlevel++;
		if ((nn = t_lex(nargc > 0 ? (--nargc, *++t_wp) : NULL)) ==
		    RPAREN) {
			parenlevel--;
			return 0;	/* missing expression */
		}
		res = oexpr(nn);
		if (t_lex(nargc > 0 ? (--nargc, *++t_wp) : NULL) != RPAREN)
			syntax(NULL, "closing paren expected");
		parenlevel--;
		return res;
	}
	if (t_wp_op && t_wp_op->op_type == UNOP) {
		/* unary expression */
		if (--nargc == 0)
			syntax(t_wp_op->op_text, "argument expected");
		switch (n) {
		case STREZ:
			return strlen(*++t_wp) == 0;
		case STRNZ:
			return strlen(*++t_wp) != 0;
		case FILTT:
			return isatty(getn(*++t_wp));
		default:
			return filstat(*++t_wp, n);
		}
	}

	if (t_lex(nargc > 0 ? t_wp[1] : NULL), t_wp_op && t_wp_op->op_type ==
	    BINOP) {
		return binop();
	}

	return strlen(*t_wp) > 0;
}

static int
binop(void)
{
	const char *opnd1, *opnd2;
	struct t_op const *op;

	opnd1 = *t_wp;
	(void) t_lex(nargc > 0 ? (--nargc, *++t_wp) : NULL);
	op = t_wp_op;

	if ((opnd2 = nargc > 0 ? (--nargc, *++t_wp) : NULL) == NULL)
		syntax(op->op_text, "argument expected");

	switch (op->op_num) {
	case STREQ:
		return strcmp(opnd1, opnd2) == 0;
	case STRNE:
		return strcmp(opnd1, opnd2) != 0;
	case STRLT:
		return strcmp(opnd1, opnd2) < 0;
	case STRGT:
		return strcmp(opnd1, opnd2) > 0;
	case INTEQ:
		return intcmp(opnd1, opnd2) == 0;
	case INTNE:
		return intcmp(opnd1, opnd2) != 0;
	case INTGE:
		return intcmp(opnd1, opnd2) >= 0;
	case INTGT:
		return intcmp(opnd1, opnd2) > 0;
	case INTLE:
		return intcmp(opnd1, opnd2) <= 0;
	case INTLT:
		return intcmp(opnd1, opnd2) < 0;
	case FILNTAA:
		return newerf(opnd1, opnd2, ATIME, ATIME);
	case FILNTAB:
		return newerf(opnd1, opnd2, ATIME, BTIME);
	case FILNTAC:
		return newerf(opnd1, opnd2, ATIME, CTIME);
	case FILNTAM:
		return newerf(opnd1, opnd2, ATIME, MTIME);
	case FILNTBA:
		return newerf(opnd1, opnd2, BTIME, ATIME);
	case FILNTBB:
		return newerf(opnd1, opnd2, BTIME, BTIME);
	case FILNTBC:
		return newerf(opnd1, opnd2, BTIME, CTIME);
	case FILNTBM:
		return newerf(opnd1, opnd2, BTIME, MTIME);
	case FILNTCA:
		return newerf(opnd1, opnd2, CTIME, ATIME);
	case FILNTCB:
		return newerf(opnd1, opnd2, CTIME, BTIME);
	case FILNTCC:
		return newerf(opnd1, opnd2, CTIME, CTIME);
	case FILNTCM:
		return newerf(opnd1, opnd2, CTIME, MTIME);
	case FILNTMA:
		return newerf(opnd1, opnd2, MTIME, ATIME);
	case FILNTMB:
		return newerf(opnd1, opnd2, MTIME, BTIME);
	case FILNTMC:
		return newerf(opnd1, opnd2, MTIME, CTIME);
	case FILNTMM:
		return newerf(opnd1, opnd2, MTIME, MTIME);
	case FILOTAA:
		return newerf(opnd2, opnd1, ATIME, ATIME);
	case FILOTAB:
		return newerf(opnd2, opnd1, BTIME, ATIME);
	case FILOTAC:
		return newerf(opnd2, opnd1, CTIME, ATIME);
	case FILOTAM:
		return newerf(opnd2, opnd1, MTIME, ATIME);
	case FILOTBA:
		return newerf(opnd2, opnd1, ATIME, BTIME);
	case FILOTBB:
		return newerf(opnd2, opnd1, BTIME, BTIME);
	case FILOTBC:
		return newerf(opnd2, opnd1, CTIME, BTIME);
	case FILOTBM:
		return newerf(opnd2, opnd1, MTIME, BTIME);
	case FILOTCA:
		return newerf(opnd2, opnd1, ATIME, CTIME);
	case FILOTCB:
		return newerf(opnd2, opnd1, BTIME, CTIME);
	case FILOTCC:
		return newerf(opnd2, opnd1, CTIME, CTIME);
	case FILOTCM:
		return newerf(opnd2, opnd1, MTIME, CTIME);
	case FILOTMA:
		return newerf(opnd2, opnd1, ATIME, MTIME);
	case FILOTMB:
		return newerf(opnd2, opnd1, BTIME, MTIME);
	case FILOTMC:
		return newerf(opnd2, opnd1, CTIME, MTIME);
	case FILOTMM:
		return newerf(opnd2, opnd1, MTIME, MTIME);
	case FILEQ:
		return equalf (opnd1, opnd2);
	default:
		abort();
		/* NOTREACHED */
	}
}

static int
filstat(char *nm, enum token mode)
{
	struct stat s;

	if (mode == FILSYM ? lstat(nm, &s) : stat(nm, &s))
		return 0;

	switch (mode) {
	case FILRD:
		return (eaccess(nm, R_OK) == 0);
	case FILWR:
		return (eaccess(nm, W_OK) == 0);
	case FILEX:
		/* XXX work around eaccess(2) false positives for superuser */
		if (eaccess(nm, X_OK) != 0)
			return 0;
		if (S_ISDIR(s.st_mode) || geteuid() != 0)
			return 1;
		return (s.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) != 0;
	case FILEXIST:
		return (eaccess(nm, F_OK) == 0);
	case FILREG:
		return S_ISREG(s.st_mode);
	case FILDIR:
		return S_ISDIR(s.st_mode);
	case FILCDEV:
		return S_ISCHR(s.st_mode);
	case FILBDEV:
		return S_ISBLK(s.st_mode);
	case FILFIFO:
		return S_ISFIFO(s.st_mode);
	case FILSOCK:
		return S_ISSOCK(s.st_mode);
	case FILSYM:
		return S_ISLNK(s.st_mode);
	case FILSUID:
		return (s.st_mode & S_ISUID) != 0;
	case FILSGID:
		return (s.st_mode & S_ISGID) != 0;
	case FILSTCK:
		return (s.st_mode & S_ISVTX) != 0;
	case FILGZ:
		return s.st_size > (off_t)0;
	case FILUID:
		return s.st_uid == geteuid();
	case FILGID:
		return s.st_gid == getegid();
	default:
		return 1;
	}
}

static enum token
t_lex(char *s)
{
	struct t_op const *op = ops;

	if (s == 0) {
		t_wp_op = NULL;
		return EOI;
	}
	while (*op->op_text) {
		if (strcmp(s, op->op_text) == 0) {
			if (((op->op_type == UNOP || op->op_type == BUNOP)
						&& isunopoperand()) ||
			    (op->op_num == LPAREN && islparenoperand()) ||
			    (op->op_num == RPAREN && isrparenoperand()))
				break;
			t_wp_op = op;
			return op->op_num;
		}
		op++;
	}
	t_wp_op = NULL;
	return OPERAND;
}

static int
isunopoperand(void)
{
	struct t_op const *op = ops;
	char *s;
	char *t;

	if (nargc == 1)
		return 1;
	s = *(t_wp + 1);
	if (nargc == 2)
		return parenlevel == 1 && strcmp(s, ")") == 0;
	t = *(t_wp + 2);
	while (*op->op_text) {
		if (strcmp(s, op->op_text) == 0)
			return op->op_type == BINOP &&
			    (parenlevel == 0 || t[0] != ')' || t[1] != '\0');
		op++;
	}
	return 0;
}

static int
islparenoperand(void)
{
	struct t_op const *op = ops;
	char *s;

	if (nargc == 1)
		return 1;
	s = *(t_wp + 1);
	if (nargc == 2)
		return parenlevel == 1 && strcmp(s, ")") == 0;
	if (nargc != 3)
		return 0;
	while (*op->op_text) {
		if (strcmp(s, op->op_text) == 0)
			return op->op_type == BINOP;
		op++;
	}
	return 0;
}

static int
isrparenoperand(void)
{
	char *s;

	if (nargc == 1)
		return 0;
	s = *(t_wp + 1);
	if (nargc == 2)
		return parenlevel == 1 && strcmp(s, ")") == 0;
	return 0;
}

/* atoi with error detection */
static int
getn(const char *s)
{
	char *p;
	long r;

	errno = 0;
	r = strtol(s, &p, 10);

	if (s == p)
		error("%s: bad number", s);

	if (errno != 0)
		error((errno == EINVAL) ? "%s: bad number" :
					  "%s: out of range", s);

	while (isspace((unsigned char)*p))
		p++;

	if (*p)
		error("%s: bad number", s);

	return (int) r;
}

/* atoi with error detection and 64 bit range */
static intmax_t
getq(const char *s)
{
	char *p;
	intmax_t r;

	errno = 0;
	r = strtoimax(s, &p, 10);

	if (s == p)
		error("%s: bad number", s);

	if (errno != 0)
		error((errno == EINVAL) ? "%s: bad number" :
					  "%s: out of range", s);

	while (isspace((unsigned char)*p))
		p++;

	if (*p)
		error("%s: bad number", s);

	return r;
}

static int
intcmp (const char *s1, const char *s2)
{
	intmax_t q1, q2;


	q1 = getq(s1);
	q2 = getq(s2);

	if (q1 > q2)
		return 1;

	if (q1 < q2)
		return -1;

	return 0;
}

static int
newerf (const char *f1, const char *f2, enum time_types t1, enum time_types t2)
{
	struct stat b1, b2;
	struct timespec *ts1, *ts2;

	if (stat(f1, &b1) != 0 || stat(f2, &b2) != 0)
		return 0;

	switch (t1) {
	case ATIME:	ts1 = &b1.st_atim;	break;
	case BTIME:	ts1 = &b1.st_birthtim;	break;
	case CTIME:	ts1 = &b1.st_ctim;	break;
	default:	ts1 = &b1.st_mtim;	break;
	}

	switch (t2) {
	case ATIME:	ts2 = &b2.st_atim;	break;
	case BTIME:	ts2 = &b2.st_birthtim;	break;
	case CTIME:	ts2 = &b2.st_ctim;	break;
	default:	ts2 = &b2.st_mtim;	break;
	}

	if (ts1->tv_sec > ts2->tv_sec)
		return 1;
	if (ts1->tv_sec < ts2->tv_sec)
		return 0;

       return (ts1->tv_nsec > ts2->tv_nsec);
}

static int
equalf (const char *f1, const char *f2)
{
	struct stat b1, b2;

	return (stat (f1, &b1) == 0 &&
		stat (f2, &b2) == 0 &&
		b1.st_dev == b2.st_dev &&
		b1.st_ino == b2.st_ino);
}