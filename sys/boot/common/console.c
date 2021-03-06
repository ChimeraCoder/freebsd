
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
/*
 * Core console support
 */

static int	cons_set(struct env_var *ev, int flags, const void *value);
static int	cons_find(const char *name);
static int	cons_check(const char *string);
static void	cons_change(const char *string);

/*
 * Detect possible console(s) to use.  If preferred console(s) have been
 * specified, mark them as active. Else, mark the first probed console
 * as active.  Also create the console variable.
 */
void
cons_probe(void) 
{
    int			cons;
    int			active;
    char		*prefconsole;
    
    /* Do all console probes */
    for (cons = 0; consoles[cons] != NULL; cons++) {
	consoles[cons]->c_flags = 0;
 	consoles[cons]->c_probe(consoles[cons]);
    }
    /* Now find the first working one */
    active = -1;
    for (cons = 0; consoles[cons] != NULL && active == -1; cons++) {
	consoles[cons]->c_flags = 0;
 	consoles[cons]->c_probe(consoles[cons]);
	if (consoles[cons]->c_flags == (C_PRESENTIN | C_PRESENTOUT))
	    active = cons;
    }
    /* Force a console even if all probes failed */
    if (active == -1)
	active = 0;

    /* Check to see if a console preference has already been registered */
    prefconsole = getenv("console");
    if (prefconsole != NULL)
	prefconsole = strdup(prefconsole);
    if (prefconsole != NULL) {
	unsetenv("console");		/* we want to replace this */
	cons_change(prefconsole);
    } else {
	consoles[active]->c_flags |= C_ACTIVEIN | C_ACTIVEOUT;
	consoles[active]->c_init(0);
	prefconsole = strdup(consoles[active]->c_name);
    }

    printf("Consoles: ");
    for (cons = 0; consoles[cons] != NULL; cons++)
	if (consoles[cons]->c_flags & (C_ACTIVEIN | C_ACTIVEOUT))
	    printf("%s  ", consoles[cons]->c_desc);
    printf("\n");

    if (prefconsole != NULL) {
	env_setenv("console", EV_VOLATILE, prefconsole, cons_set,
	    env_nounset);
	free(prefconsole);
    }
}

int
getchar(void)
{
    int		cons;
    int		rv;

    /* Loop forever polling all active consoles */
    for(;;)
	for (cons = 0; consoles[cons] != NULL; cons++)
	    if ((consoles[cons]->c_flags & (C_PRESENTIN | C_ACTIVEIN)) ==
		(C_PRESENTIN | C_ACTIVEIN) &&
		((rv = consoles[cons]->c_in()) != -1))
		return(rv);
}

int
ischar(void)
{
    int		cons;

    for (cons = 0; consoles[cons] != NULL; cons++)
	if ((consoles[cons]->c_flags & (C_PRESENTIN | C_ACTIVEIN)) ==
	    (C_PRESENTIN | C_ACTIVEIN) &&
	    (consoles[cons]->c_ready() != 0))
		return(1);
    return(0);
}

void
putchar(int c)
{
    int		cons;

    /* Expand newlines */
    if (c == '\n')
	putchar('\r');

    for (cons = 0; consoles[cons] != NULL; cons++)
	if ((consoles[cons]->c_flags & (C_PRESENTOUT | C_ACTIVEOUT)) ==
	    (C_PRESENTOUT | C_ACTIVEOUT))
	    consoles[cons]->c_out(c);
}

/*
 * Find the console with the specified name.
 */
static int
cons_find(const char *name)
{
    int		cons;

    for (cons = 0; consoles[cons] != NULL; cons++)
	if (!strcmp(consoles[cons]->c_name, name))
	    return (cons);
    return (-1);
}

/*
 * Select one or more consoles.
 */
static int
cons_set(struct env_var *ev, int flags, const void *value)
{
    int		cons;

    if ((value == NULL) || (cons_check(value) == -1)) {
	if (value != NULL) 
	    printf("no such console!\n");
	printf("Available consoles:\n");
	for (cons = 0; consoles[cons] != NULL; cons++)
	    printf("    %s\n", consoles[cons]->c_name);
	return(CMD_ERROR);
    }

    cons_change(value);

    env_setenv(ev->ev_name, flags | EV_NOHOOK, value, NULL, NULL);
    return(CMD_OK);
}

/*
 * Check that all of the consoles listed in *string are valid consoles
 */
static int
cons_check(const char *string)
{
    int		cons;
    char	*curpos, *dup, *next;

    dup = next = strdup(string);
    cons = -1;
    while (next != NULL) {
	curpos = strsep(&next, " ,");
	if (*curpos != '\0') {
	    cons = cons_find(curpos);
	    if (cons == -1)
		break;
	}
    }

    free(dup);
    return (cons);
}

/*
 * Activate all of the consoles listed in *string and disable all the others.
 */
static void
cons_change(const char *string)
{
    int		cons;
    char	*curpos, *dup, *next;

    /* Disable all consoles */
    for (cons = 0; consoles[cons] != NULL; cons++) {
	consoles[cons]->c_flags &= ~(C_ACTIVEIN | C_ACTIVEOUT);
    }

    /* Enable selected consoles */
    dup = next = strdup(string);
    while (next != NULL) {
	curpos = strsep(&next, " ,");
	if (*curpos == '\0')
		continue;
	cons = cons_find(curpos);
	if (cons >= 0) {
	    consoles[cons]->c_flags |= C_ACTIVEIN | C_ACTIVEOUT;
	    consoles[cons]->c_init(0);
	    if ((consoles[cons]->c_flags & (C_PRESENTIN | C_PRESENTOUT)) !=
		(C_PRESENTIN | C_PRESENTOUT))
		printf("console %s failed to initialize\n",
		    consoles[cons]->c_name);
	}
    }

    free(dup);
}