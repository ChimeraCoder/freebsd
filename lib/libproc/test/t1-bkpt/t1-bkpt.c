
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
#include <sys/types.h>
#include <sys/wait.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <libproc.h>

int __noinline
t1_bkpt_t()
{
	printf("TEST OK\n");
}

int
t1_bkpt_d()
{
	struct proc_handle *phdl;
	char *targv[] = { "t1-bkpt-t", NULL};
	unsigned long saved;

	proc_create("./t1-bkpt", targv, NULL, NULL, &phdl);
	assert(proc_bkptset(phdl, (uintptr_t)t1_bkpt_t, &saved) == 0);
	proc_continue(phdl);
	assert(proc_wstatus(phdl) == PS_STOP);
	proc_bkptexec(phdl, saved);
	proc_continue(phdl);
	proc_wstatus(phdl);
	proc_free(phdl);
}


int
main(int argc, char **argv)
{
	if (!strcmp(argv[0], "t1-bkpt-t"))
		t1_bkpt_t();
	else
		t1_bkpt_d();
}