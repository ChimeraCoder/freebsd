
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
#include <sys/disklabel.h>
#include "bootstrap.h"

#include <efi.h>
#include <efilib.h>

static int i386_parsedev(struct devdesc **, const char *, const char **);

/* 
 * Point (dev) at an allocated device specifier for the device matching the
 * path in (devspec). If it contains an explicit device specification,
 * use that.  If not, use the default device.
 */
int
i386_getdev(void **vdev, const char *devspec, const char **path)
{
	struct devdesc **dev = (struct devdesc **)vdev;
	int rv;

	/*
	 * If it looks like this is just a path and no device, then
	 * use the current device instead.
	 */
	if (devspec == NULL || *devspec == '/' || !strchr(devspec, ':')) {
		rv = i386_parsedev(dev, getenv("currdev"), NULL);
		if (rv == 0 && path != NULL)
			*path = devspec;
		return (rv);
	}

	/* Parse the device name off the beginning of the devspec. */
	return (i386_parsedev(dev, devspec, path));
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
 * fs<unit>:
 */
static int
i386_parsedev(struct devdesc **dev, const char *devspec, const char **path)
{
	struct devdesc *idev;
	struct devsw *dv;
	char *cp;
	const char *np;
	int i, err;

	/* minimum length check */
	if (strlen(devspec) < 2)
		return (EINVAL);

	/* look for a device that matches */
	for (i = 0; devsw[i] != NULL; i++) {
		dv = devsw[i];
		if (!strncmp(devspec, dv->dv_name, strlen(dv->dv_name)))
			break;
	}
	if (devsw[i] == NULL)
		return (ENOENT);

	idev = malloc(sizeof(struct devdesc));
	if (idev == NULL)
		return (ENOMEM);

	idev->d_dev = dv;
	idev->d_type = dv->dv_type;
	idev->d_unit = -1;

	err = 0;
	np = devspec + strlen(dv->dv_name);
	if (*np != '\0' && *np != ':') {
		idev->d_unit = strtol(np, &cp, 0);
		if (cp == np) {
			idev->d_unit = -1;
			free(idev);
			return (EUNIT);
		}
	}
	if (*cp != '\0' && *cp != ':') {
		free(idev);
		return (EINVAL);
	}

	if (path != NULL)
		*path = (*cp == 0) ? cp : cp + 1;
	if (dev != NULL)
		*dev = idev;
	else
		free(idev);
	return (0);
}

char *
i386_fmtdev(void *vdev)
{
	struct devdesc *dev = (struct devdesc *)vdev;
	static char buf[32];	/* XXX device length constant? */

	switch(dev->d_type) {
	case DEVT_NONE:
		strcpy(buf, "(no device)");
		break;

	default:
		sprintf(buf, "%s%d:", dev->d_dev->dv_name, dev->d_unit);
		break;
	}

	return(buf);
}

/*
 * Set currdev to suit the value being supplied in (value)
 */
int
i386_setcurrdev(struct env_var *ev, int flags, const void *value)
{
	struct devdesc *ncurr;
	int rv;

	rv = i386_parsedev(&ncurr, value, NULL);
	if (rv != 0)
		return(rv);

	free(ncurr);
	env_setenv(ev->ev_name, flags | EV_NOHOOK, value, NULL, NULL);
	return (0);
}