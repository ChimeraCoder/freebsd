
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
#include <sys/file.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/condvar.h>
#include <sys/conf.h>
#include <sys/fcntl.h>
#include <sys/ioccom.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <sys/mutex.h>
#include <sys/poll.h>
#include <sys/proc.h>
#include <sys/queue.h>
#include <sys/syscall.h>
#include <sys/sysent.h>
#include <sys/sysproto.h>
#include <sys/uio.h>

#if __FreeBSD_version >= 900041
#include <sys/capability.h>
#endif

#include "filemon.h"

#if defined(COMPAT_IA32) || defined(COMPAT_FREEBSD32) || defined(COMPAT_ARCH32)
#include <compat/freebsd32/freebsd32_syscall.h>
#include <compat/freebsd32/freebsd32_proto.h>

extern struct sysentvec ia32_freebsd_sysvec;
#endif

extern struct sysentvec elf32_freebsd_sysvec;
extern struct sysentvec elf64_freebsd_sysvec;

static d_close_t	filemon_close;
static d_ioctl_t	filemon_ioctl;
static d_open_t		filemon_open;
static int		filemon_unload(void);
static void		filemon_load(void *);

static struct cdevsw filemon_cdevsw = {
	.d_version	= D_VERSION,
	.d_close	= filemon_close,
	.d_ioctl	= filemon_ioctl,
	.d_open		= filemon_open,
	.d_name		= "filemon",
};

MALLOC_DECLARE(M_FILEMON);
MALLOC_DEFINE(M_FILEMON, "filemon", "File access monitor");

struct filemon {
	TAILQ_ENTRY(filemon) link;	/* Link into the in-use list. */
	struct mtx	mtx;		/* Lock mutex for this filemon. */
	struct cv	cv;		/* Lock condition variable for this
					   filemon. */
	struct file	*fp;		/* Output file pointer. */
	struct thread	*locker;	/* Ptr to the thread locking this
					   filemon. */
	pid_t		pid;		/* The process ID being monitored. */
	char		fname1[MAXPATHLEN]; /* Temporary filename buffer. */
	char		fname2[MAXPATHLEN]; /* Temporary filename buffer. */
	char		msgbufr[1024];	/* Output message buffer. */
};

static TAILQ_HEAD(, filemon) filemons_inuse = TAILQ_HEAD_INITIALIZER(filemons_inuse);
static TAILQ_HEAD(, filemon) filemons_free = TAILQ_HEAD_INITIALIZER(filemons_free);
static int n_readers = 0;
static struct mtx access_mtx;
static struct cv access_cv;
static struct thread *access_owner = NULL;
static struct thread *access_requester = NULL;

static struct cdev *filemon_dev;

#include "filemon_lock.c"
#include "filemon_wrapper.c"

static void
filemon_dtr(void *data)
{
	struct filemon *filemon = data;

	if (filemon != NULL) {
		struct file *fp = filemon->fp;

		/* Get exclusive write access. */
		filemon_lock_write();

		/* Remove from the in-use list. */
		TAILQ_REMOVE(&filemons_inuse, filemon, link);

		filemon->fp = NULL;
		filemon->pid = -1;

		/* Add to the free list. */
		TAILQ_INSERT_TAIL(&filemons_free, filemon, link);

		/* Give up write access. */
		filemon_unlock_write();

		if (fp != NULL)
			fdrop(fp, curthread);
	}
}

#if __FreeBSD_version < 900041
#define FGET_WRITE(a1, a2, a3) fget_write((a1), (a2), (a3))
#else
#define FGET_WRITE(a1, a2, a3) fget_write((a1), (a2), CAP_WRITE | CAP_SEEK, (a3))
#endif

static int
filemon_ioctl(struct cdev *dev, u_long cmd, caddr_t data, int flag __unused,
    struct thread *td)
{
	int error = 0;
	struct filemon *filemon;

	devfs_get_cdevpriv((void **) &filemon);

	switch (cmd) {
	/* Set the output file descriptor. */
	case FILEMON_SET_FD:
		if ((error = FGET_WRITE(td, *(int *)data, &filemon->fp)) == 0)
			/* Write the file header. */
			filemon_comment(filemon);
		break;

	/* Set the monitored process ID. */
	case FILEMON_SET_PID:
		filemon->pid = *((pid_t *)data);
		break;

	default:
		error = EINVAL;
		break;
	}

	return (error);
}

static int
filemon_open(struct cdev *dev, int oflags __unused, int devtype __unused,
    struct thread *td __unused)
{
	struct filemon *filemon;

	/* Get exclusive write access. */
	filemon_lock_write();

	if ((filemon = TAILQ_FIRST(&filemons_free)) != NULL)
		TAILQ_REMOVE(&filemons_free, filemon, link);

	/* Give up write access. */
	filemon_unlock_write();

	if (filemon == NULL) {
		filemon = malloc(sizeof(struct filemon), M_FILEMON,
		    M_WAITOK | M_ZERO);

		filemon->fp = NULL;

		mtx_init(&filemon->mtx, "filemon", "filemon", MTX_DEF);
		cv_init(&filemon->cv, "filemon");
	}

	filemon->pid = curproc->p_pid;

	devfs_set_cdevpriv(filemon, filemon_dtr);

	/* Get exclusive write access. */
	filemon_lock_write();

	/* Add to the in-use list. */
	TAILQ_INSERT_TAIL(&filemons_inuse, filemon, link);

	/* Give up write access. */
	filemon_unlock_write();

	return (0);
}

static int
filemon_close(struct cdev *dev __unused, int flag __unused, int fmt __unused,
    struct thread *td __unused)
{

	return (0);
}

static void
filemon_load(void *dummy __unused)
{
	mtx_init(&access_mtx, "filemon", "filemon", MTX_DEF);
	cv_init(&access_cv, "filemon");

	/* Install the syscall wrappers. */
	filemon_wrapper_install();

	filemon_dev = make_dev(&filemon_cdevsw, 0, UID_ROOT, GID_WHEEL, 0666,
	    "filemon");
}

static int
filemon_unload(void)
{
 	struct filemon *filemon;
	int error = 0;

	/* Get exclusive write access. */
	filemon_lock_write();

	if (TAILQ_FIRST(&filemons_inuse) != NULL)
		error = EBUSY;
	else {
		destroy_dev(filemon_dev);

		/* Deinstall the syscall wrappers. */
		filemon_wrapper_deinstall();
	}

	/* Give up write access. */
	filemon_unlock_write();

	if (error == 0) {
		/* free() filemon structs free list. */
		filemon_lock_write();
		while ((filemon = TAILQ_FIRST(&filemons_free)) != NULL) {
			TAILQ_REMOVE(&filemons_free, filemon, link);
			mtx_destroy(&filemon->mtx);
			cv_destroy(&filemon->cv);
			free(filemon, M_FILEMON);
		}
		filemon_unlock_write();

		mtx_destroy(&access_mtx);
		cv_destroy(&access_cv);
	}

	return (error);
}

static int
filemon_modevent(module_t mod __unused, int type, void *data)
{
	int error = 0;

	switch (type) {
	case MOD_LOAD:
		filemon_load(data);
		break;

	case MOD_UNLOAD:
		error = filemon_unload();
		break;

	case MOD_SHUTDOWN:
		break;

	default:
		error = EOPNOTSUPP;
		break;

	}

	return (error);
}

DEV_MODULE(filemon, filemon_modevent, NULL);
MODULE_VERSION(filemon, 1);