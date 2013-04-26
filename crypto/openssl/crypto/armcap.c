
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
/*
 * Following subroutines could have been inlined, but it's not all
 * ARM compilers support inline assembler...
 */void _armv7_neon_probe(void);
unsigned int _armv7_tick(void);

unsigned int OPENSSL_rdtsc(void)
	{
	if (OPENSSL_armcap_P|ARMV7_TICK)
		return _armv7_tick();
	else
		return 0;
	}

#if defined(__GNUC__) && __GNUC__>=2
void OPENSSL_cpuid_setup(void) __attribute__((constructor));
#endif
void OPENSSL_cpuid_setup(void)
	{
	char *e;
	struct sigaction	ill_oact,ill_act;
	sigset_t		oset;
	static int trigger=0;

	if (trigger) return;
	trigger=1;
 
	if ((e=getenv("OPENSSL_armcap")))
		{
		OPENSSL_armcap_P=strtoul(e,NULL,0);
		return;
		}

	sigfillset(&all_masked);
	sigdelset(&all_masked,SIGILL);
	sigdelset(&all_masked,SIGTRAP);
	sigdelset(&all_masked,SIGFPE);
	sigdelset(&all_masked,SIGBUS);
	sigdelset(&all_masked,SIGSEGV);

	OPENSSL_armcap_P = 0;

	memset(&ill_act,0,sizeof(ill_act));
	ill_act.sa_handler = ill_handler;
	ill_act.sa_mask    = all_masked;

	sigprocmask(SIG_SETMASK,&ill_act.sa_mask,&oset);
	sigaction(SIGILL,&ill_act,&ill_oact);

	if (sigsetjmp(ill_jmp,1) == 0)
		{
		_armv7_neon_probe();
		OPENSSL_armcap_P |= ARMV7_NEON;
		}
	if (sigsetjmp(ill_jmp,1) == 0)
		{
		_armv7_tick();
		OPENSSL_armcap_P |= ARMV7_TICK;
		}

	sigaction (SIGILL,&ill_oact,NULL);
	sigprocmask(SIG_SETMASK,&oset,NULL);
	}