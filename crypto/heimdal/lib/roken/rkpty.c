
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

#include "config.h"

#ifndef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PTY_H
#include <pty.h>
#endif
#ifdef HAVE_UTIL_H
#include <util.h>
#endif
#ifdef HAVE_LIBUTIL_H
#include <libutil.h>
#endif

#ifdef	STREAMSPTY
#include <stropts.h>
#endif /* STREAMPTY */

#include "roken.h"
#include <getarg.h>

struct command {
    enum { CMD_EXPECT = 0, CMD_SEND, CMD_PASSWORD } type;
    unsigned int lineno;
    char *str;
    struct command *next;
};

/*
 *
 */

static struct command *commands, **next = &commands;

static sig_atomic_t alarmset = 0;

static int timeout = 10;
static int verbose;
static int help_flag;
static int version_flag;

static int master;
static int slave;
static char line[256] = { 0 };

static void
caught_signal(int signo)
{
    alarmset = signo;
}


static void
open_pty(void)
{
#ifdef _AIX
    printf("implement open_pty\n");
    exit(77);
#endif
#if defined(HAVE_OPENPTY) || defined(__linux) || defined(__osf__) /* XXX */
    if(openpty(&master, &slave, line, 0, 0) == 0)
	return;
#endif /* HAVE_OPENPTY .... */
#ifdef STREAMSPTY
    {
	char *clone[] = {
	    "/dev/ptc",
	    "/dev/ptmx",
	    "/dev/ptm",
	    "/dev/ptym/clone",
	    NULL
	};
	char **q;

	for(q = clone; *q; q++){
	    master = open(*q, O_RDWR);
	    if(master >= 0){
#ifdef HAVE_GRANTPT
		grantpt(master);
#endif
#ifdef HAVE_UNLOCKPT
		unlockpt(master);
#endif
		strlcpy(line, ptsname(master), sizeof(line));
		slave = open(line, O_RDWR);
		if (slave < 0)
		    errx(1, "failed to open slave when using %s", *q);
		ioctl(slave, I_PUSH, "ptem");
		ioctl(slave, I_PUSH, "ldterm");

		return;
	    }
	}
    }
#endif /* STREAMSPTY */

    /* more cases, like open /dev/ptmx, etc */

    exit(77);
}

/*
 *
 */

static char *
iscmd(const char *buf, const char *s)
{
    size_t len = strlen(s);
    if (strncmp(buf, s, len) != 0)
	return NULL;
    return estrdup(buf + len);
}

static void
parse_configuration(const char *fn)
{
    struct command *c;
    char s[1024];
    char *str;
    unsigned int lineno = 0;
    FILE *cmd;

    cmd = fopen(fn, "r");
    if (cmd == NULL)
	err(1, "open: %s", fn);

    while (fgets(s, sizeof(s),  cmd) != NULL) {

	s[strcspn(s, "#\n")] = '\0';
	lineno++;

	c = calloc(1, sizeof(*c));
	if (c == NULL)
	    errx(1, "malloc");

	c->lineno = lineno;
	(*next) = c;
	next = &(c->next);

	if ((str = iscmd(s, "expect ")) != NULL) {
	    c->type = CMD_EXPECT;
	    c->str = str;
	} else if ((str = iscmd(s, "send ")) != NULL) {
	    c->type = CMD_SEND;
	    c->str = str;
	} else if ((str = iscmd(s, "password ")) != NULL) {
	    c->type = CMD_PASSWORD;
	    c->str = str;
	} else
	    errx(1, "Invalid command on line %d: %s", lineno, s);
    }

    fclose(cmd);
}


/*
 *
 */

