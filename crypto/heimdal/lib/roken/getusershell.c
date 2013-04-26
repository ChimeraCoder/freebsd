
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

#ifndef HAVE_GETUSERSHELL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef HAVE_PATHS_H
#include <paths.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef HAVE_USERSEC_H
struct aud_rec;
#include <usersec.h>
#endif
#ifdef HAVE_USERCONF_H
#include <userconf.h>
#endif
#include "roken.h"

#ifndef _PATH_SHELLS
#define _PATH_SHELLS "/etc/shells"
#endif

#ifndef _PATH_BSHELL
#define _PATH_BSHELL "/bin/sh"
#endif

#ifndef _PATH_CSHELL
#define _PATH_CSHELL "/bin/csh"
#endif

/*
 * Local shells should NOT be added here.  They should be added in
 * /etc/shells.
 */

static char *okshells[] = { _PATH_BSHELL, _PATH_CSHELL, NULL };
static char **curshell, **shells, *strings;
static char **initshells (void);

/*
 * Get a list of shells from _PATH_SHELLS, if it exists.
 */
ROKEN_LIB_FUNCTION char * ROKEN_LIB_CALL
getusershell()
{
    char *ret;

    if (curshell == NULL)
	curshell = initshells();
    ret = *curshell;
    if (ret != NULL)
	curshell++;
    return (ret);
}

ROKEN_LIB_FUNCTION void ROKEN_LIB_CALL
endusershell()
{
    if (shells != NULL)
	free(shells);
    shells = NULL;
    if (strings != NULL)
	free(strings);
    strings = NULL;
    curshell = NULL;
}

ROKEN_LIB_FUNCTION void ROKEN_LIB_CALL
setusershell()
{
    curshell = initshells();
}

static char **
initshells()
{
    char **sp, *cp;
#ifdef HAVE_GETCONFATTR
    char *tmp;
    int nsh;
#else
    FILE *fp;
#endif
    struct stat statb;

    free(shells);
    shells = NULL;
    free(strings);
    strings = NULL;
#ifdef HAVE_GETCONFATTR
    if(getconfattr(SC_SYS_LOGIN, SC_SHELLS, &tmp, SEC_LIST) != 0)
	return okshells;

    for(cp = tmp, nsh = 0; *cp; cp += strlen(cp) + 1, nsh++);

    shells = calloc(nsh + 1, sizeof(*shells));
    if(shells == NULL)
	return okshells;

    strings = malloc(cp - tmp);
    if(strings == NULL) {
	free(shells);
	shells = NULL;
	return okshells;
    }
    memcpy(strings, tmp, cp - tmp);
    for(sp = shells, cp = strings; *cp; cp += strlen(cp) + 1, sp++)
	*sp = cp;
#else
    if ((fp = fopen(_PATH_SHELLS, "r")) == NULL)
	return (okshells);
    if (fstat(fileno(fp), &statb) == -1) {
	fclose(fp);
	return (okshells);
    }
    if ((strings = malloc((u_int)statb.st_size)) == NULL) {
	fclose(fp);
	return (okshells);
    }
    shells = calloc((unsigned)statb.st_size / 3, sizeof (char *));
    if (shells == NULL) {
	fclose(fp);
	free(strings);
	strings = NULL;
	return (okshells);
    }
    sp = shells;
    cp = strings;
    while (fgets(cp, MaxPathLen + 1, fp) != NULL) {
	while (*cp != '#' && *cp != '/' && *cp != '\0')
	    cp++;
	if (*cp == '#' || *cp == '\0')
	    continue;
	*sp++ = cp;
	while (!isspace((unsigned char)*cp) && *cp != '#' && *cp != '\0')
	    cp++;
	*cp++ = '\0';
    }
    fclose(fp);
#endif
    *sp = NULL;
    return (shells);
}
#endif /* HAVE_GETUSERSHELL */