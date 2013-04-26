
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

#include <sys/param.h>
#include <sys/queue.h>
#include <sys/exec.h>
#include <sys/lock.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/mount.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/sbuf.h>
#include <sys/sysproto.h>
#include <sys/systm.h>
#include <sys/vnode.h>

#include <vm/vm.h>
#include <vm/pmap.h>
#include <vm/vm_param.h>

#include <fs/pseudofs/pseudofs.h>
#include <fs/procfs/procfs.h>

/*
 * Filler function for proc/pid/self
 */
int
procfs_doprocfile(PFS_FILL_ARGS)
{
	char *fullpath;
	char *freepath;
	struct vnode *textvp;
	int error;

	freepath = NULL;
	PROC_LOCK(p);
	textvp = p->p_textvp;
	vhold(textvp);
	PROC_UNLOCK(p);
	error = vn_fullpath(td, textvp, &fullpath, &freepath);
	vdrop(textvp);
	if (error == 0)
		sbuf_printf(sb, "%s", fullpath);
	if (freepath != NULL)
		free(freepath, M_TEMP);
	return (error);
}

/*
 * Filler function for proc/curproc
 */
int
procfs_docurproc(PFS_FILL_ARGS)
{
	sbuf_printf(sb, "%ld", (long)td->td_proc->p_pid);
	return (0);
}

/*
 * Adjust mode for some nodes that need it
 */
int
procfs_attr(PFS_ATTR_ARGS)
{

	/* XXX inefficient, split into separate functions */
	if (strcmp(pn->pn_name, "ctl") == 0 ||
	    strcmp(pn->pn_name, "note") == 0 ||
	    strcmp(pn->pn_name, "notepg") == 0)
		vap->va_mode = 0200;
	else if (strcmp(pn->pn_name, "mem") == 0 ||
	    strcmp(pn->pn_name, "regs") == 0 ||
	    strcmp(pn->pn_name, "dbregs") == 0 ||
	    strcmp(pn->pn_name, "fpregs") == 0 ||
	    strcmp(pn->pn_name, "osrel") == 0)
		vap->va_mode = 0600;

	if (p != NULL) {
		PROC_LOCK_ASSERT(p, MA_OWNED);

		if ((p->p_flag & P_SUGID) && pn->pn_type != pfstype_procdir)
			vap->va_mode = 0;
	}

	return (0);
}

/*
 * Visibility: some files only exist for non-system processes
 * Non-static because linprocfs uses it.
 */
int
procfs_notsystem(PFS_VIS_ARGS)
{
	PROC_LOCK_ASSERT(p, MA_OWNED);
	return ((p->p_flag & P_SYSTEM) == 0);
}

/*
 * Visibility: some files are only visible to process that can debug
 * the target process.
 */
int
procfs_candebug(PFS_VIS_ARGS)
{
	PROC_LOCK_ASSERT(p, MA_OWNED);
	return ((p->p_flag & P_SYSTEM) == 0 && p_candebug(td, p) == 0);
}

/*
 * Constructor
 */
static int
procfs_init(PFS_INIT_ARGS)
{
	struct pfs_node *root;
	struct pfs_node *dir;
	struct pfs_node *node;

	root = pi->pi_root;

	pfs_create_link(root, "curproc", procfs_docurproc,
	    NULL, NULL, NULL, 0);

	dir = pfs_create_dir(root, "pid",
	    procfs_attr, NULL, NULL, PFS_PROCDEP);
	pfs_create_file(dir, "cmdline", procfs_doproccmdline,
	    NULL, NULL, NULL, PFS_RD);
	pfs_create_file(dir, "ctl", procfs_doprocctl,
	    procfs_attr, NULL, NULL, PFS_WR);
	pfs_create_file(dir, "dbregs", procfs_doprocdbregs,
	    procfs_attr, procfs_candebug, NULL, PFS_RDWR|PFS_RAW);
	pfs_create_file(dir, "etype", procfs_doproctype,
	    NULL, NULL, NULL, PFS_RD);
	pfs_create_file(dir, "fpregs", procfs_doprocfpregs,
	    procfs_attr, procfs_candebug, NULL, PFS_RDWR|PFS_RAW);
	pfs_create_file(dir, "map", procfs_doprocmap,
	    NULL, procfs_notsystem, NULL, PFS_RD);
	node = pfs_create_file(dir, "mem", procfs_doprocmem,
	    procfs_attr, procfs_candebug, NULL, PFS_RDWR|PFS_RAW);
	node->pn_ioctl = procfs_ioctl;
	node->pn_close = procfs_close;
	pfs_create_file(dir, "note", procfs_doprocnote,
	    procfs_attr, procfs_candebug, NULL, PFS_WR);
	pfs_create_file(dir, "notepg", procfs_doprocnote,
	    procfs_attr, procfs_candebug, NULL, PFS_WR);
	pfs_create_file(dir, "regs", procfs_doprocregs,
	    procfs_attr, procfs_candebug, NULL, PFS_RDWR|PFS_RAW);
	pfs_create_file(dir, "rlimit", procfs_doprocrlimit,
	    NULL, NULL, NULL, PFS_RD);
	pfs_create_file(dir, "status", procfs_doprocstatus,
	    NULL, NULL, NULL, PFS_RD);
	pfs_create_file(dir, "osrel", procfs_doosrel,
	    procfs_attr, procfs_candebug, NULL, PFS_RDWR);

	pfs_create_link(dir, "file", procfs_doprocfile,
	    NULL, procfs_notsystem, NULL, 0);

	return (0);
}

/*
 * Destructor
 */
static int
procfs_uninit(PFS_INIT_ARGS)
{
	/* nothing to do, pseudofs will GC */
	return (0);
}

PSEUDOFS(procfs, 1, PR_ALLOW_MOUNT_PROCFS);