
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
#include "sh.h"
#include "tw.h"

RCSID("$tcsh: tc.vers.c,v 3.54 2006/03/02 18:46:45 christos Exp $")

#include "patchlevel.h"


/* fix_version():
 *	Print a reasonable version string, printing all compile time
 *	options that might affect the user.
 */
void
fix_version(void)
{
#ifdef WIDE_STRINGS
# define SSSTR "wide"
#elif defined (SHORT_STRINGS)
# define SSSTR "8b"
#else
# define SSSTR "7b"
#endif 
#ifdef NLS
# define NLSSTR ",nls"
#else
# define NLSSTR ""
#endif 
#ifdef LOGINFIRST
# define LFSTR ",lf"
#else
# define LFSTR ""
#endif 
#ifdef DOTLAST
# define DLSTR ",dl"
#else
# define DLSTR ""
#endif 
#ifdef VIDEFAULT
# define VISTR ",vi"
#else
# define VISTR ""
#endif 
#ifdef TESLA
# define DTRSTR ",dtr"
#else
# define DTRSTR ""
#endif 
#ifdef KAI
# define BYESTR ",bye"
#else
# define BYESTR ""
#endif 
#ifdef AUTOLOGOUT
# define ALSTR ",al"
#else
# define ALSTR ""
#endif 
#ifdef KANJI
# define KANSTR ",kan"
#else
# define KANSTR ""
#endif 
#ifdef SYSMALLOC
# define SMSTR	",sm"
#else
# define SMSTR  ""
#endif 
#ifdef HASHBANG
# define HBSTR	",hb"
#else
# define HBSTR  ""
#endif 
#ifdef NEWGRP
# define NGSTR	",ng"
#else
# define NGSTR	""
#endif
#ifdef REMOTEHOST
# define RHSTR	",rh"
#else
# define RHSTR	""
#endif
#ifdef AFS
# define AFSSTR	",afs"
#else
# define AFSSTR	""
#endif
#ifdef NODOT
# define NDSTR	",nd"
#else
# define NDSTR	""
#endif
#ifdef COLOR_LS_F
# define COLORSTR ",color"
#else /* ifndef COLOR_LS_F */
# define COLORSTR ""
#endif /* COLOR_LS_F */
#ifdef DSPMBYTE
# define DSPMSTR ",dspm"
#else
# define DSPMSTR ""
#endif
#ifdef COLORCAT
# define CCATSTR ",ccat"
#else
# define CCATSTR ""
#endif
#if defined(FILEC) && defined(TIOCSTI)
# define FILECSTR ",filec"
#else
# define FILECSTR ""
#endif
/* if you want your local version to say something */
#ifndef LOCALSTR
# define LOCALSTR ""
#endif /* LOCALSTR */
    char    *version;
    const Char *machtype = tgetenv(STRMACHTYPE);
    const Char *vendor   = tgetenv(STRVENDOR);
    const Char *ostype   = tgetenv(STROSTYPE);

    if (vendor == NULL)
	vendor = STRunknown;
    if (machtype == NULL)
	machtype = STRunknown;
    if (ostype == NULL)
	ostype = STRunknown;


    version = xasprintf(
"tcsh %d.%.2d.%.2d (%s) %s (%S-%S-%S) options %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
	     REV, VERS, PATCHLEVEL, ORIGIN, DATE, machtype, vendor, ostype,
	     SSSTR, NLSSTR, LFSTR, DLSTR, VISTR, DTRSTR, BYESTR,
	     ALSTR, KANSTR, SMSTR, HBSTR, NGSTR, RHSTR, AFSSTR, NDSTR,
	     COLORSTR, DSPMSTR, CCATSTR, FILECSTR, LOCALSTR);
    cleanup_push(version, xfree);
    setcopy(STRversion, str2short(version), VAR_READWRITE);
    cleanup_until(version);
    version = xasprintf("%d.%.2d.%.2d", REV, VERS, PATCHLEVEL);
    cleanup_push(version, xfree);
    setcopy(STRtcsh, str2short(version), VAR_READWRITE);
    cleanup_until(version);
}