
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

/*
 * Small replacement for ps(1) - uses only sysctl(3) to retrieve info
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/stat.h>
#include <sys/user.h>

char p_stat[] = "?iRSTZWM";

int
main(int argc, char *argv[])
{
	int mib[4], i, num, len, j, plen;
	char buf[MAXPATHLEN], vty[5], pst[5], wmesg[10];
	struct kinfo_proc *ki;
	char *t;
	int ma, mi;

	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_ALL;
	if (sysctl(mib, 3, NULL, &len, NULL, 0) != 0) {
		perror("sysctl sizing");
		exit(1);
	}
	t = (char *)malloc(len);
	if (sysctl(mib, 3, t, &len, NULL, 0) != 0) {
		perror("sysctl info");
		exit(1);
	}
	mib[2] = KERN_PROC_ARGS;
	num = len / KINFO_PROC_SIZE;
	i = 0;
	printf("USERNAME   PID  PPID PRI NICE TTY STAT WCHAN   COMMAND\n");
	while(i < num) {
		ki = (struct kinfo_proc *)(t + (num - i - 1) * KINFO_PROC_SIZE);
		mib[3] = ki->ki_pid;
		plen = MAXPATHLEN;
		if (sysctl(mib, 4, buf, &plen, NULL, 0) != 0) {
			perror("sysctl cmd info");
			exit(1);
		}
		if (plen == 0) {
			sprintf(buf, "(%s)", ki->ki_comm);
		} else {
			for (j = 0; j < plen - 1; j++) {
				if (buf[j] == '\0') buf[j] = ' ';
			}
		}
		if (strcmp(ki->ki_wmesg, "") == 0) {
			sprintf(wmesg, "-");
		} else {
			strcpy(wmesg, ki->ki_wmesg);
		}
		ma = major(ki->ki_tdev);
		mi = minor(ki->ki_tdev);
		switch(ma) {
		case 255:
			strcpy(vty, "??");
			break;
		case 12:
			if(mi != 255) {
				sprintf(vty, "v%d", mi);
				break;
			}
			/* FALLTHROUGH */
		case 0:
			strcpy(vty, "con");
			break;
		case 5:
			sprintf(vty, "p%d", mi);
			break;
		}
		sprintf(pst, "%c", p_stat[ki->ki_stat]);
		printf("%8s %5u %5u %3d %4d %3s %-4s %-7s %s\n",
			ki->ki_login,
			ki->ki_pid,
			ki->ki_ppid,
			ki->ki_pri.pri_level, /* XXX check this */
			ki->ki_nice,
			vty,
			pst,
			wmesg,
			buf);
		i++;
	}
	free((void *)t);
	exit(0);
}