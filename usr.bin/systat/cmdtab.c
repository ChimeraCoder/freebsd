
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

#ifdef lint
static const char sccsid[] = "@(#)cmdtab.c	8.1 (Berkeley) 6/6/93";
#endif

#include "systat.h"
#include "extern.h"
#include "mode.h"

struct	cmdtab cmdtab[] = {
	{ "pigs",	showpigs,	fetchpigs,	labelpigs,
	  initpigs,	openpigs,	closepigs,	0,
	  0,		CF_LOADAV },
	{ "swap",	showswap,	fetchswap,	labelswap,
	  initswap,	openswap,	closeswap,	0,
	  0,		CF_LOADAV },
	{ "mbufs",	showmbufs,	fetchmbufs,	labelmbufs,
	  initmbufs,	openmbufs,	closembufs,	0,
	  0,		CF_LOADAV },
	{ "iostat",	showiostat,	fetchiostat,	labeliostat,
	  initiostat,	openiostat,	closeiostat,	cmdiostat,
	  0,		CF_LOADAV },
	{ "vmstat",	showkre,	fetchkre,	labelkre,
	  initkre,	openkre,	closekre,	cmdkre,
	  0,		0 },
	{ "netstat",	shownetstat,	fetchnetstat,	labelnetstat,
	  initnetstat,	opennetstat,	closenetstat,	cmdnetstat,
	  0,		CF_LOADAV },
	{ "icmp",	showicmp,	fetchicmp,	labelicmp,
	  initicmp,	openicmp,	closeicmp,	cmdmode,
	  reseticmp,	CF_LOADAV },
	{ "ip",		showip,		fetchip,	labelip,
	  initip,	openip,		closeip,	cmdmode,
	  resetip,	CF_LOADAV },
#ifdef INET6
	{ "icmp6",	showicmp6,	fetchicmp6,	labelicmp6,
	  initicmp6,	openicmp6,	closeicmp6,	cmdmode,
	  reseticmp6,	CF_LOADAV },
	{ "ip6",	showip6,	fetchip6,	labelip6,
	  initip6,	openip6,	closeip6,	cmdmode,
	  resetip6,	CF_LOADAV },
#endif
	{ "tcp",	showtcp,	fetchtcp,	labeltcp,
	  inittcp,	opentcp,	closetcp,	cmdmode,
	  resettcp,	CF_LOADAV },
	{ "ifstat",	showifstat,	fetchifstat,	labelifstat,
	  initifstat,	openifstat,	closeifstat,	cmdifstat,
	  0,		CF_LOADAV },
	{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0 }
};
struct  cmdtab *curcmd = &cmdtab[0];