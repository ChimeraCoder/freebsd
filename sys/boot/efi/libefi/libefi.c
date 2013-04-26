
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

#include <efi.h>
#include <efilib.h>
#include <stand.h>

EFI_HANDLE		IH;
EFI_SYSTEM_TABLE	*ST;
EFI_BOOT_SERVICES	*BS;
EFI_RUNTIME_SERVICES	*RS;

static EFI_PHYSICAL_ADDRESS heap;
static UINTN heapsize;

static CHAR16 *
arg_skipsep(CHAR16 *argp)
{

	while (*argp == ' ' || *argp == '\t')
		argp++;
	return (argp);
}

static CHAR16 *
arg_skipword(CHAR16 *argp)
{

	while (*argp && *argp != ' ' && *argp != '\t')
		argp++;
	return (argp);
}

void *
efi_get_table(EFI_GUID *tbl)
{
	EFI_GUID *id;
	int i;

	for (i = 0; i < ST->NumberOfTableEntries; i++) {
		id = &ST->ConfigurationTable[i].VendorGuid;
		if (!memcmp(id, tbl, sizeof(EFI_GUID)))
			return (ST->ConfigurationTable[i].VendorTable);
	}
	return (NULL);
}

void exit(EFI_STATUS exit_code)
{

	BS->FreePages(heap, EFI_SIZE_TO_PAGES(heapsize));
	BS->Exit(IH, exit_code, 0, NULL);
}

void
efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *system_table)
{
	static EFI_GUID image_protocol = LOADED_IMAGE_PROTOCOL;
	EFI_LOADED_IMAGE *img;
	CHAR16 *argp, *args, **argv;
	EFI_STATUS status;
	int argc, addprog;

	IH = image_handle;
	ST = system_table;
	BS = ST->BootServices;
	RS = ST->RuntimeServices;

	heapsize = 2 * 1024 * 1024;
	status = BS->AllocatePages(AllocateAnyPages, EfiLoaderData,
	    EFI_SIZE_TO_PAGES(heapsize), &heap);
	if (status != EFI_SUCCESS)
		BS->Exit(IH, status, 0, NULL);

	setheap((void *)(uintptr_t)heap, (void *)(uintptr_t)(heap + heapsize));

	/* Use exit() from here on... */

	status = BS->HandleProtocol(IH, &image_protocol, (VOID**)&img);
	if (status != EFI_SUCCESS)
		exit(status);

	/*
	 * Pre-process the (optional) load options. If the option string
	 * is given as an ASCII string, we use a poor man's ASCII to
	 * Unicode-16 translation. The size of the option string as given
	 * to us includes the terminating null character. We assume the
	 * string is an ASCII string if strlen() plus the terminating
	 * '\0' is less than LoadOptionsSize. Even if all Unicode-16
	 * characters have the upper 8 bits non-zero, the terminating
	 * null character will cause a one-off.
	 * If the string is already in Unicode-16, we make a copy so that
	 * we know we can always modify the string.
	 */
	if (img->LoadOptionsSize > 0 && img->LoadOptions != NULL) {
		if (img->LoadOptionsSize == strlen(img->LoadOptions) + 1) {
			args = malloc(img->LoadOptionsSize << 1);
			for (argc = 0; argc < img->LoadOptionsSize; argc++)
				args[argc] = ((char*)img->LoadOptions)[argc];
		} else {
			args = malloc(img->LoadOptionsSize);
			memcpy(args, img->LoadOptions, img->LoadOptionsSize);
		}
	} else
		args = NULL;

	/*
	 * Use a quick and dirty algorithm to build the argv vector. We
	 * first count the number of words. Then, after allocating the
	 * vector, we split the string up. We don't deal with quotes or
	 * other more advanced shell features.
	 * The EFI shell will pass the name of the image as the first
	 * word in the argument list. This does not happen if we're
	 * loaded by the boot manager. This is not so easy to figure
	 * out though. The ParentHandle is not always NULL, because
	 * there can be a function (=image) that will perform the task
	 * for the boot manager.
	 */
	/* Part 1: Figure out if we need to add our program name. */
	addprog = (args == NULL || img->ParentHandle == NULL ||
	    img->FilePath == NULL) ? 1 : 0;
	if (!addprog) {
		addprog =
		    (DevicePathType(img->FilePath) != MEDIA_DEVICE_PATH ||
		     DevicePathSubType(img->FilePath) != MEDIA_FILEPATH_DP ||
		     DevicePathNodeLength(img->FilePath) <=
			sizeof(FILEPATH_DEVICE_PATH)) ? 1 : 0;
		if (!addprog) {
			/* XXX todo. */
		}
	}
	/* Part 2: count words. */
	argc = (addprog) ? 1 : 0;
	argp = args;
	while (argp != NULL && *argp != 0) {
		argp = arg_skipsep(argp);
		if (*argp == 0)
			break;
		argc++;
		argp = arg_skipword(argp);
	}
	/* Part 3: build vector. */
	argv = malloc((argc + 1) * sizeof(CHAR16*));
	argc = 0;
	if (addprog)
		argv[argc++] = L"loader.efi";
	argp = args;
	while (argp != NULL && *argp != 0) {
		argp = arg_skipsep(argp);
		if (*argp == 0)
			break;
		argv[argc++] = argp;
		argp = arg_skipword(argp);
		/* Terminate the words. */
		if (*argp != 0)
			*argp++ = 0;
	}
	argv[argc] = NULL;

	status = main(argc, argv);
	exit(status);
}