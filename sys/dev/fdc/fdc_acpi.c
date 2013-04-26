
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
#include <sys/kernel.h>
#include <sys/bio.h>
#include <sys/bus.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <sys/proc.h>

#include <contrib/dev/acpica/include/acpi.h>

#include <dev/acpica/acpivar.h>
#include <dev/fdc/fdcvar.h>

static int		fdc_acpi_probe(device_t dev);
static int		fdc_acpi_attach(device_t dev);
static int		fdc_acpi_probe_children(device_t bus, device_t dev,
			    void *fde);
static ACPI_STATUS	fdc_acpi_probe_child(ACPI_HANDLE h, device_t *dev,
			    int level, void *arg);

/* Maximum number of child devices of a controller (4 floppy + 1 tape.) */
#define ACPI_FDC_MAXDEVS	5

/* Standard size of buffer returned by the _FDE method. */
#define ACPI_FDC_FDE_LEN	(ACPI_FDC_MAXDEVS * sizeof(uint32_t))

/*
 * Parameters for the tape drive (5th device).  Some BIOS authors use this
 * for all drives, not just the tape drive (e.g., ASUS K8V).  This isn't
 * grossly incompatible with the spec since it says the first four devices
 * are simple booleans.
 */
#define ACPI_FD_UNKNOWN		0
#define ACPI_FD_PRESENT		1
#define ACPI_FD_NEVER_PRESENT	2

/* Temporary buf length for evaluating _FDE and _FDI. */
#define ACPI_FDC_BUFLEN		1024

/* Context for walking FDC child devices. */
struct fdc_walk_ctx {
	uint32_t	fd_present[ACPI_FDC_MAXDEVS];
	int		index;
	device_t	acpi_dev;
	device_t	dev;
};

static int
fdc_acpi_probe(device_t dev)
{
	device_t bus;
	static char *fdc_ids[] = { "PNP0700", "PNP0701", NULL };

	bus = device_get_parent(dev);
	if (ACPI_ID_PROBE(bus, dev, fdc_ids) == NULL)
		return (ENXIO);

	if (ACPI_SUCCESS(ACPI_EVALUATE_OBJECT(bus, dev, "_FDE", NULL, NULL)))
		device_set_desc(dev, "floppy drive controller (FDE)");
	else
		device_set_desc(dev, "floppy drive controller");
	return (0);
}

static int
fdc_acpi_attach(device_t dev)
{
	struct fdc_data *sc;
	ACPI_BUFFER buf;
	ACPI_OBJECT *obj;
	device_t bus;
	int error;

	/* Get our softc and use the same accessor as ISA. */
	sc = device_get_softc(dev);
	sc->fdc_dev = dev;

	/* Initialize variables and get a temporary buffer for _FDE. */
	error = ENXIO;
	buf.Length = ACPI_FDC_BUFLEN;
	buf.Pointer = malloc(buf.Length, M_TEMP, M_NOWAIT | M_ZERO);
	if (buf.Pointer == NULL)
		goto out;

	/* Allocate resources the same as the ISA attachment. */
	error = fdc_isa_alloc_resources(dev, sc);
	if (error != 0)
		goto out;

	/* Call common attach code in fdc(4) first. */
	error = fdc_attach(dev);
	if (error != 0)
		goto out;

	/*
	 * Enumerate _FDE, which lists floppy drives that are present.  If
	 * this fails, fall back to the ISA hints-based probe method.
	 */
	bus = device_get_parent(dev);
	if (ACPI_FAILURE(ACPI_EVALUATE_OBJECT(bus, dev, "_FDE", NULL, &buf))) {
		error = fdc_hints_probe(dev);
		goto out;
	}

	/* Add fd child devices as specified. */
	obj = buf.Pointer;
	error = fdc_acpi_probe_children(bus, dev, obj->Buffer.Pointer);

out:
	if (buf.Pointer)
		free(buf.Pointer, M_TEMP);
	if (error != 0)
		fdc_release_resources(sc);

	return (error);
}

