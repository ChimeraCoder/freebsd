
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
		/* boundary of 32 was experimentally determined on		   Linux 2.6.22, might have to be adjusted on AIX... */
		if (num>=32 && (num&3)==0 && (OPENSSL_ppccap_P&PPC_FPU64))
			{
			sigset_t oset;
			int ret;

			sigprocmask(SIG_SETMASK,&all_masked,&oset);
			ret=bn_mul_mont_fpu64(rp,ap,bp,np,n0,num);
			sigprocmask(SIG_SETMASK,&oset,NULL);

			return ret;
			}
#endif
		}
	else if ((OPENSSL_ppccap_P&PPC_FPU64))
		/* this is a "must" on POWER6, but run-time detection
		 * is not implemented yet... */
		return bn_mul_mont_fpu64(rp,ap,bp,np,n0,num);

	return bn_mul_mont_int(rp,ap,bp,np,n0,num);
	}
#endif

static sigjmp_buf ill_jmp;
static void ill_handler (int sig) { siglongjmp(ill_jmp,sig); }

void OPENSSL_ppc64_probe(void);
void OPENSSL_altivec_probe(void);

void OPENSSL_cpuid_setup(void)
	{
	char *e;
	struct sigaction	ill_oact,ill_act;
	sigset_t		oset;
	static int trigger=0;

	if (trigger) return;
	trigger=1;
 
	sigfillset(&all_masked);
	sigdelset(&all_masked,SIGILL);
	sigdelset(&all_masked,SIGTRAP);
#ifdef SIGEMT
	sigdelset(&all_masked,SIGEMT);
#endif
	sigdelset(&all_masked,SIGFPE);
	sigdelset(&all_masked,SIGBUS);
	sigdelset(&all_masked,SIGSEGV);

	if ((e=getenv("OPENSSL_ppccap")))
		{
		OPENSSL_ppccap_P=strtoul(e,NULL,0);
		return;
		}

	OPENSSL_ppccap_P = 0;

#if defined(_AIX)
	if (sizeof(size_t)==4
# if defined(_SC_AIX_KERNEL_BITMODE)
	    && sysconf(_SC_AIX_KERNEL_BITMODE)!=64
# endif
	   )
		return;
#endif

	memset(&ill_act,0,sizeof(ill_act));
	ill_act.sa_handler = ill_handler;
	ill_act.sa_mask    = all_masked;

	sigprocmask(SIG_SETMASK,&ill_act.sa_mask,&oset);
	sigaction(SIGILL,&ill_act,&ill_oact);

	if (sizeof(size_t)==4)
		{
		if (sigsetjmp(ill_jmp,1) == 0)
			{
			OPENSSL_ppc64_probe();
			OPENSSL_ppccap_P |= PPC_FPU64;
			}
		}
	else
		{
		/*
		 * Wanted code detecting POWER6 CPU and setting PPC_FPU64
		 */
		}

	if (sigsetjmp(ill_jmp,1) == 0)
		{
		OPENSSL_altivec_probe();
		OPENSSL_ppccap_P |= PPC_ALTIVEC;
		}

	sigaction (SIGILL,&ill_oact,NULL);
	sigprocmask(SIG_SETMASK,&oset,NULL);
	}