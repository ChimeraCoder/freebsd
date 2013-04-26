
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
#include <sys/firmware.h>
#include <sys/kernel.h>
#include <sys/linker.h>
#include <sys/module.h>
#include <sys/systm.h>

#if	defined(ISP_ALL) || !defined(KLD_MODULE) 
#ifdef __sparc64__
#define	ISP_1000	1
#endif
#define	ISP_1040	1
#define	ISP_1040_IT	1
#define	ISP_1080	1
#define	ISP_1080_IT	1
#define	ISP_12160	1
#define	ISP_12160_IT	1
#define	ISP_2100	1
#define	ISP_2200	1
#define	ISP_2300	1
#define	ISP_2322	1
#define	ISP_2400	1
#define	ISP_2400_MULTI	1
#define	ISP_2500	1
#define	ISP_2500_MULTI	1
#endif

#ifndef MODULE_NAME
#define	MODULE_NAME	"ispfw"
#endif

#if	defined(ISP_1000)
#ifdef __sparc64__
#include <dev/ispfw/asm_1000.h>
#else
#error "firmware not compatible with this platform"
#endif
#endif
#if	defined(ISP_1040) || defined(ISP_1040_IT)
#include <dev/ispfw/asm_1040.h>
#endif
#if	defined(ISP_1080) || defined(ISP_1080_IT)
#include <dev/ispfw/asm_1080.h>
#endif
#if	defined(ISP_12160) || defined(ISP_12160_IT)
#include <dev/ispfw/asm_12160.h>
#endif
#if	defined(ISP_2100)
#include <dev/ispfw/asm_2100.h>
#endif
#if	defined(ISP_2200)
#include <dev/ispfw/asm_2200.h>
#endif
#if	defined(ISP_2300)
#include <dev/ispfw/asm_2300.h>
#endif
#if	defined(ISP_2322)
#include <dev/ispfw/asm_2322.h>
#endif
#if	defined(ISP_2400) || defined(ISP_2400_MULTI)
#include <dev/ispfw/asm_2400.h>
#endif
#if	defined(ISP_2500) || defined(ISP_2500_MULTI)
#include <dev/ispfw/asm_2500.h>
#endif

#if	defined(ISP_1000)
static int	isp_1000_loaded;
#endif
#if	defined(ISP_1040)
static int	isp_1040_loaded;
#endif
#if	defined(ISP_1040_IT)
static int	isp_1040_it_loaded;
#endif
#if	defined(ISP_1080)
static int	isp_1080_loaded;
#endif
#if	defined(ISP_1080_IT)
static int	isp_1080_it_loaded;
#endif
#if	defined(ISP_12160)
static int	isp_12160_loaded;
#endif
#if	defined(ISP_12160_IT)
static int	isp_12160_it_loaded;
#endif
#if	defined(ISP_2100)
static int	isp_2100_loaded;
#endif
#if	defined(ISP_2200)
static int	isp_2200_loaded;
#endif
#if	defined(ISP_2300)
static int	isp_2300_loaded;
#endif
#if	defined(ISP_2322)
static int	isp_2322_loaded;
#endif
#if	defined(ISP_2400)
static int	isp_2400_loaded;
#endif
#if	defined(ISP_2400_MULTI)
static int	isp_2400_multi_loaded;
#endif
#if	defined(ISP_2500)
static int	isp_2500_loaded;
#endif
#if	defined(ISP_2500_MULTI)
static int	isp_2500_multi_loaded;
#endif

#define	ISPFW_VERSION	1

#if	!defined(KLD_MODULE)
#define	ISPFW_KLD	0
#else
#define	ISPFW_KLD	1
#endif

