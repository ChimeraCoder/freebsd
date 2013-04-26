
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)rexec.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/stat.h>

#include <netinet/in.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <ctype.h>
#include <err.h>
#include <stdlib.h>
#include <unistd.h>

int	rexecoptions;
char	*getpass(), *getlogin();

/*
 * Options and other state info.
 */
struct macel {
	char mac_name[9];	/* macro name */
	char *mac_start;	/* start of macro in macbuf */
	char *mac_end;		/* end of macro in macbuf */
};

int macnum;			/* number of defined macros */
struct macel macros[16];
char macbuf[4096];

static	FILE *cfile;

#define	DEFAULT	1
#define	LOGIN	2
#define	PASSWD	3
#define	ACCOUNT 4
#define MACDEF  5
#define	ID	10
#define	MACH	11

static char tokval[100];

static struct toktab {
	char *tokstr;
	int tval;
} toktab[]= {
	{ "default",	DEFAULT },
	{ "login",	LOGIN },
	{ "password",	PASSWD },
	{ "passwd",	PASSWD },
	{ "account",	ACCOUNT },
	{ "machine",	MACH },
	{ "macdef",	MACDEF },
	{ NULL,		0 }
};

static int
token()
{
	char *cp;
	int c;
	struct toktab *t;

	if (feof(cfile) || ferror(cfile))
		return (0);
	while ((c = getc(cfile)) != EOF &&
	    (c == '\n' || c == '\t' || c == ' ' || c == ','))
		continue;
	if (c == EOF)
		return (0);
	cp = tokval;
	if (c == '"') {
		while ((c = getc(cfile)) != EOF && c != '"') {
			if (c == '\\')
				c = getc(cfile);
			*cp++ = c;
		}
	} else {
		*cp++ = c;
		while ((c = getc(cfile)) != EOF
		    && c != '\n' && c != '\t' && c != ' ' && c != ',') {
			if (c == '\\')
				c = getc(cfile);
			*cp++ = c;
		}
	}
	*cp = 0;
	if (tokval[0] == 0)
		return (0);
	for (t = toktab; t->tokstr; t++)
		if (!strcmp(t->tokstr, tokval))
			return (t->tval);
	return (ID);
}

static int
ruserpass(host, aname, apass, aacct)
	char *host, **aname, **apass, **aacct;
{
	char *hdir, buf[BUFSIZ], *tmp;
	char myname[MAXHOSTNAMELEN], *mydomain;
	int t, i, c, usedefault = 0;
	struct stat stb;

	hdir = getenv("HOME");
	if (hdir == NULL)
		hdir = ".";
	if (strlen(hdir) + 8 > sizeof(buf))
		return (0);
	(void) sprintf(buf, "%s/.netrc", hdir);
	cfile = fopen(buf, "r");
	if (cfile == NULL) {
		if (errno != ENOENT)
			warn("%s", buf);
		return (0);
	}
	if (gethostname(myname, sizeof(myname)) < 0)
		myname[0] = '\0';
	if ((mydomain = strchr(myname, '.')) == NULL)
		mydomain = "";
next:
	while ((t = token())) switch(t) {

	case DEFAULT:
		usedefault = 1;
		/* FALL THROUGH */

	case MACH:
		if (!usedefault) {
			if (token() != ID)
				continue;
			/*
			 * Allow match either for user's input host name
			 * or official hostname.  Also allow match of
			 * incompletely-specified host in local domain.
			 */
			if (strcasecmp(host, tokval) == 0)
				goto match;
			if ((tmp = strchr(host, '.')) != NULL &&
			    strcasecmp(tmp, mydomain) == 0 &&
			    strncasecmp(host, tokval, tmp - host) == 0 &&
			    tokval[tmp - host] == '\0')
				goto match;
			continue;
		}
	match:
		while ((t = token()) && t != MACH && t != DEFAULT) switch(t) {

		case LOGIN:
			if (token())
				if (*aname == 0) {
					*aname = malloc((unsigned) strlen(tokval) + 1);
					(void) strcpy(*aname, tokval);
				} else {
					if (strcmp(*aname, tokval))
						goto next;
				}
			break;
		case PASSWD:
			if ((*aname == 0 || strcmp(*aname, "anonymous")) &&
			    fstat(fileno(cfile), &stb) >= 0 &&
			    (stb.st_mode & 077) != 0) {
	warnx("Error: .netrc file is readable by others.");
	warnx("Remove password or make file unreadable by others.");
				goto bad;
			}
			if (token() && *apass == 0) {
				*apass = malloc((unsigned) strlen(tokval) + 1);
				(void) strcpy(*apass, tokval);
			}
			break;
		case ACCOUNT:
			if (fstat(fileno(cfile), &stb) >= 0
			    && (stb.st_mode & 077) != 0) {
	warnx("Error: .netrc file is readable by others.");
	warnx("Remove account or make file unreadable by others.");
				goto bad;
			}
			if (token() && *aacct == 0) {
				*aacct = malloc((unsigned) strlen(tokval) + 1);
				(void) strcpy(*aacct, tokval);
			}
			break;
		case MACDEF:
			while ((c=getc(cfile)) != EOF &&
						(c == ' ' || c == '\t'))
				;
			if (c == EOF || c == '\n') {
				printf("Missing macdef name argument.\n");
				goto bad;
			}
			if (macnum == 16) {
				printf("Limit of 16 macros have already been defined\n");
				goto bad;
			}
			tmp = macros[macnum].mac_name;
			*tmp++ = c;
			for (i=0; i < 8 && (c=getc(cfile)) != EOF &&
			    !isspace(c); ++i) {
				*tmp++ = c;
			}
			if (c == EOF) {
				printf("Macro definition missing null line terminator.\n");
				goto bad;
			}
			*tmp = '\0';
			if (c != '\n') {
				while ((c=getc(cfile)) != EOF && c != '\n');
			}
			if (c == EOF) {
				printf("Macro definition missing null line terminator.\n");
				goto bad;
			}
			if (macnum == 0) {
				macros[macnum].mac_start = macbuf;
			}
			else {
				macros[macnum].mac_start = macros[macnum-1].mac_end + 1;
			}
			tmp = macros[macnum].mac_start;
			while (tmp != macbuf + 4096) {
				if ((c=getc(cfile)) == EOF) {
				printf("Macro definition missing null line terminator.\n");
					goto bad;
				}
				*tmp = c;
				if (*tmp == '\n') {
					if (*(tmp-1) == '\0') {
					   macros[macnum++].mac_end = tmp - 1;
					   break;
					}
					*tmp = '\0';
				}
				tmp++;
			}
			if (tmp == macbuf + 4096) {
				printf("4K macro buffer exceeded\n");
				goto bad;
			}
			break;
		default:
			warnx("Unknown .netrc keyword %s", tokval);
			break;
		}
		goto done;
	}
done:
	(void) fclose(cfile);
	return (0);
bad:
	(void) fclose(cfile);
	return (-1);
}

