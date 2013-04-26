
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

#include <config.h>

#include "roken.h"

/*
 * Write datablob to a filename, don't care about errors.
 */

ROKEN_LIB_FUNCTION void ROKEN_LIB_CALL
rk_dumpdata (const char *filename, const void *buf, size_t size)
{
    int fd;

    fd = open(filename, O_WRONLY|O_TRUNC|O_CREAT, 0640);
    if (fd < 0)
	return;
    net_write(fd, buf, size);
    close(fd);
}

/*
 * Read all data from a filename, care about errors.
 */

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
rk_undumpdata(const char *filename, void **buf, size_t *size)
{
    struct stat sb;
    int fd, ret;
    ssize_t sret;

    *buf = NULL;

    fd = open(filename, O_RDONLY, 0);
    if (fd < 0)
	return errno;
    if (fstat(fd, &sb) != 0){
	ret = errno;
	goto out;
    }
    *buf = malloc(sb.st_size);
    if (*buf == NULL) {
	ret = ENOMEM;
	goto out;
    }
    *size = sb.st_size;

    sret = net_read(fd, *buf, *size);
    if (sret < 0)
	ret = errno;
    else if (sret != (ssize_t)*size) {
	ret = EINVAL;
	free(*buf);
	*buf = NULL;
    } else
	ret = 0;

 out:
    close(fd);
    return ret;
}