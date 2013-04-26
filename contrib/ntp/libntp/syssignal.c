
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
/* Added for PPS clocks on Solaris 7 which get EINTR errors */# ifdef SIGPOLL
	if (sig == SIGPOLL) vec.sa_flags = SA_RESTART;
# endif
# ifdef SIGIO
	if (sig == SIGIO)   vec.sa_flags = SA_RESTART;
# endif
#endif

	while (1)
	{
		struct sigaction ovec;

		n = sigaction(sig, &vec, &ovec);
		if (n == -1 && errno == EINTR) continue;
		if (ovec.sa_flags
#ifdef	SA_RESTART
		    && ovec.sa_flags != SA_RESTART
#endif
		    )
			msyslog(LOG_DEBUG, "signal_no_reset: signal %d had flags %x",
				sig, ovec.sa_flags);
		break;
	}
	if (n == -1) {
		perror("sigaction");
		exit(1);
	}
}

#elif  HAVE_SIGVEC

void
signal_no_reset(
	int sig,
	RETSIGTYPE (*func) (int)
	)
{
	struct sigvec sv;
	int n;

	bzero((char *) &sv, sizeof(sv));
	sv.sv_handler = func;
	n = sigvec(sig, &sv, (struct sigvec *)NULL);
	if (n == -1) {
		perror("sigvec");
		exit(1);
	}
}

#elif  HAVE_SIGSET

void
signal_no_reset(
	int sig,
	RETSIGTYPE (*func) (int)
	)
{
	int n;

	n = sigset(sig, func);
	if (n == -1) {
		perror("sigset");
		exit(1);
	}
}

#else

/* Beware!	This implementation resets the signal to SIG_DFL */
void
signal_no_reset(
	int sig,
	RETSIGTYPE (*func) (int)
	)
{
#ifdef SIG_ERR
	if (SIG_ERR == signal(sig, func)) {
#else
	int n;
	n = signal(sig, func);
	if (n == -1) {
#endif
		perror("signal");
		exit(1);
	}
}

#endif