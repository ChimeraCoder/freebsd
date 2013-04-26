
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

#include <openssl/e_os2.h>
#if !defined(OPENSSL_SYS_MSDOS) && !defined(OPENSSL_SYS_VMS) && !defined(OPENSSL_SYS_WIN32)
#ifdef OPENSSL_UNISTD
# include OPENSSL_UNISTD
#else
# include <unistd.h>
#endif
/* If unistd.h defines _POSIX_VERSION, we conclude that we
 * are on a POSIX system and have sigaction and termios. */
#if defined(_POSIX_VERSION)

# define SIGACTION
# if !defined(TERMIOS) && !defined(TERMIO) && !defined(SGTTY)
# define TERMIOS
# endif

#endif
#endif

/* #define SIGACTION */ /* Define this if you have sigaction() */

#ifdef WIN16TTY
#undef OPENSSL_SYS_WIN16
#undef _WINDOWS
#include <graph.h>
#endif

/* 06-Apr-92 Luke Brennan    Support for VMS */
#include "des_locl.h"
#include "cryptlib.h"
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <errno.h>

#ifdef OPENSSL_SYS_VMS			/* prototypes for sys$whatever */
#include <starlet.h>
#ifdef __DECC
#pragma message disable DOLLARID
#endif
#endif

#ifdef WIN_CONSOLE_BUG
#include <windows.h>
#ifndef OPENSSL_SYS_WINCE
#include <wincon.h>
#endif
#endif


/* There are 5 types of terminal interface supported,
 * TERMIO, TERMIOS, VMS, MSDOS and SGTTY
 */

#if defined(__sgi) && !defined(TERMIOS)
#define TERMIOS
#undef  TERMIO
#undef  SGTTY
#endif

#if defined(linux) && !defined(TERMIO)
#undef  TERMIOS
#define TERMIO
#undef  SGTTY
#endif

#ifdef _LIBC
#undef  TERMIOS
#define TERMIO
#undef  SGTTY
#endif

#if !defined(TERMIO) && !defined(TERMIOS) && !defined(OPENSSL_SYS_VMS) && !defined(OPENSSL_SYS_MSDOS) && !defined(MAC_OS_pre_X) && !defined(MAC_OS_GUSI_SOURCE)
#undef  TERMIOS
#undef  TERMIO
#define SGTTY
#endif

#if defined(OPENSSL_SYS_VXWORKS)
#undef TERMIOS
#undef TERMIO
#undef SGTTY
#endif

#ifdef TERMIOS
#include <termios.h>
#define TTY_STRUCT		struct termios
#define TTY_FLAGS		c_lflag
#define	TTY_get(tty,data)	tcgetattr(tty,data)
#define TTY_set(tty,data)	tcsetattr(tty,TCSANOW,data)
#endif

#ifdef TERMIO
#include <termio.h>
#define TTY_STRUCT		struct termio
#define TTY_FLAGS		c_lflag
#define TTY_get(tty,data)	ioctl(tty,TCGETA,data)
#define TTY_set(tty,data)	ioctl(tty,TCSETA,data)
#endif

#ifdef SGTTY
#include <sgtty.h>
#define TTY_STRUCT		struct sgttyb
#define TTY_FLAGS		sg_flags
#define TTY_get(tty,data)	ioctl(tty,TIOCGETP,data)
#define TTY_set(tty,data)	ioctl(tty,TIOCSETP,data)
#endif

#if !defined(_LIBC) && !defined(OPENSSL_SYS_MSDOS) && !defined(OPENSSL_SYS_VMS) && !defined(MAC_OS_pre_X)
#include <sys/ioctl.h>
#endif

#if defined(OPENSSL_SYS_MSDOS) && !defined(__CYGWIN32__) && !defined(OPENSSL_SYS_WINCE)
#include <conio.h>
#define fgets(a,b,c) noecho_fgets(a,b,c)
#endif

#ifdef OPENSSL_SYS_VMS
#include <ssdef.h>
#include <iodef.h>
#include <ttdef.h>
#include <descrip.h>
struct IOSB {
	short iosb$w_value;
	short iosb$w_count;
	long  iosb$l_info;
	};
#endif

#if defined(MAC_OS_pre_X) || defined(MAC_OS_GUSI_SOURCE)
/*
 * This one needs work. As a matter of fact the code is unoperational
 * and this is only a trick to get it compiled.
 *					<appro@fy.chalmers.se>
 */
#define TTY_STRUCT int
#endif

