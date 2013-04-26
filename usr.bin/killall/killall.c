
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

#include <sys/param.h>
#include <sys/jail.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/user.h>
#include <sys/sysctl.h>
#include <fcntl.h>
#include <dirent.h>
#include <jail.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <signal.h>
#include <regex.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <locale.h>

static void __dead2
usage(void)
{

	fprintf(stderr, "usage: killall [-delmsvz] [-help] [-I] [-j jail]\n");
	fprintf(stderr,
	    "               [-u user] [-t tty] [-c cmd] [-SIGNAL] [cmd]...\n");
	fprintf(stderr, "At least one option or argument to specify processes must be given.\n");
	exit(1);
}


static void
printsig(FILE *fp)
{
	const char	*const * p;
	int		cnt;
	int		offset = 0;

	for (cnt = NSIG, p = sys_signame + 1; --cnt; ++p) {
		offset += fprintf(fp, "%s ", *p);
		if (offset >= 75 && cnt > 1) {
			offset = 0;
			fprintf(fp, "\n");
		}
	}
	fprintf(fp, "\n");
}

static void
nosig(char *name)
{

	warnx("unknown signal %s; valid signals:", name);
	printsig(stderr);
	exit(1);
}

int
main(int ac, char **av)
{
	struct kinfo_proc *procs, *newprocs;
	struct stat	sb;
	struct passwd	*pw;
	regex_t		rgx;
	regmatch_t	pmatch;
	int		i, j, ch;
	char		buf[256];
	char		first;
	char		*user = NULL;
	char		*tty = NULL;
	char		*cmd = NULL;
	int		vflag = 0;
	int		sflag = 0;
	int		dflag = 0;
	int		eflag = 0;
	int		Iflag = 0;
	int		jflag = 0;
	int		mflag = 0;
	int		zflag = 0;
	uid_t		uid = 0;
	dev_t		tdev = 0;
	pid_t		mypid;
	char		thiscmd[MAXCOMLEN + 1];
	pid_t		thispid;
	uid_t		thisuid;
	dev_t		thistdev;
	int		sig = SIGTERM;
	const char *const *p;
	char		*ep;
	int		errors = 0;
	int		jid;
	int		mib[4];
	size_t		miblen;
	int		st, nprocs;
	size_t		size;
	int		matched;
	int		killed = 0;

	setlocale(LC_ALL, "");

	av++;
	ac--;

	while (ac > 0) {
		if (strcmp(*av, "-l") == 0) {
			printsig(stdout);
			exit(0);
		}
		if (strcmp(*av, "-help") == 0)
			usage();
		if (**av == '-') {
			++*av;
			switch (**av) {
			case 'I':
				Iflag = 1;
				break;
			case 'j':
				++*av;
				if (**av == '\0') {
					++av;
					--ac;
				}
				jflag++;
				if (*av == NULL)
				    	errx(1, "must specify jail");
				jid = jail_getid(*av);
				if (jid < 0)
					errx(1, "%s", jail_errmsg);
				if (jail_attach(jid) == -1)
					err(1, "jail_attach(%d)", jid);
				break;
			case 'u':
				++*av;
				if (**av == '\0') {
					++av;
					--ac;
				}
				if (*av == NULL)
				    	errx(1, "must specify user");
				user = *av;
				break;
			case 't':
				++*av;
				if (**av == '\0') {
					++av;
					--ac;
				}
				if (*av == NULL)
				    	errx(1, "must specify tty");
				tty = *av;
				break;
			case 'c':
				++*av;
				if (**av == '\0') {
					++av;
					--ac;
				}
				if (*av == NULL)
				    	errx(1, "must specify procname");
				cmd = *av;
				break;
			case 'v':
				vflag++;
				break;
			case 's':
				sflag++;
				break;
			case 'd':
				dflag++;
				break;
			case 'e':
				eflag++;
				break;
			case 'm':
				mflag++;
				break;
			case 'z':
				zflag++;
				break;
			default:
				if (isalpha((unsigned char)**av)) {
					if (strncasecmp(*av, "SIG", 3) == 0)
						*av += 3;
					for (sig = NSIG, p = sys_signame + 1;
					     --sig; ++p)
						if (strcasecmp(*p, *av) == 0) {
							sig = p - sys_signame;
							break;
						}
					if (!sig)
						nosig(*av);
				} else if (isdigit((unsigned char)**av)) {
					sig = strtol(*av, &ep, 10);
					if (!*av || *ep)
						errx(1, "illegal signal number: %s", *av);
					if (sig < 0 || sig >= NSIG)
						nosig(*av);
				} else
					nosig(*av);
			}
			++av;
			--ac;
		} else {
			break;
		}
	}

	if (user == NULL && tty == NULL && cmd == NULL && !jflag && ac == 0)
		usage();

	if (tty) {
		if (strncmp(tty, "/dev/", 5) == 0)
			snprintf(buf, sizeof(buf), "%s", tty);
		else if (strncmp(tty, "tty", 3) == 0)
			snprintf(buf, sizeof(buf), "/dev/%s", tty);
		else
			snprintf(buf, sizeof(buf), "/dev/tty%s", tty);
		if (stat(buf, &sb) < 0)
			err(1, "stat(%s)", buf);
		if (!S_ISCHR(sb.st_mode))
			errx(1, "%s: not a character device", buf);
		tdev = sb.st_rdev;
		if (dflag)
			printf("ttydev:0x%x\n", tdev);
	}
	if (user) {
		uid = strtol(user, &ep, 10);
		if (*user == '\0' || *ep != '\0') { /* was it a number? */
			pw = getpwnam(user);
			if (pw == NULL)
				errx(1, "user %s does not exist", user);
			uid = pw->pw_uid;
			if (dflag)
				printf("uid:%d\n", uid);
		}
	} else {
		uid = getuid();
		if (uid != 0) {
			pw = getpwuid(uid);
			if (pw)
				user = pw->pw_name;
			if (dflag)
				printf("uid:%d\n", uid);
		}
	}
	size = 0;
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;

	if (user) {
		mib[2] = eflag ? KERN_PROC_UID : KERN_PROC_RUID;
		mib[3] = uid;
		miblen = 4;
	} else if (tty) {
		mib[2] = KERN_PROC_TTY;
		mib[3] = tdev;
		miblen = 4;
	} else {
		mib[2] = KERN_PROC_PROC;
		mib[3] = 0;
		miblen = 3;
	}

	procs = NULL;
	st = sysctl(mib, miblen, NULL, &size, NULL, 0);
	do {
		size += size / 10;
		newprocs = realloc(procs, size);
		if (newprocs == NULL) {
			free(procs);
			err(1, "could not reallocate memory");
		}
		procs = newprocs;
		st = sysctl(mib, miblen, procs, &size, NULL, 0);
	} while (st == -1 && errno == ENOMEM);
	if (st == -1)
		err(1, "could not sysctl(KERN_PROC)");
	if (size % sizeof(struct kinfo_proc) != 0) {
		fprintf(stderr, "proc size mismatch (%zu total, %zu chunks)\n",
			size, sizeof(struct kinfo_proc));
		fprintf(stderr, "userland out of sync with kernel\n");
		exit(1);
	}
	nprocs = size / sizeof(struct kinfo_proc);
	if (dflag)
		printf("nprocs %d\n", nprocs);
	mypid = getpid();

	for (i = 0; i < nprocs; i++) {
		if (procs[i].ki_stat == SZOMB && !zflag)
			continue;
		thispid = procs[i].ki_pid;
		strlcpy(thiscmd, procs[i].ki_comm, sizeof(thiscmd));
		thistdev = procs[i].ki_tdev;
		if (eflag)
			thisuid = procs[i].ki_uid;	/* effective uid */
		else
			thisuid = procs[i].ki_ruid;	/* real uid */

		if (thispid == mypid)
			continue;
		matched = 1;
		if (user) {
			if (thisuid != uid)
				matched = 0;
		}
		if (tty) {
			if (thistdev != tdev)
				matched = 0;
		}
		if (cmd) {
			if (mflag) {
				if (regcomp(&rgx, cmd,
				    REG_EXTENDED|REG_NOSUB) != 0) {
					mflag = 0;
					warnx("%s: illegal regexp", cmd);
				}
			}
			if (mflag) {
				pmatch.rm_so = 0;
				pmatch.rm_eo = strlen(thiscmd);
				if (regexec(&rgx, thiscmd, 0, &pmatch,
				    REG_STARTEND) != 0)
					matched = 0;
				regfree(&rgx);
			} else {
				if (strncmp(thiscmd, cmd, MAXCOMLEN) != 0)
					matched = 0;
			}
		}
		if (jflag && thispid == getpid())
			matched = 0;
		if (matched == 0)
			continue;
		if (ac > 0)
			matched = 0;
		for (j = 0; j < ac; j++) {
			if (mflag) {
				if (regcomp(&rgx, av[j],
				    REG_EXTENDED|REG_NOSUB) != 0) {
					mflag = 0;
					warnx("%s: illegal regexp", av[j]);
				}
			}
			if (mflag) {
				pmatch.rm_so = 0;
				pmatch.rm_eo = strlen(thiscmd);
				if (regexec(&rgx, thiscmd, 0, &pmatch,
				    REG_STARTEND) == 0)
					matched = 1;
				regfree(&rgx);
			} else {
				if (strcmp(thiscmd, av[j]) == 0)
					matched = 1;
			}
			if (matched)
				break;
		}
		if (matched != 0 && Iflag) {
			printf("Send signal %d to %s (pid %d uid %d)? ",
				sig, thiscmd, thispid, thisuid);
			fflush(stdout);
			first = ch = getchar();
			while (ch != '\n' && ch != EOF)
				ch = getchar();
			if (first != 'y' && first != 'Y')
				matched = 0;
		}
		if (matched == 0)
			continue;
		if (dflag)
			printf("sig:%d, cmd:%s, pid:%d, dev:0x%x uid:%d\n", sig,
			    thiscmd, thispid, thistdev, thisuid);

		if (vflag || sflag)
			printf("kill -%s %d\n", sys_signame[sig], thispid);

		killed++;
		if (!dflag && !sflag) {
			if (kill(thispid, sig) < 0 /* && errno != ESRCH */ ) {
				warn("warning: kill -%s %d",
				    sys_signame[sig], thispid);
				errors = 1;
			}
		}
	}
	if (killed == 0) {
		fprintf(stderr, "No matching processes %swere found\n",
		    getuid() != 0 ? "belonging to you " : "");
		errors = 1;
	}
	exit(errors);
}