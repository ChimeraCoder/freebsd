
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
 * Small replacement for 'dmesg'. It doesn't need libkvm nor /dev/kmem.
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/sysctl.h>

int
main(int argc, char *argv[])
{
	int len,i;
	char *buf,*p;
	char *mib="kern.msgbuf";

	/* We use sysctlbyname, because the oid is unknown (OID_AUTO) */

	/* get the buffer size */
	i=sysctlbyname(mib,NULL,&len,NULL,0);
	if(i) {
		perror("buffer sizing");
		exit(-1);
	}
	buf=(char *)malloc(len*sizeof(char));
	i=sysctlbyname(mib,buf,&len,NULL,0);
	if(i) {
		perror("retrieving data");
		exit(-1);
	}
	p=buf;
	i=0;
	while(p<(buf+len)) {
		switch(*p) {
		case '\0':
			/* skip initial NULLs */
			break;
		default:
			putchar(*p);
		}
		p++;
	}
	if(*--p!='\n') putchar('\n');
	free(buf);
	exit(0);
}