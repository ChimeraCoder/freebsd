
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
#if 0
static char sccsid[] = "@(#)cmdtab.c	8.1 (Berkeley) 6/6/93";
#endif
static const char rcsid[] =
  "$FreeBSD$";
#endif /* not lint */

#include "timedc.h"

char	clockdiffhelp[] =	"measures clock differences between machines";
char	helphelp[] =		"gets help on commands";
char	msitehelp[] =		"finds location of master";
char	quithelp[] =		"exits timedc";
char	testinghelp[] =		"causes election timers to expire";
char	tracinghelp[] =		"turns tracing on or off";

struct cmd cmdtab[] = {
	{ "clockdiff",	clockdiffhelp,	clockdiff,	0 },
	{ "election",	testinghelp,	testing,	1 },
	{ "help",	helphelp,	help,		0 },
	{ "msite",	msitehelp,	msite,		0 },
	{ "quit",	quithelp,	quit,		0 },
	{ "trace",	tracinghelp,	tracing,	1 },
	{ "?",		helphelp,	help,		0 },
};

int	NCMDS = sizeof (cmdtab) / sizeof (cmdtab[0]);