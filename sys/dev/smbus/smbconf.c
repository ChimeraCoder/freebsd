
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
#include <sys/lock.h>
#include <sys/module.h>
#include <sys/mutex.h>
#include <sys/bus.h>

#include <dev/smbus/smbconf.h>
#include <dev/smbus/smbus.h>
#include "smbus_if.h"

/*
 * smbus_intr()
 */
void
smbus_intr(device_t bus, u_char devaddr, char low, char high, int error)
{
	struct smbus_softc *sc = device_get_softc(bus);

	/* call owner's intr routine */
	mtx_lock(&sc->lock);
	if (sc->owner)
		SMBUS_INTR(sc->owner, devaddr, low, high, error);
	mtx_unlock(&sc->lock);
}

/*
 * smbus_error()
 *
 * Converts an smbus error to a unix error.
 */
int
smbus_error(int smb_error)
{
	int error = 0;

	if (smb_error == SMB_ENOERR)
		return (0);
	
	if (smb_error & (SMB_ENOTSUPP))
		error = ENODEV;
	else if (smb_error & (SMB_ENOACK))
		error = ENXIO;
	else if (smb_error & (SMB_ETIMEOUT))
		error = EWOULDBLOCK;
	else if (smb_error & (SMB_EBUSY))
		error = EBUSY;
	else if (smb_error & (SMB_EABORT | SMB_EBUSERR | SMB_ECOLLI))
		error = EIO;
	else
		error = EINVAL;

	return (error);
}

static int
smbus_poll(struct smbus_softc *sc, int how)
{
	int error;

	switch (how) {
	case SMB_WAIT | SMB_INTR:		
		error = msleep(sc, &sc->lock, SMBPRI|PCATCH, "smbreq", 0);
		break;

	case SMB_WAIT | SMB_NOINTR:
		error = msleep(sc, &sc->lock, SMBPRI, "smbreq", 0);
		break;

	default:
		error = EWOULDBLOCK;
		break;
	}

	return (error);
}

/*
 * smbus_request_bus()
 *
 * Allocate the device to perform transfers.
 *
 * how	: SMB_WAIT or SMB_DONTWAIT
 */
int
smbus_request_bus(device_t bus, device_t dev, int how)
{
	struct smbus_softc *sc = device_get_softc(bus);
	device_t parent;
	int error;

	/* first, ask the underlying layers if the request is ok */
	parent = device_get_parent(bus);
	mtx_lock(&sc->lock);
	do {
		mtx_unlock(&sc->lock);
		error = SMBUS_CALLBACK(parent, SMB_REQUEST_BUS, &how);
		mtx_lock(&sc->lock);

		if (error)
			error = smbus_poll(sc, how);
	} while (error == EWOULDBLOCK);

	while (error == 0) {
		if (sc->owner && sc->owner != dev)
			error = smbus_poll(sc, how);
		else {
			sc->owner = dev;
			break;
		}

		/* free any allocated resource */
		if (error) {
			mtx_unlock(&sc->lock);
			SMBUS_CALLBACK(parent, SMB_RELEASE_BUS, &how);
			return (error);
		}
	}
	mtx_unlock(&sc->lock);

	return (error);
}

/*
 * smbus_release_bus()
 *
 * Release the device allocated with smbus_request_dev()
 */
int
smbus_release_bus(device_t bus, device_t dev)
{
	struct smbus_softc *sc = device_get_softc(bus);
	int error;

	/* first, ask the underlying layers if the release is ok */
	error = SMBUS_CALLBACK(device_get_parent(bus), SMB_RELEASE_BUS, NULL);

	if (error)
		return (error);

	mtx_lock(&sc->lock);
	if (sc->owner == dev) {
		sc->owner = NULL;

		/* wakeup waiting processes */
		wakeup(sc);
	} else
		error = EACCES;
	mtx_unlock(&sc->lock);

	return (error);
}