#ifndef NX509_SIG
#define NX509_SIG 32
#endif

static void read_till_nl(FILE *);
static void recsig(int);
static void pushsig(void);
static void popsig(void);
#if defined(OPENSSL_SYS_MSDOS) && !defined(OPENSSL_SYS_WIN16)
static int noecho_fgets(char *buf, int size, FILE *tty);
#endif
#ifdef SIGACTION
 static struct sigaction savsig[NX509_SIG];
#else
  static void (*savsig[NX509_SIG])(int );
#endif
static jmp_buf save;

int des_read_pw_string(char *buf, int length, const char *prompt,
	     int verify)
	{
	char buff[BUFSIZ];
	int ret;

	ret=des_read_pw(buf,buff,(length>BUFSIZ)?BUFSIZ:length,prompt,verify);
	OPENSSL_cleanse(buff,BUFSIZ);
	return(ret);
	}

#ifdef OPENSSL_SYS_WINCE

int des_read_pw(char *buf, char *buff, int size, const char *prompt, int verify)
	{ 
	memset(buf,0,size);
	memset(buff,0,size);
	return(0);
	}

#elif defined(OPENSSL_SYS_WIN16)

int des_read_pw(char *buf, char *buff, int size, char *prompt, int verify)
	{ 
	memset(buf,0,size);
	memset(buff,0,size);
	return(0);
	}

#else /* !OPENSSL_SYS_WINCE && !OPENSSL_SYS_WIN16 */

static void read_till_nl(FILE *in)
	{
#define SIZE 4
	char buf[SIZE+1];

	do	{
		fgets(buf,SIZE,in);
		} while (strchr(buf,'\n') == NULL);
	}


/* return 0 if ok, 1 (or -1) otherwise */
int des_read_pw(char *buf, char *buff, int size, const char *prompt,
	     int verify)
	{
#ifdef OPENSSL_SYS_VMS
	struct IOSB iosb;
	$DESCRIPTOR(terminal,"TT");
	long tty_orig[3], tty_new[3];
	long status;
	unsigned short channel = 0;
#else
#if !defined(OPENSSL_SYS_MSDOS) || defined(__DJGPP__)
	TTY_STRUCT tty_orig,tty_new;
#endif
#endif
	int number;
	int ok;
	/* statics are simply to avoid warnings about longjmp clobbering
	   things */
	static int ps;
	int is_a_tty;
	static FILE *tty;
	char *p;

	if (setjmp(save))
		{
		ok=0;
		goto error;
		}

	number=5;
	ok=0;
	ps=0;
	is_a_tty=1;
	tty=NULL;

#ifdef OPENSSL_SYS_MSDOS
	if ((tty=fopen("con","r")) == NULL)
		tty=stdin;
#elif defined(MAC_OS_pre_X) || defined(OPENSSL_SYS_VXWORKS)
	tty=stdin;
#else
#ifndef OPENSSL_SYS_MPE
	if ((tty=fopen("/dev/tty","r")) == NULL)
#endif
		tty=stdin;
#endif

#if defined(TTY_get) && !defined(OPENSSL_SYS_VMS)
	if (TTY_get(fileno(tty),&tty_orig) == -1)
		{
#ifdef ENOTTY
		if (errno == ENOTTY)
			is_a_tty=0;
		else
#endif
#ifdef EINVAL
		/* Ariel Glenn ariel@columbia.edu reports that solaris
		 * can return EINVAL instead.  This should be ok */
		if (errno == EINVAL)
			is_a_tty=0;
		else
#endif
			return(-1);
		}
	memcpy(&(tty_new),&(tty_orig),sizeof(tty_orig));
#endif
#ifdef OPENSSL_SYS_VMS
	status = sys$assign(&terminal,&channel,0,0);
	if (status != SS$_NORMAL)
		return(-1);
	status=sys$qiow(0,channel,IO$_SENSEMODE,&iosb,0,0,tty_orig,12,0,0,0,0);
	if ((status != SS$_NORMAL) || (iosb.iosb$w_value != SS$_NORMAL))
		return(-1);
#endif

	pushsig();
	ps=1;

#ifdef TTY_FLAGS
	tty_new.TTY_FLAGS &= ~ECHO;
#endif

#if defined(TTY_set) && !defined(OPENSSL_SYS_VMS)
	if (is_a_tty && (TTY_set(fileno(tty),&tty_new) == -1))
#ifdef OPENSSL_SYS_MPE 
		; /* MPE lies -- echo really has been disabled */
#else
		return(-1);
#endif
#endif
#ifdef OPENSSL_SYS_VMS
	tty_new[0] = tty_orig[0];
	tty_new[1] = tty_orig[1] | TT$M_NOECHO;
	tty_new[2] = tty_orig[2];
	status = sys$qiow(0,channel,IO$_SETMODE,&iosb,0,0,tty_new,12,0,0,0,0);
	if ((status != SS$_NORMAL) || (iosb.iosb$w_value != SS$_NORMAL))
		return(-1);
#endif
	ps=2;

	while ((!ok) && (number--))
		{
		fputs(prompt,stderr);
		fflush(stderr);

		buf[0]='\0';
		fgets(buf,size,tty);
		if (feof(tty)) goto error;
		if (ferror(tty)) goto error;
		if ((p=(char *)strchr(buf,'\n')) != NULL)
			*p='\0';
		else	read_till_nl(tty);
		if (verify)
			{
			fprintf(stderr,"\nVerifying password - %s",prompt);
			fflush(stderr);
			buff[0]='\0';
			fgets(buff,size,tty);
			if (feof(tty)) goto error;
			if ((p=(char *)strchr(buff,'\n')) != NULL)
				*p='\0';
			else	read_till_nl(tty);
				
			if (strcmp(buf,buff) != 0)
				{
				fprintf(stderr,"\nVerify failure");
				fflush(stderr);
				break;
				/* continue; */
				}
			}
		ok=1;
		}

error:
	fprintf(stderr,"\n");
#if 0
	perror("fgets(tty)");
#endif
	/* What can we do if there is an error? */
#if defined(TTY_set) && !defined(OPENSSL_SYS_VMS)
	if (ps >= 2) TTY_set(fileno(tty),&tty_orig);
#endif
#ifdef OPENSSL_SYS_VMS
	if (ps >= 2)
		status = sys$qiow(0,channel,IO$_SETMODE,&iosb,0,0
			,tty_orig,12,0,0,0,0);
#endif
	
	if (ps >= 1) popsig();
	if (stdin != tty) fclose(tty);
#ifdef OPENSSL_SYS_VMS
	status = sys$dassgn(channel);
#endif
	return(!ok);
	}

