
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
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <libgeom.h>
#include <devid.h>

int
devid_str_decode(char *devidstr, ddi_devid_t *retdevid, char **retminor_name)
{

	if (strlcpy(retdevid->devid, devidstr, sizeof(retdevid->devid)) >=
	    sizeof(retdevid->devid)) {
		return (EINVAL);
	}
	*retminor_name = strdup("");
	if (*retminor_name == NULL)
		return (ENOMEM);
	return (0);
}

int
devid_deviceid_to_nmlist(char *search_path, ddi_devid_t devid, char *minor_name,
    devid_nmlist_t **retlist)
{
	char path[MAXPATHLEN];
	char *dst;

	if (g_get_name(devid.devid, path, sizeof(path)) == -1)
		return (errno);
	*retlist = malloc(sizeof(**retlist));
	if (*retlist == NULL)
		return (ENOMEM);
	if (strlcpy((*retlist)[0].devname, path,
	    sizeof((*retlist)[0].devname)) >= sizeof((*retlist)[0].devname)) {
		free(*retlist);
		return (ENAMETOOLONG);
	}
	return (0);
}

void
devid_str_free(char *str)
{

	free(str);
}

void
devid_free(ddi_devid_t devid)
{
	/* Do nothing. */
}

void
devid_free_nmlist(devid_nmlist_t *list)
{

	free(list);
}

int
devid_get(int fd, ddi_devid_t *retdevid)
{

	return (ENOENT);
}

int
devid_get_minor_name(int fd, char **retminor_name)
{

	*retminor_name = strdup("");
	if (*retminor_name == NULL)
		return (ENOMEM);
	return (0);
}

char *
devid_str_encode(ddi_devid_t devid, char *minor_name)
{

	return (strdup(devid.devid));
}