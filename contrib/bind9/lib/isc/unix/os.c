
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

/* $Id: os.c,v 1.18 2007/06/19 23:47:18 tbox Exp $ */

#include <config.h>

#include <isc/os.h>


#ifdef HAVE_SYSCONF

#include <unistd.h>

#ifndef __hpux
static inline long
sysconf_ncpus(void) {
#if defined(_SC_NPROCESSORS_ONLN)
	return sysconf((_SC_NPROCESSORS_ONLN));
#elif defined(_SC_NPROC_ONLN)
	return sysconf((_SC_NPROC_ONLN));
#else
	return (0);
#endif
}
#endif
#endif /* HAVE_SYSCONF */


#ifdef __hpux

#include <sys/pstat.h>

static inline int
hpux_ncpus(void) {
	struct pst_dynamic psd;
	if (pstat_getdynamic(&psd, sizeof(psd), 1, 0) != -1)
		return (psd.psd_proc_cnt);
	else
		return (0);
}

#endif /* __hpux */

#if defined(HAVE_SYS_SYSCTL_H) && defined(HAVE_SYSCTLBYNAME)
#include <sys/types.h>  /* for FreeBSD */
#include <sys/param.h>  /* for NetBSD */
#include <sys/sysctl.h>

static int
sysctl_ncpus(void) {
	int ncpu, result;
	size_t len;

	len = sizeof(ncpu);
	result = sysctlbyname("hw.ncpu", &ncpu, &len , 0, 0);
	if (result != -1)
		return (ncpu);
	return (0);
}
#endif

unsigned int
isc_os_ncpus(void) {
	long ncpus = 0;

#ifdef __hpux
	ncpus = hpux_ncpus();
#elif defined(HAVE_SYSCONF)
	ncpus = sysconf_ncpus();
#endif
#if defined(HAVE_SYS_SYSCTL_H) && defined(HAVE_SYSCTLBYNAME)
	if (ncpus <= 0)
		ncpus = sysctl_ncpus();
#endif
	if (ncpus <= 0)
		ncpus = 1;

	return ((unsigned int)ncpus);
}