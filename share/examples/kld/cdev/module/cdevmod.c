
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
#include <sys/module.h>
#include <sys/conf.h>

#include "cdev.h"

static struct cdevsw my_devsw = {
	/* version */	.d_version = D_VERSION,
	/* open */	.d_open = mydev_open,
	/* close */	.d_close = mydev_close,
	/* read */	.d_read = mydev_read,
	/* write */	.d_write = mydev_write,
	/* ioctl */	.d_ioctl = mydev_ioctl,
	/* name */	.d_name = "cdev"
};

/* 
 * Used as the variable that is the reference to our device
 * in devfs... we must keep this variable sane until we 
 * call kldunload.
 */
static struct cdev *sdev;

/*
 * This function is called each time the module is loaded or unloaded.
 * Since we are a miscellaneous module, we have to provide whatever
 * code is necessary to patch ourselves into the area we are being
 * loaded to change.
 *
 * The stat information is basically common to all modules, so there
 * is no real issue involved with stat; we will leave it lkm_nullcmd(),
 * since we don't have to do anything about it.
 */

static int
cdev_load(module_t mod, int cmd, void *arg)
{
    int  err = 0;

    switch (cmd) {
    case MOD_LOAD:
	
	/* Do any initialization that you should do with the kernel */
	
	/* if we make it to here, print copyright on console*/
	printf("\nSample Loaded kld character device driver\n");
	printf("Copyright (c) 1998\n");
	printf("Rajesh Vaidheeswarran\n");
	printf("All rights reserved\n");
	sdev = make_dev(&my_devsw, 0, UID_ROOT, GID_WHEEL, 0600, "cdev");
	break;		/* Success*/

    case MOD_UNLOAD:
	printf("Unloaded kld character device driver\n");
	destroy_dev(sdev);
	break;		/* Success*/

    default:	/* we only understand load/unload*/
	err = EOPNOTSUPP;
	break;
    }

    return(err);
}

/* Now declare the module to the system */

DEV_MODULE(cdev, cdev_load, NULL);