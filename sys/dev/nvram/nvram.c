
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
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/conf.h>
#include <sys/fcntl.h>
#include <sys/lock.h>
#include <sys/proc.h>
#include <sys/sx.h>
#include <sys/uio.h>
#include <sys/module.h>

#include <isa/rtc.h>

/*
 * Linux-style /dev/nvram driver
 *
 * cmos ram starts at bytes 14 through 128, for a total of 114 bytes.
 * The driver exposes byte 14 as file offset 0.
 *
 * Offsets 2 through 31 are checksummed at offset 32, 33.
 * In order to avoid the possibility of making the machine unbootable at the
 * bios level (press F1 to continue!), we refuse to allow writes if we do
 * not see a pre-existing valid checksum.  If the existing sum is invalid,
 * then presumably we do not know how to make a sum that the bios will accept.
 */

#define NVRAM_FIRST	RTC_DIAG	/* 14 */
#define NVRAM_LAST	128

#define CKSUM_FIRST	2
#define CKSUM_LAST	31
#define CKSUM_MSB	32
#define CKSUM_LSB	33

static d_open_t		nvram_open;
static d_read_t		nvram_read;
static d_write_t	nvram_write;

static struct cdev *nvram_dev;
static struct sx nvram_lock;

static struct cdevsw nvram_cdevsw = {
	.d_version =	D_VERSION,
	.d_open =	nvram_open,
	.d_read =	nvram_read,
	.d_write =	nvram_write,
	.d_name =	"nvram",
};

static int
nvram_open(struct cdev *dev __unused, int flags, int fmt __unused,
    struct thread *td)
{
	int error = 0;

	if (flags & FWRITE)
		error = securelevel_gt(td->td_ucred, 0);

	return (error);
}

static int
nvram_read(struct cdev *dev, struct uio *uio, int flags)
{
	int nv_off;
	u_char v;
	int error = 0;

	while (uio->uio_resid > 0 && error == 0) {
		nv_off = uio->uio_offset + NVRAM_FIRST;
		if (nv_off < NVRAM_FIRST || nv_off >= NVRAM_LAST)
			return (0);	/* Signal EOF */
		/* Single byte at a time */
		v = rtcin(nv_off);
		error = uiomove(&v, 1, uio);
	}
	return (error);

}

static int
nvram_write(struct cdev *dev, struct uio *uio, int flags)
{
	int nv_off;
	u_char v;
	int error = 0;
	int i;
	uint16_t sum;

	sx_xlock(&nvram_lock);

	/* Assert that we understand the existing checksum first!  */
	sum = rtcin(NVRAM_FIRST + CKSUM_MSB) << 8 |
	      rtcin(NVRAM_FIRST + CKSUM_LSB);
	for (i = CKSUM_FIRST; i <= CKSUM_LAST; i++)
		sum -= rtcin(NVRAM_FIRST + i);
	if (sum != 0) {
		sx_xunlock(&nvram_lock);
		return (EIO);
	}
	/* Bring in user data and write */
	while (uio->uio_resid > 0 && error == 0) {
		nv_off = uio->uio_offset + NVRAM_FIRST;
		if (nv_off < NVRAM_FIRST || nv_off >= NVRAM_LAST) {
			sx_xunlock(&nvram_lock);
			return (0);	/* Signal EOF */
		}
		/* Single byte at a time */
		error = uiomove(&v, 1, uio);
		writertc(nv_off, v);
	}
	/* Recalculate checksum afterwards */
	sum = 0;
	for (i = CKSUM_FIRST; i <= CKSUM_LAST; i++)
		sum += rtcin(NVRAM_FIRST + i);
	writertc(NVRAM_FIRST + CKSUM_MSB, sum >> 8);
	writertc(NVRAM_FIRST + CKSUM_LSB, sum);
	sx_xunlock(&nvram_lock);
	return (error);
}

static int
nvram_modevent(module_t mod __unused, int type, void *data __unused)
{
	switch (type) {
	case MOD_LOAD:
		sx_init(&nvram_lock, "nvram");
		nvram_dev = make_dev(&nvram_cdevsw, 0,
		    UID_ROOT, GID_KMEM, 0640, "nvram");
		break;
	case MOD_UNLOAD:
	case MOD_SHUTDOWN:
		destroy_dev(nvram_dev);
		sx_destroy(&nvram_lock);
		break;
	default:
		return (EOPNOTSUPP);
	}
	return (0);
}
DEV_MODULE(nvram, nvram_modevent, NULL);