static int
fdc_acpi_probe_children(device_t bus, device_t dev, void *fde)
{
	struct fdc_walk_ctx *ctx;
	devclass_t fd_dc;
	int i;

	/* Setup the context and walk all child devices. */
	ctx = malloc(sizeof(struct fdc_walk_ctx), M_TEMP, M_NOWAIT);
	if (ctx == NULL) {
		device_printf(dev, "no memory for walking children\n");
		return (ENOMEM);
	}
	bcopy(fde, ctx->fd_present, sizeof(ctx->fd_present));
	ctx->index = 0;
	ctx->dev = dev;
	ctx->acpi_dev = bus;
	ACPI_SCAN_CHILDREN(ctx->acpi_dev, dev, 1, fdc_acpi_probe_child,
	    ctx);

	/* Add any devices not represented by an AML Device handle/node. */
	fd_dc = devclass_find("fd");
	for (i = 0; i < ACPI_FDC_MAXDEVS; i++)
		if (ctx->fd_present[i] == ACPI_FD_PRESENT &&
		    devclass_get_device(fd_dc, i) == NULL) {
			if (fdc_add_child(dev, "fd", i) == NULL)
				device_printf(dev, "fd add failed\n");
		}
	free(ctx, M_TEMP);

	/* Attach any children found during the probe. */
	return (bus_generic_attach(dev));
}

static ACPI_STATUS
fdc_acpi_probe_child(ACPI_HANDLE h, device_t *dev, int level, void *arg)
{
	struct fdc_walk_ctx *ctx;
	device_t child, old_child;
	ACPI_BUFFER buf;
	ACPI_OBJECT *pkg, *obj;
	ACPI_STATUS status;

	ctx = (struct fdc_walk_ctx *)arg;
	buf.Pointer = NULL;

	/*
	 * The first four ints are booleans that indicate whether fd0-3 are
	 * present or not.  The last is for a tape device, which we don't
	 * bother supporting for now.
	 */
	if (ctx->index > 3)
		return (AE_OK);

	/* This device is not present, move on to the next. */
	if (ctx->fd_present[ctx->index] != ACPI_FD_PRESENT)
		goto out;

	/* Create a device for the child with the given index. */
	child = fdc_add_child(ctx->dev, "fd", ctx->index);
	if (child == NULL)
		goto out;
	old_child = *dev;
	*dev = child;

	/* Get temporary buffer for _FDI probe. */
	buf.Length = ACPI_FDC_BUFLEN;
	buf.Pointer = malloc(buf.Length, M_TEMP, M_NOWAIT | M_ZERO);
	if (buf.Pointer == NULL)
		goto out;

	/*
	 * Evaluate _FDI to get drive type to pass to the child.  We use the
	 * old child here since it has a valid ACPI_HANDLE since it is a
	 * child of acpi.  A better way to implement this would be to make fdc
	 * support the ACPI handle ivar for its children.
	 */
	status = ACPI_EVALUATE_OBJECT(ctx->acpi_dev, old_child, "_FDI", NULL,
	    &buf);
	if (ACPI_FAILURE(status)) {
		if (status != AE_NOT_FOUND)
			device_printf(ctx->dev, "_FDI failed - %#x\n", status);
		goto out;
	}
	pkg = (ACPI_OBJECT *)buf.Pointer;
	if (!ACPI_PKG_VALID(pkg, 16)) {
		device_printf(ctx->dev, "invalid _FDI package\n");
		goto out;
	}
	obj = &pkg->Package.Elements[1];
	if (obj == NULL || obj->Type != ACPI_TYPE_INTEGER) {
		device_printf(ctx->dev, "invalid type object in _FDI\n");
		goto out;
	}
	fdc_set_fdtype(child, obj->Integer.Value);

out:
	ctx->index++;
	if (buf.Pointer)
		free(buf.Pointer, M_TEMP);
	return (AE_OK);
}

static device_method_t fdc_acpi_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		fdc_acpi_probe),
	DEVMETHOD(device_attach,	fdc_acpi_attach),
	DEVMETHOD(device_detach,	fdc_detach),

	/* Bus interface */
	DEVMETHOD(bus_print_child,	fdc_print_child),
	DEVMETHOD(bus_read_ivar,	fdc_read_ivar),
	DEVMETHOD(bus_write_ivar,	fdc_write_ivar),

	DEVMETHOD_END
};

static driver_t fdc_acpi_driver = {
	"fdc",
	fdc_acpi_methods,
	sizeof(struct fdc_data)
};

DRIVER_MODULE(fdc, acpi, fdc_acpi_driver, fdc_devclass, 0, 0);