static int
eval_parent(pid_t pid)
{
    struct command *c;
    char in;
    size_t len = 0;
    ssize_t sret;

    for (c = commands; c != NULL; c = c->next) {
	switch(c->type) {
	case CMD_EXPECT:
	    if (verbose)
		printf("[expecting %s]", c->str);
	    len = 0;
	    alarm(timeout);
	    while((sret = read(master, &in, sizeof(in))) > 0) {
		alarm(timeout);
		printf("%c", in);
		if (c->str[len] != in) {
		    len = 0;
		    continue;
		}
		len++;
		if (c->str[len] == '\0')
		    break;
	    }
	    alarm(0);
	    if (alarmset == SIGALRM)
		errx(1, "timeout waiting for %s (line %u)",
		     c->str, c->lineno);
	    else if (alarmset)
		errx(1, "got a signal %d waiting for %s (line %u)",
		     alarmset, c->str, c->lineno);
	    if (sret <= 0)
		errx(1, "end command while waiting for %s (line %u)",
		     c->str, c->lineno);
	    break;
	case CMD_SEND:
	case CMD_PASSWORD: {
	    size_t i = 0;
	    const char *msg = (c->type == CMD_PASSWORD) ? "****" : c->str;

	    if (verbose)
		printf("[send %s]", msg);

	    len = strlen(c->str);

	    while (i < len) {
		if (c->str[i] == '\\' && i < len - 1) {
		    char ctrl;
		    i++;
		    switch(c->str[i]) {
		    case 'n': ctrl = '\n'; break;
		    case 'r': ctrl = '\r'; break;
		    case 't': ctrl = '\t'; break;
		    default:
			errx(1, "unknown control char %c (line %u)",
			     c->str[i], c->lineno);
		    }
		    if (net_write(master, &ctrl, 1) != 1)
			errx(1, "command refused input (line %u)", c->lineno);
		} else {
		    if (net_write(master, &c->str[i], 1) != 1)
			errx(1, "command refused input (line %u)", c->lineno);
		}
		i++;
	    }
	    break;
	}
	default:
	    abort();
	}
    }
    while(read(master, &in, sizeof(in)) > 0)
	printf("%c", in);

    if (verbose)
	printf("[end of program]\n");

    /*
     * Fetch status from child
     */
    {
	int ret, status;

	ret = waitpid(pid, &status, 0);
	if (ret == -1)
	    err(1, "waitpid");
	if (WIFEXITED(status) && WEXITSTATUS(status))
	    return WEXITSTATUS(status);
	else if (WIFSIGNALED(status)) {
	    printf("killed by signal: %d\n", WTERMSIG(status));
	    return 1;
	}
    }
    return 0;
}

/*
 *
 */

static struct getargs args[] = {
    { "timeout", 	't', arg_integer, &timeout, "timout", "seconds" },
    { "verbose", 	'v', arg_counter, &verbose, "verbose debugging" },
    { "version",	0, arg_flag,	&version_flag, "print version" },
    { "help",		0, arg_flag,	&help_flag, NULL }
};

static void
usage(int ret)
{
    arg_printusage (args, sizeof(args)/sizeof(*args), NULL, "infile command..");
    exit (ret);
}

int
main(int argc, char **argv)
{
    int optidx = 0;
    pid_t pid;

    setprogname(argv[0]);

    if(getarg(args, sizeof(args) / sizeof(args[0]), argc, argv, &optidx))
	usage(1);

    if (help_flag)
	usage (0);

    if (version_flag) {
	fprintf (stderr, "%s from %s-%s\n", getprogname(), PACKAGE, VERSION);
	return 0;
    }

    argv += optidx;
    argc -= optidx;

    if (argc < 2)
	usage(1);

    parse_configuration(argv[0]);

    argv += 1;

    open_pty();

    pid = fork();
    switch (pid) {
    case -1:
	err(1, "Failed to fork");
    case 0:

	if(setsid()<0)
	    err(1, "setsid");

	dup2(slave, STDIN_FILENO);
	dup2(slave, STDOUT_FILENO);
	dup2(slave, STDERR_FILENO);
	closefrom(STDERR_FILENO + 1);

	execvp(argv[0], argv); /* add NULL to end of array ? */
	err(1, "Failed to exec: %s", argv[0]);
    default:
	close(slave);
	{
	    struct sigaction sa;

	    sa.sa_handler = caught_signal;
	    sa.sa_flags = 0;
	    sigemptyset (&sa.sa_mask);

	    sigaction(SIGALRM, &sa, NULL);
	}

	return eval_parent(pid);
    }
}