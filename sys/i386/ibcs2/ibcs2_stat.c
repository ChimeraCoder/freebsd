
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
#include <sys/systm.h>
#include <sys/namei.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/filedesc.h>
#include <sys/jail.h>
#include <sys/kernel.h>
#include <sys/mount.h>
#include <sys/malloc.h>
#include <sys/vnode.h>
#include <sys/syscallsubr.h>
#include <sys/sysctl.h>
#include <sys/sysproto.h>

#include <i386/ibcs2/ibcs2_signal.h>
#include <i386/ibcs2/ibcs2_stat.h>
#include <i386/ibcs2/ibcs2_statfs.h>
#include <i386/ibcs2/ibcs2_proto.h>
#include <i386/ibcs2/ibcs2_util.h>
#include <i386/ibcs2/ibcs2_utsname.h>


static void bsd_stat2ibcs_stat(struct stat *, struct ibcs2_stat *);
static int  cvt_statfs(struct statfs *, caddr_t, int);

static void
bsd_stat2ibcs_stat(st, st4)
	struct stat *st;
	struct ibcs2_stat *st4;
{
	bzero(st4, sizeof(*st4));
	st4->st_dev  = (ibcs2_dev_t)st->st_dev;
	st4->st_ino  = (ibcs2_ino_t)st->st_ino;
	st4->st_mode = (ibcs2_mode_t)st->st_mode;
	st4->st_nlink= (ibcs2_nlink_t)st->st_nlink;
	st4->st_uid  = (ibcs2_uid_t)st->st_uid;
	st4->st_gid  = (ibcs2_gid_t)st->st_gid;
	st4->st_rdev = (ibcs2_dev_t)st->st_rdev;
	if (st->st_size < (quad_t)1 << 32)
		st4->st_size = (ibcs2_off_t)st->st_size;
	else
		st4->st_size = -2;
	st4->st_atim = (ibcs2_time_t)st->st_atim.tv_sec;
	st4->st_mtim = (ibcs2_time_t)st->st_mtim.tv_sec;
	st4->st_ctim = (ibcs2_time_t)st->st_ctim.tv_sec;
}

static int
cvt_statfs(sp, buf, len)
	struct statfs *sp;
	caddr_t buf;
	int len;
{
	struct ibcs2_statfs ssfs;

	if (len < 0)
		return (EINVAL);
	else if (len > sizeof(ssfs))
		len = sizeof(ssfs);
	bzero(&ssfs, sizeof ssfs);
	ssfs.f_fstyp = 0;
	ssfs.f_bsize = sp->f_bsize;
	ssfs.f_frsize = 0;
	ssfs.f_blocks = sp->f_blocks;
	ssfs.f_bfree = sp->f_bfree;
	ssfs.f_files = sp->f_files;
	ssfs.f_ffree = sp->f_ffree;
	ssfs.f_fname[0] = 0;
	ssfs.f_fpack[0] = 0;
	return copyout((caddr_t)&ssfs, buf, len);
}	

int
ibcs2_statfs(td, uap)
	struct thread *td;
	struct ibcs2_statfs_args *uap;
{
	struct statfs sf;
	char *path;
	int error;

	CHECKALTEXIST(td, uap->path, &path);
	error = kern_statfs(td, path, UIO_SYSSPACE, &sf);
	free(path, M_TEMP);
	if (error)
		return (error);
	return cvt_statfs(&sf, (caddr_t)uap->buf, uap->len);
}

int
ibcs2_fstatfs(td, uap)
	struct thread *td;
	struct ibcs2_fstatfs_args *uap;
{
	struct statfs sf;
	int error;

	error = kern_fstatfs(td, uap->fd, &sf);
	if (error)
		return (error);
	return cvt_statfs(&sf, (caddr_t)uap->buf, uap->len);
}

int
ibcs2_stat(td, uap)
	struct thread *td;
	struct ibcs2_stat_args *uap;
{
	struct ibcs2_stat ibcs2_st;
	struct stat st;
	char *path;
	int error;

	CHECKALTEXIST(td, uap->path, &path);

	error = kern_stat(td, path, UIO_SYSSPACE, &st);
	free(path, M_TEMP);
	if (error)
		return (error);
	bsd_stat2ibcs_stat(&st, &ibcs2_st);
	return copyout((caddr_t)&ibcs2_st, (caddr_t)uap->st,
		       ibcs2_stat_len);
}

int
ibcs2_lstat(td, uap)
	struct thread *td;
	struct ibcs2_lstat_args *uap;
{
	struct ibcs2_stat ibcs2_st;
	struct stat st;
	char *path;
	int error;

	CHECKALTEXIST(td, uap->path, &path);

	error = kern_lstat(td, path, UIO_SYSSPACE, &st);
	free(path, M_TEMP);
	if (error)
		return (error);
	bsd_stat2ibcs_stat(&st, &ibcs2_st);
	return copyout((caddr_t)&ibcs2_st, (caddr_t)uap->st,
		       ibcs2_stat_len);
}

int
ibcs2_fstat(td, uap)
	struct thread *td;
	struct ibcs2_fstat_args *uap;
{
	struct ibcs2_stat ibcs2_st;
	struct stat st;
	int error;

	error = kern_fstat(td, uap->fd, &st);
	if (error)
		return (error);
	bsd_stat2ibcs_stat(&st, &ibcs2_st);
	return copyout((caddr_t)&ibcs2_st, (caddr_t)uap->st,
		       ibcs2_stat_len);
}

int
ibcs2_utssys(td, uap)
	struct thread *td;
	struct ibcs2_utssys_args *uap;
{
	switch (uap->flag) {
	case 0:			/* uname(2) */
	{
		char machine_name[9], *p;
		struct ibcs2_utsname sut;
		bzero(&sut, ibcs2_utsname_len);

		strncpy(sut.sysname,
			IBCS2_UNAME_SYSNAME, sizeof(sut.sysname) - 1);
		strncpy(sut.release,
			IBCS2_UNAME_RELEASE, sizeof(sut.release) - 1);
		strncpy(sut.version,
			IBCS2_UNAME_VERSION, sizeof(sut.version) - 1);
		getcredhostname(td->td_ucred, machine_name,
		    sizeof(machine_name) - 1);
		p = strchr(machine_name, '.');
		if ( p )
			*p = '\0';
		strncpy(sut.nodename, machine_name, sizeof(sut.nodename) - 1);
		strncpy(sut.machine, machine, sizeof(sut.machine) - 1);

		DPRINTF(("IBCS2 uname: sys=%s rel=%s ver=%s node=%s mach=%s\n",
			 sut.sysname, sut.release, sut.version, sut.nodename,
			 sut.machine));
		return copyout((caddr_t)&sut, (caddr_t)uap->a1,
			       ibcs2_utsname_len);
	}

	case 2:			/* ustat(2) */
	{
		return ENOSYS;	/* XXX - TODO */
	}

	default:
		return ENOSYS;
	}
}