#define	RMACRO(token)	do {						\
	if (token##_loaded)						\
		break;							\
	if (firmware_register(#token, token##_risc_code,		\
	    token##_risc_code[3] * sizeof(token##_risc_code[3]),	\
	    ISPFW_VERSION, NULL) == NULL) {				\
		printf("%s: unable to register firmware <%s>\n",	\
		    MODULE_NAME, #token);				\
		break;							\
	}								\
	token##_loaded++;						\
	if (bootverbose || ISPFW_KLD)					\
		printf("%s: registered firmware <%s>\n", MODULE_NAME, 	\
		    #token);						\
} while (0)

#define	UMACRO(token)	do {						\
	if (!token##_loaded)						\
		break;							\
	if (firmware_unregister(#token) != 0) {				\
		printf("%s: unable to unregister firmware <%s>\n",	\
		    MODULE_NAME, #token);				\
		break;							\
	}								\
	token##_loaded--;						\
	if (bootverbose || ISPFW_KLD)					\
		printf("%s: unregistered firmware <%s>\n", MODULE_NAME,	\
		    #token);						\
} while (0)

static void
do_load_fw(void)
{

#if	defined(ISP_1000)
	RMACRO(isp_1000);
#endif
#if	defined(ISP_1040)
	RMACRO(isp_1040);
#endif
#if	defined(ISP_1040_IT)
	RMACRO(isp_1040_it);
#endif
#if	defined(ISP_1080)
	RMACRO(isp_1080);
#endif
#if	defined(ISP_1080_IT)
	RMACRO(isp_1080_it);
#endif
#if	defined(ISP_12160)
	RMACRO(isp_12160);
#endif
#if	defined(ISP_12160_IT)
	RMACRO(isp_12160_it);
#endif
#if	defined(ISP_2100)
	RMACRO(isp_2100);
#endif
#if	defined(ISP_2200)
	RMACRO(isp_2200);
#endif
#if	defined(ISP_2300)
	RMACRO(isp_2300);
#endif
#if	defined(ISP_2322)
	RMACRO(isp_2322);
#endif
#if	defined(ISP_2400)
	RMACRO(isp_2400);
#endif
#if	defined(ISP_2400_MULTI)
	RMACRO(isp_2400_multi);
#endif
#if	defined(ISP_2500)
	RMACRO(isp_2500);
#endif
#if	defined(ISP_2500_MULTI)
	RMACRO(isp_2500_multi);
#endif
}

static void
do_unload_fw(void)
{

#if	defined(ISP_1000)
	UMACRO(isp_1000);
#endif
#if	defined(ISP_1040)
	UMACRO(isp_1040);
#endif
#if	defined(ISP_1040_IT)
	UMACRO(isp_1040_it);
#endif
#if	defined(ISP_1080)
	UMACRO(isp_1080);
#endif
#if	defined(ISP_1080_IT)
	UMACRO(isp_1080_it);
#endif
#if	defined(ISP_12160)
	UMACRO(isp_12160);
#endif
#if	defined(ISP_12160_IT)
	UMACRO(isp_12160_it);
#endif
#if	defined(ISP_2100)
	UMACRO(isp_2100);
#endif
#if	defined(ISP_2200)
	UMACRO(isp_2200);
#endif
#if	defined(ISP_2300)
	UMACRO(isp_2300);
#endif
#if	defined(ISP_2322)
	UMACRO(isp_2322);
#endif
#if	defined(ISP_2400)
	UMACRO(isp_2400);
#endif
#if	defined(ISP_2400_MULTI)
	UMACRO(isp_2400_multi);
#endif
#if	defined(ISP_2500)
	UMACRO(isp_2500);
#endif
#if	defined(ISP_2500_MULTI)
	UMACRO(isp_2500_multi);
#endif
}

static int
module_handler(module_t mod, int what, void *arg)
{

	switch (what) {
	case MOD_LOAD:
		do_load_fw();
		break;
	case MOD_UNLOAD:
		do_unload_fw();
		break;
	case MOD_SHUTDOWN:
		break;
	default:
		return (EOPNOTSUPP);
	}
	return (0);
}
static moduledata_t ispfw_mod = {
	MODULE_NAME, module_handler, NULL
};
#if	defined(ISP_ALL) || !defined(KLD_MODULE) 
DECLARE_MODULE(ispfw, ispfw_mod, SI_SUB_DRIVERS, SI_ORDER_THIRD);
#elif	defined(ISP_1000)
DECLARE_MODULE(isp_1000, ispfw_mod, SI_SUB_DRIVERS, SI_ORDER_THIRD);
#elif	defined(ISP_1040)
DECLARE_MODULE(isp_1040, ispfw_mod, SI_SUB_DRIVERS, SI_ORDER_THIRD);
#elif	defined(ISP_1040_IT)
DECLARE_MODULE(isp_1040_it, ispfw_mod, SI_SUB_DRIVERS, SI_ORDER_THIRD);
#elif	defined(ISP_1080)
DECLARE_MODULE(isp_1080, ispfw_mod, SI_SUB_DRIVERS, SI_ORDER_THIRD);
#elif	defined(ISP_1080_IT)
DECLARE_MODULE(isp_1080_it, ispfw_mod, SI_SUB_DRIVERS, SI_ORDER_THIRD);
#elif	defined(ISP_12160)
DECLARE_MODULE(isp_12160, ispfw_mod, SI_SUB_DRIVERS, SI_ORDER_THIRD);
#elif	defined(ISP_12160_IT)
DECLARE_MODULE(isp_12160_IT, ispfw_mod, SI_SUB_DRIVERS, SI_ORDER_THIRD);
#elif	defined(ISP_2100)
DECLARE_MODULE(isp_2100, ispfw_mod, SI_SUB_DRIVERS, SI_ORDER_THIRD);
#elif	defined(ISP_2200)
DECLARE_MODULE(isp_2200, ispfw_mod, SI_SUB_DRIVERS, SI_ORDER_THIRD);
#elif	defined(ISP_2300)
DECLARE_MODULE(isp_2300, ispfw_mod, SI_SUB_DRIVERS, SI_ORDER_THIRD);
#elif	defined(ISP_2322)
DECLARE_MODULE(isp_2322, ispfw_mod, SI_SUB_DRIVERS, SI_ORDER_THIRD);
#elif	defined(ISP_2400)
DECLARE_MODULE(isp_2400, ispfw_mod, SI_SUB_DRIVERS, SI_ORDER_THIRD);
#elif	defined(ISP_2400_MULTI)
DECLARE_MODULE(isp_2400_multi, ispfw_mod, SI_SUB_DRIVERS, SI_ORDER_THIRD);
#elif	defined(ISP_2500)
DECLARE_MODULE(isp_2500, ispfw_mod, SI_SUB_DRIVERS, SI_ORDER_THIRD);
#elif	defined(ISP_2500_MULTI)
DECLARE_MODULE(isp_2500_multi, ispfw_mod, SI_SUB_DRIVERS, SI_ORDER_THIRD);
#else
#error	"firmware not specified"
#endif