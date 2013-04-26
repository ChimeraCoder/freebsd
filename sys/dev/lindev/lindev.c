
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

/*
 * "lindev" is supposed to be a collection of linux-specific devices
 * that we also support, just not by default.
 * While currently there is only "/dev/full", we are planning to see
 * more in the future.
 * This file is only the container to load/unload all supported devices;
 * the implementation of each should go into its own file.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/module.h>

#include <dev/lindev/lindev.h>

/* ARGSUSED */
static int
lindev_modevent(module_t mod, int type, void *data)
{
	int error;

	switch(type) {
	case MOD_LOAD:
		error = lindev_modevent_full(mod, type, data);
		break;

	case MOD_UNLOAD:
		error = lindev_modevent_full(mod, type, data);
		break;

	case MOD_SHUTDOWN:
		error = lindev_modevent_full(mod, type, data);
		break;

	default:
		return (EOPNOTSUPP);
	}

	return (error);
}

DEV_MODULE(lindev, lindev_modevent, NULL);
MODULE_VERSION(lindev, 1);