
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
 * Copyright (c) 2013 by Delphix. All rights reserved.
 */

/*
 * The 'missing' vdev is a special vdev type used only during import.  It
 * signifies a placeholder in the root vdev for some vdev that we know is
 * missing.  We pass it down to the kernel to allow the rest of the
 * configuration to parsed and an attempt made to open all available devices.
 * Because its GUID is always 0, we know that the guid sum will mismatch and we
 * won't be able to open the pool anyway.
 */

#include <sys/zfs_context.h>
#include <sys/spa.h>
#include <sys/vdev_impl.h>
#include <sys/fs/zfs.h>
#include <sys/zio.h>

/* ARGSUSED */
static int
vdev_missing_open(vdev_t *vd, uint64_t *psize, uint64_t *max_psize,
    uint64_t *ashift)
{
	/*
	 * Really this should just fail.  But then the root vdev will be in the
	 * faulted state with VDEV_AUX_NO_REPLICAS, when what we really want is
	 * VDEV_AUX_BAD_GUID_SUM.  So we pretend to succeed, knowing that we
	 * will fail the GUID sum check before ever trying to open the pool.
	 */
	*psize = 0;
	*max_psize = 0;
	*ashift = 0;
	return (0);
}

/* ARGSUSED */
static void
vdev_missing_close(vdev_t *vd)
{
}

/* ARGSUSED */
static int
vdev_missing_io_start(zio_t *zio)
{
	zio->io_error = SET_ERROR(ENOTSUP);
	return (ZIO_PIPELINE_CONTINUE);
}

/* ARGSUSED */
static void
vdev_missing_io_done(zio_t *zio)
{
}

vdev_ops_t vdev_missing_ops = {
	vdev_missing_open,
	vdev_missing_close,
	vdev_default_asize,
	vdev_missing_io_start,
	vdev_missing_io_done,
	NULL,
	NULL,
	NULL,
	VDEV_TYPE_MISSING,	/* name of this vdev type */
	B_TRUE			/* leaf vdev */
};

vdev_ops_t vdev_hole_ops = {
	vdev_missing_open,
	vdev_missing_close,
	vdev_default_asize,
	vdev_missing_io_start,
	vdev_missing_io_done,
	NULL,
	NULL,
	NULL,
	VDEV_TYPE_HOLE,		/* name of this vdev type */
	B_TRUE			/* leaf vdev */
};