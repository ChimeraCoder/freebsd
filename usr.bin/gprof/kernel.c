
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
/* Things which get -E excluded by default. */static char	*excludes[] = { ".mcount", "_mcleanup", NULL };

int
kernel_getnfile(const char *unused __unused, char ***defaultEs)
{
	char *namelist;
	size_t len;
	char *name;

	if (sysctlbyname("kern.function_list", NULL, &len, NULL, 0) == -1)
		err(1, "sysctlbyname: function_list size");
	for (;;) {
		namelist = malloc(len);
		if (namelist == NULL)
			err(1, "malloc");
		if (sysctlbyname("kern.function_list", namelist, &len, NULL,
		   0) == 0)
			break;
		if (errno == ENOMEM)
			free(namelist);
		else
			err(1, "sysctlbyname: function_list");
	}
	nname = 0;
	for (name = namelist; *name != '\0'; name += strlen(name) + 1)
		nname++;
	/* Allocate memory for them, plus a terminating entry. */
	if ((nl = (nltype *)calloc(nname + 1, sizeof(nltype))) == NULL)
		errx(1, "Insufficient memory for symbol table");
	npe = nl;
	for (name = namelist; *name != '\0'; name += strlen(name) + 1) {
		struct kld_sym_lookup ksl;

		ksl.version = sizeof(ksl);
		ksl.symname = name;
		if (kldsym(0, KLDSYM_LOOKUP, &ksl))
			err(1, "kldsym(%s)", name);
		/* aflag not supported */
		if (uflag && strchr(name, '.') != NULL)
			continue;
		npe->value = ksl.symvalue;
		npe->name = name;
		npe++;
	}
	npe->value = -1;

	*defaultEs = excludes;
	return (0);
}