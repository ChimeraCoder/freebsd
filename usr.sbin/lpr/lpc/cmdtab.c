
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

#if 0
#ifndef lint
static char sccsid[] = "@(#)cmdtab.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */
#endif

#include "lp.cdefs.h"		/* A cross-platform version of <sys/cdefs.h> */
__FBSDID("$FreeBSD$");

#include "lpc.h"
#include "extern.h"

/*
 * lpc -- command tables
 */
char	aborthelp[] =	"terminate a spooling daemon immediately and disable printing";
char	botmqhelp[] =	"move job(s) to the bottom of printer queue";
char	cleanhelp[] =	"remove cruft files from a queue";
char	enablehelp[] =	"turn a spooling queue on";
char	disablehelp[] =	"turn a spooling queue off";
char	downhelp[] =	"do a 'stop' followed by 'disable' and put a message in status";
char	helphelp[] =	"get help on commands";
char	quithelp[] =	"exit lpc";
char	restarthelp[] =	"kill (if possible) and restart a spooling daemon";
char	setstatushelp[] = "set the status message of a queue, requires\n"
			"\t\t\"-msg\" before the text of the new message";
char	starthelp[] =	"enable printing and start a spooling daemon";
char	statushelp[] =	"show status of daemon and queue";
char	stophelp[] =	"stop a spooling daemon after current job completes and disable printing";
char	tcleanhelp[] =	"test to see what files a clean cmd would remove";
char	topqhelp[] =	"move job(s) to the top of printer queue";
char	uphelp[] =	"enable everything and restart spooling daemon";

/* Use some abbreviations so entries won't need to wrap */
#define PR	LPC_PRIVCMD
#define M	LPC_MSGOPT

struct cmd cmdtab[] = {
	{ "abort",	aborthelp,	PR,	0,		abort_q },
	{ "bottomq",	botmqhelp,	PR,	bottomq_cmd,	0 },
	{ "clean",	cleanhelp,	PR,	clean_gi,	clean_q },
	{ "enable",	enablehelp,	PR,	0,		enable_q },
	{ "exit",	quithelp,	0,	quit,		0 },
	{ "disable",	disablehelp,	PR,	0, 		disable_q },
	{ "down",	downhelp,	PR|M,	down_gi,	down_q },
	{ "help",	helphelp,	0,	help,		0 },
	{ "quit",	quithelp,	0,	quit,		0 },
	{ "restart",	restarthelp,	0,	0,		restart_q },
	{ "start",	starthelp,	PR,	0,		start_q },
	{ "status",	statushelp,	0,	0,		status },
	{ "setstatus",	setstatushelp,	PR|M,	setstatus_gi,	setstatus_q },
	{ "stop",	stophelp,	PR,	0,		stop_q },
	{ "tclean",	tcleanhelp,	0,	tclean_gi,	clean_q },
	{ "topq",	topqhelp,	PR,	topq_cmd,	0 },
	{ "up",		uphelp,		PR,	0,		up_q },
	{ "?",		helphelp,	0,	help,		0 },
	{ "xtopq",	topqhelp,	PR,	topq,		0 },
	{ 0, 0, 0, 0, 0},
};

int	NCMDS = sizeof (cmdtab) / sizeof (cmdtab[0]);