
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

#include <stand.h>
#include <string.h>

#include "bootstrap.h"
#include "disk.h"
#include "libuboot.h"

static int uboot_parsedev(struct uboot_devdesc **dev, const char *devspec,
    const char **path);

/*
 * Point (dev) at an allocated device specifier for the device matching the
 * path in (devspec). If it contains an explicit device specification,
 * use that.  If not, use the default device.
 */
int
uboot_getdev(void **vdev, const char *devspec, const char **path)
{
	struct uboot_devdesc **dev = (struct uboot_devdesc **)vdev;
	int rv;

	/*
	 * If it looks like this is just a path and no
	 * device, go with the current device.
	 */
	if ((devspec == NULL) || (devspec[0] == '/') ||
	    (strchr(devspec, ':') == NULL)) {

		if (((rv = uboot_parsedev(dev, getenv("currdev"), NULL)) == 0)
		    && (path != NULL))
		*path = devspec;
		return(rv);
	}

	/*
	 * Try to parse the device name off the beginning of the devspec.
	 */
	return (uboot_parsedev(dev, devspec, path));
}

/*
 * Point (dev) at an allocated device specifier matching the string version
 * at the beginning of (devspec).  Return a pointer to the remaining
 * text in (path).
 *
 * In all cases, the beginning of (devspec) is compared to the names
 * of known devices in the device switch, and then any following text
 * is parsed according to the rules applied to the device type.
 *
 * For disk-type devices, the syntax is:
 *
 * disk<unit>[<partition>]:
 *
 */
static int
uboot_parsedev(struct uboot_devdesc **dev, const char *devspec,
    const char **path)
{
	struct uboot_devdesc *idev;
	struct devsw *dv;
	char *cp;
	const char *np;
	int i, unit, err;

	/* minimum length check */
	if (strlen(devspec) < 2)
		return(EINVAL);

	/* look for a device that matches */
	for (i = 0, dv = NULL; devsw[i] != NULL; i++) {
		if (!strncmp(devspec, devsw[i]->dv_name,
		    strlen(devsw[i]->dv_name))) {
			dv = devsw[i];
			break;
		}
	}
	if (dv == NULL)
		return(ENOENT);
	idev = malloc(sizeof(struct uboot_devdesc));
	err = 0;
	np = (devspec + strlen(dv->dv_name));

	switch(dv->dv_type) {
	case DEVT_NONE:
		break;

#ifdef LOADER_DISK_SUPPORT
	case DEVT_DISK:
		err = disk_parsedev((struct disk_devdesc *)idev, np, path);
		if (err != 0)
			goto fail;
		break;
#endif

	case DEVT_NET:
		unit = 0;

		if (*np && (*np != ':')) {
			/* get unit number if present */
			unit = strtol(np, &cp, 0);
			if (cp == np) {
				err = EUNIT;
				goto fail;
			}
		}
		if (*cp && (*cp != ':')) {
			err = EINVAL;
			goto fail;
		}
		idev->d_unit = unit;

		if (path != NULL)
			*path = (*cp == 0) ? cp : cp + 1;
		break;

	default:
		err = EINVAL;
		goto fail;
	}
	idev->d_dev = dv;
	idev->d_type = dv->dv_type;
	if (dev == NULL) {
		free(idev);
	} else {
		*dev = idev;
	}
	return (0);

fail:
	free(idev);
	return (err);
}


char *
uboot_fmtdev(void *vdev)
{
	struct uboot_devdesc *dev = (struct uboot_devdesc *)vdev;
	static char buf[128];

	switch(dev->d_type) {
	case DEVT_NONE:
		strcpy(buf, "(no device)");
		break;

	case DEVT_DISK:
#ifdef LOADER_DISK_SUPPORT
		return (disk_fmtdev(vdev));
#endif

	case DEVT_NET:
		sprintf(buf, "%s%d:", dev->d_dev->dv_name, dev->d_unit);
		break;
	}
	return(buf);
}

/*
 * Set currdev to suit the value being supplied in (value).
 */
int
uboot_setcurrdev(struct env_var *ev, int flags, const void *value)
{
	struct uboot_devdesc *ncurr;
	int rv;

	if ((rv = uboot_parsedev(&ncurr, value, NULL)) != 0)
		return (rv);
	free(ncurr);
	env_setenv(ev->ev_name, flags | EV_NOHOOK, value, NULL, NULL);
	return (0);
}