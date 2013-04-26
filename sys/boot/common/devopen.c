
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

int
devopen(struct open_file *f, const char *fname, const char **file) 
{
	struct devdesc *dev;
	int result;

	result = archsw.arch_getdev((void **)&dev, fname, file);
	if (result)
		return (result);

	/* point to device-specific data so that device open can use it */
	f->f_devdata = dev;
	result = dev->d_dev->dv_open(f, dev);
	if (result != 0) {
		f->f_devdata = NULL;
		free(dev);
		return (result);
	}

	/* reference the devsw entry from the open_file structure */
	f->f_dev = dev->d_dev;
	return (0);
}

int
devclose(struct open_file *f)
{

	if (f->f_devdata != NULL) {
		free(f->f_devdata);
	}
	return (0);
}