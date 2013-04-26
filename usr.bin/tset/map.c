
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

#include <sys/cdefs.h>

__FBSDID("$FreeBSD$");

#ifndef lint
static const char sccsid[] = "@(#)map.c	8.1 (Berkeley) 6/9/93";
#endif

#include <sys/types.h>

#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

#include "extern.h"

static speed_t tset_baudrate(char *);

/* Baud rate conditionals for mapping. */
#define	GT		0x01
#define	EQ		0x02
#define	LT		0x04
#define	NOT		0x08
#define	GE		(GT | EQ)
#define	LE		(LT | EQ)

typedef struct map {
	struct map *next;	/* Linked list of maps. */
	char *porttype;		/* Port type, or "" for any. */
	char *type;		/* Terminal type to select. */
	int conditional;	/* Baud rate conditionals bitmask. */
	speed_t	speed;		/* Baud	rate to	compare	against. */
} MAP;

static MAP *cur, *maplist;

/*
 * Syntax for -m:
 * [port-type][test baudrate]:terminal-type
 * The baud rate tests are: >, <, @, =, !
 */
void
add_mapping(const char *port, char *arg)
{
	MAP *mapp;
	char *copy, *p, *termp;

	copy = strdup(arg);
	mapp = malloc(sizeof(MAP));
	if (copy == NULL || mapp == NULL)
		errx(1, "malloc");
	mapp->next = NULL;
	if (maplist == NULL)
		cur = maplist = mapp;
	else {
		cur->next = mapp;
		cur =  mapp;
	}

	mapp->porttype = arg;
	mapp->conditional = 0;

	arg = strpbrk(arg, "><@=!:");

	if (arg == NULL) {			/* [?]term */
		mapp->type = mapp->porttype;
		mapp->porttype = NULL;
		goto done;
	}

	if (arg == mapp->porttype)		/* [><@=! baud]:term */
		termp = mapp->porttype = NULL;
	else
		termp = arg;

	for (;; ++arg)				/* Optional conditionals. */
		switch(*arg) {
		case '<':
			if (mapp->conditional & GT)
				goto badmopt;
			mapp->conditional |= LT;
			break;
		case '>':
			if (mapp->conditional & LT)
				goto badmopt;
			mapp->conditional |= GT;
			break;
		case '@':
		case '=':			/* Not documented. */
			mapp->conditional |= EQ;
			break;
		case '!':
			mapp->conditional |= NOT;
			break;
		default:
			goto next;
		}

next:	if (*arg == ':') {
		if (mapp->conditional)
			goto badmopt;
		++arg;
	} else {				/* Optional baudrate. */
		arg = strchr(p = arg, ':');
		if (arg == NULL)
			goto badmopt;
		*arg++ = '\0';
		mapp->speed = tset_baudrate(p);
	}

	if (*arg == '\0')			/* Non-optional type. */
		goto badmopt;

	mapp->type = arg;

	/* Terminate porttype, if specified. */
	if (termp != NULL)
		*termp = '\0';

	/* If a NOT conditional, reverse the test. */
	if (mapp->conditional & NOT)
		mapp->conditional = ~mapp->conditional & (EQ | GT | LT);

	/* If user specified a port with an option flag, set it. */
done:	if (port) {
		if (mapp->porttype)
badmopt:		errx(1, "illegal -m option format: %s", copy);
		mapp->porttype = strdup(port);
	}

#ifdef MAPDEBUG
	(void)printf("port: %s\n", mapp->porttype ? mapp->porttype : "ANY");
	(void)printf("type: %s\n", mapp->type);
	(void)printf("conditional: ");
	p = "";
	if (mapp->conditional & GT) {
		(void)printf("GT");
		p = "/";
	}
	if (mapp->conditional & EQ) {
		(void)printf("%sEQ", p);
		p = "/";
	}
	if (mapp->conditional & LT)
		(void)printf("%sLT", p);
	(void)printf("\nspeed: %d\n", mapp->speed);
#endif
}

/*
 * Return the type of terminal to use for a port of type 'type', as specified
 * by the first applicable mapping in 'map'.  If no mappings apply, return
 * 'type'.
 */
const char *
mapped(const char *type)
{
	MAP *mapp;
	int match;

	match = 0;
	for (mapp = maplist; mapp; mapp = mapp->next)
		if (mapp->porttype == NULL || !strcmp(mapp->porttype, type)) {
			switch (mapp->conditional) {
			case 0:			/* No test specified. */
				match = 1;
				break;
			case EQ:
				match =	(Ospeed	== mapp->speed);
				break;
			case GE:
				match =	(Ospeed	>= mapp->speed);
				break;
			case GT:
				match =	(Ospeed	> mapp->speed);
				break;
			case LE:
				match =	(Ospeed	<= mapp->speed);
				break;
			case LT:
				match =	(Ospeed	< mapp->speed);
				break;
			}
			if (match)
				return (mapp->type);
		}
	/* No match found; return given type. */
	return (type);
}

typedef struct speeds {
	const char	*string;
	speed_t	speed;
} SPEEDS;

static SPEEDS speeds[] = {
	{ "0",		B0 },
	{ "134.5",	B134 },
	{ "exta",	B19200 },
	{ "extb",	B38400 },
	{ NULL, 0 }
};

static speed_t
tset_baudrate(char *rate)
{
	SPEEDS *sp;
	speed_t speed;

	/* The baudrate number can be preceded by a 'B', which is ignored. */
	if (*rate == 'B')
		++rate;

	for (sp = speeds; sp->string; ++sp)
		if (!strcasecmp(rate, sp->string))
			return (sp->speed);
	speed = atol(rate);
	if (speed == 0)
		errx(1, "unknown baud rate %s", rate);
	return speed;
}