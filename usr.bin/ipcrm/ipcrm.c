
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
#define _KERNEL
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#undef _KERNEL

#include <ctype.h>
#include <err.h>
#include <grp.h>
#include <kvm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ipc.h"

int	signaled;
int	errflg;
int	rmverbose = 0;

void	usage(void);

int	msgrm(key_t, int);
int	shmrm(key_t, int);
int	semrm(key_t, int);
void	not_configured(int);

void
usage(void)
{

	fprintf(stderr,
	    "usage: ipcrm [-W] [-v[v]]\n"
	    "             [-q msqid] [-m shmid] [-s semid]\n"
	    "             [-Q msgkey] [-M shmkey] [-S semkey] ...\n");
	exit(1);
}

int
msgrm(key_t key, int id)
{

	if (key == -1 || id == -1) {
		struct msqid_kernel *kxmsqids;
		size_t kxmsqids_len;
		int num;

		kget(X_MSGINFO, &msginfo, sizeof(msginfo));
		kxmsqids_len = sizeof(struct msqid_kernel) * msginfo.msgmni;
		kxmsqids = malloc(kxmsqids_len);
		kget(X_MSQIDS, kxmsqids, kxmsqids_len);
		num = msginfo.msgmni;
		while (num-- && !signaled)
			if (kxmsqids[num].u.msg_qbytes != 0) {
				id = IXSEQ_TO_IPCID(num,
					kxmsqids[num].u.msg_perm);
				if (msgctl(id, IPC_RMID, NULL) < 0) {
					if (rmverbose > 1)
						warn("msqid(%d): ", id);
					errflg++;
				} else
					if (rmverbose)
						printf(
						    "Removed %s %d\n",
						    IPC_TO_STRING('Q'),
						    id);
			}
		return signaled ? -1 : 0;       /* errors maybe handled above */
	}

	if (key) {
		id = msgget(key, 0);
		if (id == -1)
			return -1;
	}

	return msgctl(id, IPC_RMID, NULL);
}

int
shmrm(key_t key, int id)
{

	if (key == -1 || id == -1) {
		struct shmid_kernel *kxshmids;
		size_t kxshmids_len;
		int num;

		kget(X_SHMINFO, &shminfo, sizeof(shminfo));
		kxshmids_len = sizeof(struct shmid_kernel) * shminfo.shmmni;
		kxshmids = malloc(kxshmids_len);
		kget(X_SHMSEGS, kxshmids, kxshmids_len);
		num = shminfo.shmmni;
		while (num-- && !signaled)
			if (kxshmids[num].u.shm_perm.mode & 0x0800) {
				id = IXSEQ_TO_IPCID(num,
					kxshmids[num].u.shm_perm);
				if (shmctl(id, IPC_RMID, NULL) < 0) {
					if (rmverbose > 1)
						warn("shmid(%d): ", id);
					errflg++;
				} else
					if (rmverbose)
						printf(
						    "Removed %s %d\n",
						    IPC_TO_STRING('M'),
						    id);
			}
		return signaled ? -1 : 0;       /* errors maybe handled above */
	}

	if (key) {
		id = shmget(key, 0, 0);
		if (id == -1)
			return -1;
	}

	return shmctl(id, IPC_RMID, NULL);
}

int
semrm(key_t key, int id)
{
	union semun arg;

	if (key == -1 || id == -1) {
		struct semid_kernel *kxsema;
		size_t kxsema_len;
		int num;

		kget(X_SEMINFO, &seminfo, sizeof(seminfo));
		kxsema_len = sizeof(struct semid_kernel) * seminfo.semmni;
		kxsema = malloc(kxsema_len);
		kget(X_SEMA, kxsema, kxsema_len);
		num = seminfo.semmni;
		while (num-- && !signaled)
			if ((kxsema[num].u.sem_perm.mode & SEM_ALLOC) != 0) {
				id = IXSEQ_TO_IPCID(num,
					kxsema[num].u.sem_perm);
				if (semctl(id, IPC_RMID, NULL) < 0) {
					if (rmverbose > 1)
						warn("semid(%d): ", id);
					errflg++;
				} else
					if (rmverbose)
						printf(
						    "Removed %s %d\n",
						    IPC_TO_STRING('S'),
						    id);
			}
		return signaled ? -1 : 0;       /* errors maybe handled above */
	}

	if (key) {
		id = semget(key, 0, 0);
		if (id == -1)
			return -1;
	}

	return semctl(id, 0, IPC_RMID, arg);
}

void
not_configured(int signo __unused)
{

	signaled++;
}

int
main(int argc, char *argv[])
{
	int c, result, target_id;
	key_t target_key;

	while ((c = getopt(argc, argv, "q:m:s:Q:M:S:vWy")) != -1) {

		signaled = 0;
		switch (c) {
		case 'v':
			rmverbose++;
			break;
		case 'y':
			use_sysctl = 0;
			break;
		}
	}

	optind = 1;
	errflg = 0;
	signal(SIGSYS, not_configured);
	while ((c = getopt(argc, argv, "q:m:s:Q:M:S:vWy")) != -1) {

		signaled = 0;
		switch (c) {
		case 'q':
		case 'm':
		case 's':
			target_id = atoi(optarg);
			if (c == 'q')
				result = msgrm(0, target_id);
			else if (c == 'm')
				result = shmrm(0, target_id);
			else
				result = semrm(0, target_id);
			if (result < 0) {
				errflg++;
				if (!signaled)
					warn("%sid(%d): ",
					    IPC_TO_STR(toupper(c)), target_id);
				else
					warnx(
					    "%ss are not configured "
					    "in the running kernel",
					    IPC_TO_STRING(toupper(c)));
			}
			break;
		case 'Q':
		case 'M':
		case 'S':
			target_key = atol(optarg);
			if (target_key == IPC_PRIVATE) {
				warnx("can't remove private %ss",
				    IPC_TO_STRING(c));
				continue;
			}
			if (c == 'Q')
				result = msgrm(target_key, 0);
			else if (c == 'M')
				result = shmrm(target_key, 0);
			else
				result = semrm(target_key, 0);
			if (result < 0) {
				errflg++;
				if (!signaled)
					warn("%ss(%ld): ",
					    IPC_TO_STR(c), target_key);
				else
					warnx("%ss are not configured "
					    "in the running kernel",
					    IPC_TO_STRING(c));
			}
			break;
		case 'v':
		case 'y':
			/* Handled in other getopt() loop */
			break;
		case 'W':
			msgrm(-1, 0);
			shmrm(-1, 0);
			semrm(-1, 0);
			break;
		case ':':
			fprintf(stderr,
			    "option -%c requires an argument\n", optopt);
			usage();
		case '?':
			fprintf(stderr, "unrecognized option: -%c\n", optopt);
			usage();
		}
	}

	if (optind != argc) {
		fprintf(stderr, "unknown argument: %s\n", argv[optind]);
		usage();
	}
	exit(errflg);
}