static void pushsig(void)
	{
	int i;
#ifdef SIGACTION
	struct sigaction sa;

	memset(&sa,0,sizeof sa);
	sa.sa_handler=recsig;
#endif

	for (i=1; i<NX509_SIG; i++)
		{
#ifdef SIGUSR1
		if (i == SIGUSR1)
			continue;
#endif
#ifdef SIGUSR2
		if (i == SIGUSR2)
			continue;
#endif
#ifdef SIGACTION
		sigaction(i,&sa,&savsig[i]);
#else
		savsig[i]=signal(i,recsig);
#endif
		}

#ifdef SIGWINCH
	signal(SIGWINCH,SIG_DFL);
#endif
	}

static void popsig(void)
	{
	int i;

	for (i=1; i<NX509_SIG; i++)
		{
#ifdef SIGUSR1
		if (i == SIGUSR1)
			continue;
#endif
#ifdef SIGUSR2
		if (i == SIGUSR2)
			continue;
#endif
#ifdef SIGACTION
		sigaction(i,&savsig[i],NULL);
#else
		signal(i,savsig[i]);
#endif
		}
	}

static void recsig(int i)
	{
	longjmp(save,1);
#ifdef LINT
	i=i;
#endif
	}

#ifdef OPENSSL_SYS_MSDOS
static int noecho_fgets(char *buf, int size, FILE *tty)
	{
	int i;
	char *p;

	p=buf;
	for (;;)
		{
		if (size == 0)
			{
			*p='\0';
			break;
			}
		size--;
#ifdef WIN16TTY
		i=_inchar();
#else
		i=getch();
#endif
		if (i == '\r') i='\n';
		*(p++)=i;
		if (i == '\n')
			{
			*p='\0';
			break;
			}
		}
#ifdef WIN_CONSOLE_BUG
/* Win95 has several evil console bugs: one of these is that the
 * last character read using getch() is passed to the next read: this is
 * usually a CR so this can be trouble. No STDIO fix seems to work but
 * flushing the console appears to do the trick.
 */
		{
			HANDLE inh;
			inh = GetStdHandle(STD_INPUT_HANDLE);
			FlushConsoleInputBuffer(inh);
		}
#endif
	return(strlen(buf));
	}
#endif
#endif /* !OPENSSL_SYS_WINCE && !WIN16 */