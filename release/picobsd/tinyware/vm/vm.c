
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
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/vmmeter.h>
#include <vm/vm_param.h>

#define pgtok(a) ((a) * (u_int) pagesize >> 10)

int
vm_i()
{
#define CNT	49
	int cnt[CNT];
	char names[CNT*16];
	char *a, *namep[CNT*16];
	int i,len;
	long long inttotal=0;
	long uptime=1;

	len=sizeof(cnt);
	i = sysctlbyname("hw.intrcnt", &cnt, &len, NULL, 0);
	if (i != 0)
		return i ;
	len=sizeof(names);
	i = sysctlbyname("hw.intrnames", &names, &len, NULL, 0);
	if (i != 0)
		return i ;
	
	for( i=0, a = names ; i < CNT && a < names+sizeof(names) ; ) {
	    namep[i++] = a++;
	    while (a < names+sizeof(names) && *a)
		a++ ;
	    a++ ; /* skip \0 */
	}
	printf("interrupt                   total       rate\n");
        inttotal = 0;
	for (i=0; i< CNT ; i++)
	    if (cnt[i] >0) {
		printf("%-12s %20lu %10lu\n", namep[i], cnt[i], cnt[i]/uptime);
                inttotal += cnt[i];
	    }
        printf("Total        %20llu %10llu\n", inttotal,
                        inttotal / (u_int64_t) uptime);
	return 0;
}
int
main(int argc, char *argv[])
{
	int mib[2],i=0,len;
	int pagesize, pagesize_len;
	struct vmtotal v;

	if (argc > 1 && !strcmp(argv[1], "-i")) {
	    if (vm_i())
		fprintf(stderr, "vm -i stats not available via sysctl\n");
		return 0 ;
	}
	pagesize_len = sizeof(int);
	sysctlbyname("vm.stats.vm.v_page_size",&pagesize,&pagesize_len,NULL,0);

	len=sizeof(struct vmtotal);
	mib[0]=CTL_VM;
	mib[1]=VM_METER;
	for(;;) {
		sysctl(mib,2,&v,&len,NULL,0);
		if(i==0) {
			printf("  procs    kB virt mem       real mem     shared vm   shared real    free\n");
			printf(" r w l s    tot     act    tot    act    tot    act    tot    act\n");
		}
		printf("%2hd%2hd%2hd%2hd",v.t_rq-1,v.t_dw+v.t_pw,v.t_sl,v.t_sw);
		printf("%7d %7d %7d%7d",
			pgtok(v.t_vm),pgtok(v.t_avm),
			pgtok(v.t_rm),pgtok(v.t_arm));
		printf("%7d%7d%7d%7d%7d\n",
			pgtok(v.t_vmshr),pgtok(v.t_avmshr),
			pgtok(v.t_rmshr),pgtok(v.t_armshr),
			pgtok(v.t_free));
		sleep(5);
		i++;
		if(i>22) i=0;
	}
	exit(0);

}