
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
/* This code path is disabled, because of incompatibility of
 * libdevinfo.so.1 and libmalloc.so.1 (see below for details)
 */#include <malloc.h>
#include <dlfcn.h>
#include <libdevinfo.h>
#include <sys/systeminfo.h>

typedef di_node_t (*di_init_t)(const char *,uint_t);
typedef void      (*di_fini_t)(di_node_t);
typedef char *    (*di_node_name_t)(di_node_t);
typedef int       (*di_walk_node_t)(di_node_t,uint_t,di_node_name_t,int (*)(di_node_t,di_node_name_t));

#define DLLINK(h,name) (name=(name##_t)dlsym((h),#name))

static int walk_nodename(di_node_t node, di_node_name_t di_node_name)
	{
	char *name = (*di_node_name)(node);

	/* This is expected to catch all UltraSPARC flavors prior T1 */
	if (!strcmp (name,"SUNW,UltraSPARC") ||
	    !strncmp(name,"SUNW,UltraSPARC-I",17))  /* covers II,III,IV */
		{
		OPENSSL_sparcv9cap_P |= SPARCV9_PREFER_FPU|SPARCV9_VIS1;

		/* %tick is privileged only on UltraSPARC-I/II, but not IIe */
		if (name[14]!='\0' && name[17]!='\0' && name[18]!='\0')
			OPENSSL_sparcv9cap_P &= ~SPARCV9_TICK_PRIVILEGED;

		return DI_WALK_TERMINATE;
		}
	/* This is expected to catch remaining UltraSPARCs, such as T1 */
	else if (!strncmp(name,"SUNW,UltraSPARC",15))
		{
		OPENSSL_sparcv9cap_P &= ~SPARCV9_TICK_PRIVILEGED;

		return DI_WALK_TERMINATE;
		}

	return DI_WALK_CONTINUE;
	}

void OPENSSL_cpuid_setup(void)
	{
	void *h;
	char *e,si[256];
	static int trigger=0;

	if (trigger) return;
	trigger=1;

	if ((e=getenv("OPENSSL_sparcv9cap")))
		{
		OPENSSL_sparcv9cap_P=strtoul(e,NULL,0);
		return;
		}

	if (sysinfo(SI_MACHINE,si,sizeof(si))>0)
		{
		if (strcmp(si,"sun4v"))
			/* FPU is preferred for all CPUs, but US-T1/2 */
			OPENSSL_sparcv9cap_P |= SPARCV9_PREFER_FPU;
		}

	if (sysinfo(SI_ISALIST,si,sizeof(si))>0)
		{
		if (strstr(si,"+vis"))
			OPENSSL_sparcv9cap_P |= SPARCV9_VIS1;
		if (strstr(si,"+vis2"))
			{
			OPENSSL_sparcv9cap_P |= SPARCV9_VIS2;
			OPENSSL_sparcv9cap_P &= ~SPARCV9_TICK_PRIVILEGED;
			return;
			}
		}
#ifdef M_KEEP
	/*
	 * Solaris libdevinfo.so.1 is effectively incomatible with
	 * libmalloc.so.1. Specifically, if application is linked with
	 * -lmalloc, it crashes upon startup with SIGSEGV in
	 * free(3LIBMALLOC) called by di_fini. Prior call to
	 * mallopt(M_KEEP,0) somehow helps... But not always...
	 */
	if ((h = dlopen(NULL,RTLD_LAZY)))
		{
		union { void *p; int (*f)(int,int); } sym;
		if ((sym.p = dlsym(h,"mallopt"))) (*sym.f)(M_KEEP,0);
		dlclose(h);
		}
#endif
	if ((h = dlopen("libdevinfo.so.1",RTLD_LAZY))) do
		{
		di_init_t	di_init;
		di_fini_t	di_fini;
		di_walk_node_t	di_walk_node;
		di_node_name_t	di_node_name;
		di_node_t	root_node;

		if (!DLLINK(h,di_init))		break;
		if (!DLLINK(h,di_fini))		break;
		if (!DLLINK(h,di_walk_node))	break;
		if (!DLLINK(h,di_node_name))	break;

		if ((root_node = (*di_init)("/",DINFOSUBTREE))!=DI_NODE_NIL)
			{
			(*di_walk_node)(root_node,DI_WALK_SIBFIRST,
					di_node_name,walk_nodename);
			(*di_fini)(root_node);
			}
		} while(0);

	if (h) dlclose(h);
	}

#else

static sigjmp_buf common_jmp;
static void common_handler(int sig) { siglongjmp(common_jmp,sig); }

void OPENSSL_cpuid_setup(void)
	{
	char *e;
	struct sigaction	common_act,ill_oact,bus_oact;
	sigset_t		all_masked,oset;
	static int trigger=0;

	if (trigger) return;
	trigger=1;
 
	if ((e=getenv("OPENSSL_sparcv9cap")))
		{
		OPENSSL_sparcv9cap_P=strtoul(e,NULL,0);
		return;
		}

	/* Initial value, fits UltraSPARC-I&II... */
	OPENSSL_sparcv9cap_P = SPARCV9_PREFER_FPU|SPARCV9_TICK_PRIVILEGED;

	sigfillset(&all_masked);
	sigdelset(&all_masked,SIGILL);
	sigdelset(&all_masked,SIGTRAP);
#ifdef SIGEMT
	sigdelset(&all_masked,SIGEMT);
#endif
	sigdelset(&all_masked,SIGFPE);
	sigdelset(&all_masked,SIGBUS);
	sigdelset(&all_masked,SIGSEGV);
	sigprocmask(SIG_SETMASK,&all_masked,&oset);

	memset(&common_act,0,sizeof(common_act));
	common_act.sa_handler = common_handler;
	common_act.sa_mask    = all_masked;

	sigaction(SIGILL,&common_act,&ill_oact);
	sigaction(SIGBUS,&common_act,&bus_oact);/* T1 fails 16-bit ldda [on Linux] */

	if (sigsetjmp(common_jmp,1) == 0)
		{
		_sparcv9_rdtick();
		OPENSSL_sparcv9cap_P &= ~SPARCV9_TICK_PRIVILEGED;
		}

	if (sigsetjmp(common_jmp,1) == 0)
		{
		_sparcv9_vis1_probe();
		OPENSSL_sparcv9cap_P |= SPARCV9_VIS1;
		/* detect UltraSPARC-Tx, see sparccpud.S for details... */
		if (_sparcv9_vis1_instrument() >= 12)
			OPENSSL_sparcv9cap_P &= ~(SPARCV9_VIS1|SPARCV9_PREFER_FPU);
		else
			{
			_sparcv9_vis2_probe();
			OPENSSL_sparcv9cap_P |= SPARCV9_VIS2;
			}
		}

	if (sigsetjmp(common_jmp,1) == 0)
		{
		_sparcv9_fmadd_probe();
		OPENSSL_sparcv9cap_P |= SPARCV9_FMADD;
		}

	sigaction(SIGBUS,&bus_oact,NULL);
	sigaction(SIGILL,&ill_oact,NULL);

	sigprocmask(SIG_SETMASK,&oset,NULL);
	}

#endif