
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
#include <sys/conf.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/ioccom.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/module.h>

#include <dev/ofw/openfirm.h>
#include <dev/ofw/openpromio.h>

/*
 * This provides a Solaris compatible character device interface to
 * Open Firmware.  It exists entirely for compatibility with software
 * like X11, and only the features that are actually needed for that
 * are implemented.  The interface sucks too much to actually use,
 * new code should use the /dev/openfirm device.
 */

static d_open_t openprom_open;
static d_close_t openprom_close;
static d_ioctl_t openprom_ioctl;

static int openprom_modevent(module_t mode, int type, void *data);
static int openprom_node_valid(phandle_t node);
static int openprom_node_search(phandle_t root, phandle_t node);

static struct cdevsw openprom_cdevsw = {
	.d_version =	D_VERSION,
	.d_flags =	D_NEEDGIANT,
	.d_open =	openprom_open,
	.d_close =	openprom_close,
	.d_ioctl =	openprom_ioctl,
	.d_name =	"openprom",
};

static int openprom_is_open;
static struct cdev *openprom_dev;
static phandle_t openprom_node;

static int
openprom_open(struct cdev *dev, int oflags, int devtype, struct thread *td)
{

	if (openprom_is_open != 0)
		return (EBUSY);
	openprom_is_open = 1;
	return (0);
}

static int
openprom_close(struct cdev *dev, int fflag, int devtype, struct thread *td)
{

	openprom_is_open = 0;
	return (0);
}

static int
openprom_ioctl(struct cdev *dev, u_long cmd, caddr_t data, int flags,
    struct thread *td)
{
	struct openpromio *oprom;
	phandle_t node;
	uint32_t len;
	size_t done;
	int proplen;
	char *prop;
	char *buf;
	int error;

	if ((flags & FREAD) == 0)
		return (EPERM);

	prop = buf = NULL;
	error = 0;
	switch (cmd) {
	case OPROMCHILD:
	case OPROMNEXT:
		if (data == NULL || *(void **)data == NULL)
			return (EINVAL);
		oprom = *(void **)data;
		error = copyin(&oprom->oprom_size, &len, sizeof(len));
		if (error != 0)
			break;
		if (len != sizeof(node)) {
			error = EINVAL;
			break;
		}
		error = copyin(&oprom->oprom_array, &node, sizeof(node));
		if (error != 0)
			break;
		error = openprom_node_valid(node);
		if (error != 0)
			break;
		switch (cmd) {
		case OPROMCHILD:
			node = OF_child(node);
			break;
		case OPROMNEXT:
			node = OF_peer(node);
			break;
		}
		error = copyout(&node, &oprom->oprom_array, sizeof(node));
		if (error != 0)
			break;
		openprom_node = node;
		break;
	case OPROMGETPROP:
	case OPROMNXTPROP:
		if (data == NULL || *(void **)data == NULL)
			return (EINVAL);
		oprom = *(void **)data;
		error = copyin(&oprom->oprom_size, &len, sizeof(len));
		if (error != 0)
			break;
		if (len > OPROMMAXPARAM) {
			error = EINVAL;
			break;
		}
		prop = malloc(len, M_TEMP, M_WAITOK | M_ZERO);
		if (prop == NULL) {
			error = ENOMEM;
			break;
		}
		error = copyinstr(&oprom->oprom_array, prop, len, &done);
		if (error != 0)
			break;
		buf = malloc(OPROMMAXPARAM, M_TEMP, M_WAITOK | M_ZERO);
		if (buf == NULL) {
			error = ENOMEM;
			break;
		}
		node = openprom_node;
		switch (cmd) {
		case OPROMGETPROP:
			proplen = OF_getproplen(node, prop);
			if (proplen > OPROMMAXPARAM) {
				error = EINVAL;
				break;
			}
			error = OF_getprop(node, prop, buf, proplen);
			break;
		case OPROMNXTPROP:
			error = OF_nextprop(node, prop, buf, OPROMMAXPARAM);
			proplen = strlen(buf);
			break;
		}
		if (error != -1) {
			error = copyout(&proplen, &oprom->oprom_size,
			    sizeof(proplen));
			if (error == 0)
				error = copyout(buf, &oprom->oprom_array,
				    proplen + 1);
		} else
			error = EINVAL;
		break;
	default:
		error = ENOIOCTL;
		break;
	}

	if (prop != NULL)
		free(prop, M_TEMP);
	if (buf != NULL)
		free(buf, M_TEMP);

	return (error);
}

static int
openprom_node_valid(phandle_t node)
{

	if (node == 0)
		return (0);
	return (openprom_node_search(OF_peer(0), node));
}

static int
openprom_node_search(phandle_t root, phandle_t node)
{
	phandle_t child;

	if (root == node)
		return (0);
	for (child = OF_child(root); child != 0 && child != -1;
	    child = OF_peer(child))
		if (openprom_node_search(child, node) == 0)
			return (0);
	return (EINVAL);
}

static int
openprom_modevent(module_t mode, int type, void *data)
{

	switch (type) {
	case MOD_LOAD:
		openprom_dev = make_dev(&openprom_cdevsw, 0, UID_ROOT,
		    GID_WHEEL, 0600, "openprom");
		return (0);
	case MOD_UNLOAD:
		destroy_dev(openprom_dev);
		return (0);
	default:
		return (EOPNOTSUPP);
	}
}

DEV_MODULE(openprom, openprom_modevent, NULL);