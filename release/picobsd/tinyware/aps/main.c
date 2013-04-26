
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

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

/*
 * Ok, I could extract almost anything from /proc, but I'm too lazy...
 * I think it will suffice for now.
 */

int
main(int argc, char *argv[])
{
	DIR *d;
	struct dirent *e;
	FILE *fd;
	char buf[100];
	char *tok, *sep=" ", *sep1=",";
	char *name, *pid, *ppid, *uid, *gid;
	char *pgid, *sid, *tty, *cred;
	char *major, *minor;
	char con[10];

	d=opendir("/proc");
	printf(" PID   PPID  TTY  COMMAND\n");
	while((e=readdir(d))!=NULL) {
		/* Skip '.' and '..' */
		if(e->d_name[0]=='.') continue;
		/* Skip 'curproc' - it's us */
		if(e->d_name[0]=='c') continue;
		sprintf(buf,"/proc/%s/status",e->d_name);
		fd=fopen(buf,"r");
		fgets(buf,99,fd);
		fclose(fd);
		name=strtok(buf,sep);
		pid=strtok(NULL,sep);
		ppid=strtok(NULL,sep);
		pgid=strtok(NULL,sep);
		sid=strtok(NULL,sep);
		tty=strtok(NULL,sep);
		tok=strtok(NULL,sep); /* flags */
		tok=strtok(NULL,sep); /* start */
		tok=strtok(NULL,sep); /* user time */
		tok=strtok(NULL,sep); /* system time */
		tok=strtok(NULL,sep); /* wchan */
		cred=strtok(NULL,sep); /* credentials */
		major=strtok(tty,sep1);
		minor=strtok(NULL,sep1);
		if(strcmp(minor,"-1")==0) {
			minor="?";
		}
		if(strcmp(major,"-1")==0) {
			major="?";
		} else if(strcmp(major,"12")==0) {
			major="v";
		} else if(strcmp(major,"0")==0) {
			major="con";
			minor="-";
		} else if(strcmp(major,"5")==0) {
			major="p";
		} else major="x";
		if((strcmp(major,"v")==0) && (strcmp(minor,"255")==0)) {
			major="con";
			minor="-";
		}
		sprintf(con,"%s%s",major,minor);
		printf("%5s %5s %4s (%s)\n",pid,ppid,con,name);

	}
	closedir(d);
	exit(0);
}