int
rexec(ahost, rport, name, pass, cmd, fd2p)
	char **ahost;
	int rport;
	char *name, *pass, *cmd;
	int *fd2p;
{
	struct sockaddr_in sin, sin2, from;
	struct hostent *hp;
	u_short port;
	int s, timo = 1, s3;
	char c, *acct;

	hp = gethostbyname(*ahost);
	if (hp == 0) {
		herror(*ahost);
		return (-1);
	}
	*ahost = hp->h_name;
	acct = NULL;
	ruserpass(hp->h_name, &name, &pass, &acct);
	free(acct);
retry:
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		perror("rexec: socket");
		return (-1);
	}
	sin.sin_family = hp->h_addrtype;
	sin.sin_port = rport;
	bcopy(hp->h_addr, (caddr_t)&sin.sin_addr, hp->h_length);
	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		if (errno == ECONNREFUSED && timo <= 16) {
			(void) close(s);
			sleep(timo);
			timo *= 2;
			goto retry;
		}
		perror(hp->h_name);
		return (-1);
	}
	if (fd2p == 0) {
		(void) write(s, "", 1);
		port = 0;
	} else {
		char num[8];
		int s2, sin2len;

		s2 = socket(AF_INET, SOCK_STREAM, 0);
		if (s2 < 0) {
			(void) close(s);
			return (-1);
		}
		listen(s2, 1);
		sin2len = sizeof (sin2);
		if (getsockname(s2, (struct sockaddr *)&sin2, &sin2len) < 0 ||
		  sin2len != sizeof (sin2)) {
			perror("getsockname");
			(void) close(s2);
			goto bad;
		}
		port = ntohs((u_short)sin2.sin_port);
		(void) sprintf(num, "%u", port);
		(void) write(s, num, strlen(num)+1);
		{ int len = sizeof (from);
		  s3 = accept(s2, (struct sockaddr *)&from, &len);
		  close(s2);
		  if (s3 < 0) {
			perror("accept");
			port = 0;
			goto bad;
		  }
		}
		*fd2p = s3;
	}
	(void) write(s, name, strlen(name) + 1);
	/* should public key encypt the password here */
	(void) write(s, pass, strlen(pass) + 1);
	(void) write(s, cmd, strlen(cmd) + 1);
	if (read(s, &c, 1) != 1) {
		perror(*ahost);
		goto bad;
	}
	if (c != 0) {
		while (read(s, &c, 1) == 1) {
			(void) write(2, &c, 1);
			if (c == '\n')
				break;
		}
		goto bad;
	}
	return (s);
bad:
	if (port)
		(void) close(*fd2p);
	(void) close(s);
	return (-1);
}