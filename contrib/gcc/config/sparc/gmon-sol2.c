
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

/* Mangled into a form that works on SPARC Solaris 2 by Mark Eichin
 * for Cygnus Support, July 1992.
 */

#include "tconfig.h"
#include "tsystem.h"
#include <fcntl.h> /* for creat() */
#include "coretypes.h"
#include "tm.h"

#if 0
#include "sparc/gmon.h"
#else
struct phdr {
  char *lpc;
  char *hpc;
  int ncnt;
};
#define HISTFRACTION 2
#define HISTCOUNTER unsigned short
#define HASHFRACTION 1
#define ARCDENSITY 2
#define MINARCS 50
struct tostruct {
  char *selfpc;
  long count;
  unsigned short link;
};
struct rawarc {
    unsigned long       raw_frompc;
    unsigned long       raw_selfpc;
    long                raw_count;
};
#define ROUNDDOWN(x,y)  (((x)/(y))*(y))
#define ROUNDUP(x,y)    ((((x)+(y)-1)/(y))*(y))

#endif

/* extern mcount() asm ("mcount"); */
/*extern*/ char *minbrk /* asm ("minbrk") */;

    /*
     *	froms is actually a bunch of unsigned shorts indexing tos
     */
static int		profiling = 3;
static unsigned short	*froms;
static struct tostruct	*tos = 0;
static long		tolimit = 0;
static char		*s_lowpc = 0;
static char		*s_highpc = 0;
static unsigned long	s_textsize = 0;

static int	ssiz;
static char	*sbuf;
static int	s_scale;
    /* see profil(2) where this is describe (incorrectly) */
#define		SCALE_1_TO_1	0x10000L

#define	MSG "No space for profiling buffer(s)\n"

static void moncontrol (int);
extern void monstartup (char *, char *);
extern void _mcleanup (void);

void monstartup(char *lowpc, char *highpc)
{
    int			monsize;
    char		*buffer;
    register int	o;

	/*
	 *	round lowpc and highpc to multiples of the density we're using
	 *	so the rest of the scaling (here and in gprof) stays in ints.
	 */
    lowpc = (char *)
	    ROUNDDOWN((unsigned long)lowpc, HISTFRACTION*sizeof(HISTCOUNTER));
    s_lowpc = lowpc;
    highpc = (char *)
	    ROUNDUP((unsigned long)highpc, HISTFRACTION*sizeof(HISTCOUNTER));
    s_highpc = highpc;
    s_textsize = highpc - lowpc;
    monsize = (s_textsize / HISTFRACTION) + sizeof(struct phdr);
    buffer = sbrk( monsize );
    if ( buffer == (char *) -1 ) {
	write( 2 , MSG , sizeof(MSG) );
	return;
    }
    froms = (unsigned short *) sbrk( s_textsize / HASHFRACTION );
    if ( froms == (unsigned short *) -1 ) {
	write( 2 , MSG , sizeof(MSG) );
	froms = 0;
	return;
    }
    tolimit = s_textsize * ARCDENSITY / 100;
    if ( tolimit < MINARCS ) {
	tolimit = MINARCS;
    } else if ( tolimit > 65534 ) {
	tolimit = 65534;
    }
    tos = (struct tostruct *) sbrk( tolimit * sizeof( struct tostruct ) );
    if ( tos == (struct tostruct *) -1 ) {
	write( 2 , MSG , sizeof(MSG) );
	froms = 0;
	tos = 0;
	return;
    }
    minbrk = sbrk(0);
    tos[0].link = 0;
    sbuf = buffer;
    ssiz = monsize;
    ( (struct phdr *) buffer ) -> lpc = lowpc;
    ( (struct phdr *) buffer ) -> hpc = highpc;
    ( (struct phdr *) buffer ) -> ncnt = ssiz;
    monsize -= sizeof(struct phdr);
    if ( monsize <= 0 )
	return;
    o = highpc - lowpc;
    if( monsize < o )
#ifndef hp300
	s_scale = ( (float) monsize / o ) * SCALE_1_TO_1;
#else /* avoid floating point */
    {
	int quot = o / monsize;

	if (quot >= 0x10000)
		s_scale = 1;
	else if (quot >= 0x100)
		s_scale = 0x10000 / quot;
	else if (o >= 0x800000)
		s_scale = 0x1000000 / (o / (monsize >> 8));
	else
		s_scale = 0x1000000 / ((o << 8) / monsize);
    }
#endif
    else
	s_scale = SCALE_1_TO_1;
    moncontrol(1);
}

void
_mcleanup(void)
{
    int			fd;
    int			fromindex;
    int			endfrom;
    char		*frompc;
    int			toindex;
    struct rawarc	rawarc;
    char		*profdir;
    const char		*proffile;
    char		*progname;
    char		 buf[PATH_MAX];
    extern char	       **___Argv;

    moncontrol(0);

    if ((profdir = getenv("PROFDIR")) != NULL) {
	/* If PROFDIR contains a null value, no profiling output is produced */
	if (*profdir == '\0') {
	    return;
	}

	progname=strrchr(___Argv[0], '/');
	if (progname == NULL)
	    progname=___Argv[0];
	else
	    progname++;

	sprintf(buf, "%s/%ld.%s", profdir, (long) getpid(), progname);
	proffile = buf;
    } else {
	proffile = "gmon.out";
    }

    fd = creat( proffile, 0666 );
    if ( fd < 0 ) {
	perror( proffile );
	return;
    }
#   ifdef DEBUG
	fprintf( stderr , "[mcleanup] sbuf 0x%x ssiz %d\n" , sbuf , ssiz );
#   endif /* DEBUG */
    write( fd , sbuf , ssiz );
    endfrom = s_textsize / (HASHFRACTION * sizeof(*froms));
    for ( fromindex = 0 ; fromindex < endfrom ; fromindex++ ) {
	if ( froms[fromindex] == 0 ) {
	    continue;
	}
	frompc = s_lowpc + (fromindex * HASHFRACTION * sizeof(*froms));
	for (toindex=froms[fromindex]; toindex!=0; toindex=tos[toindex].link) {
#	    ifdef DEBUG
		fprintf( stderr ,
			"[mcleanup] frompc 0x%x selfpc 0x%x count %d\n" ,
			frompc , tos[toindex].selfpc , tos[toindex].count );
#	    endif /* DEBUG */
	    rawarc.raw_frompc = (unsigned long) frompc;
	    rawarc.raw_selfpc = (unsigned long) tos[toindex].selfpc;
	    rawarc.raw_count = tos[toindex].count;
	    write( fd , &rawarc , sizeof rawarc );
	}
    }
    close( fd );
}

/*
 * The SPARC stack frame is only held together by the frame pointers
 * in the register windows. According to the SVR4 SPARC ABI
 * Supplement, Low Level System Information/Operating System
 * Interface/Software Trap Types, a type 3 trap will flush all of the
 * register windows to the stack, which will make it possible to walk
 * the frames and find the return addresses.
 * 	However, it seems awfully expensive to incur a trap (system
 * call) for every function call. It turns out that "call" simply puts
 * the return address in %o7 expecting the "save" in the procedure to
 * shift it into %i7; this means that before the "save" occurs, %o7
 * contains the address of the call to mcount, and %i7 still contains
 * the caller above that. The asm mcount here simply saves those
 * registers in argument registers and branches to internal_mcount,
 * simulating a call with arguments.
 * 	Kludges:
 * 	1) the branch to internal_mcount is hard coded; it should be
 * possible to tell asm to use the assembler-name of a symbol.
 * 	2) in theory, the function calling mcount could have saved %i7
 * somewhere and reused the register; in practice, I *think* this will
 * break longjmp (and maybe the debugger) but I'm not certain. (I take
 * some comfort in the knowledge that it will break the native mcount
 * as well.)
 * 	3) if builtin_return_address worked, this could be portable.
 * However, it would really have to be optimized for arguments of 0
 * and 1 and do something like what we have here in order to avoid the
 * trap per function call performance hit. 
 * 	4) the atexit and monsetup calls prevent this from simply
 * being a leaf routine that doesn't do a "save" (and would thus have
 * access to %o7 and %i7 directly) but the call to write() at the end
 * would have also prevented this.
 *
 * -- [eichin:19920702.1107EST]
 */

static void internal_mcount (char *, unsigned short *) __attribute__ ((used));

/* i7 == last ret, -> frompcindex */
/* o7 == current ret, -> selfpc */
/* Solaris 2 libraries use _mcount.  */
asm(".global _mcount; _mcount: mov %i7,%o1; mov %o7,%o0;b,a internal_mcount");
/* This is for compatibility with old versions of gcc which used mcount.  */
asm(".global mcount; mcount: mov %i7,%o1; mov %o7,%o0;b,a internal_mcount");

static void internal_mcount(char *selfpc, unsigned short *frompcindex)
{
	register struct tostruct	*top;
	register struct tostruct	*prevtop;
	register long			toindex;
	static char already_setup;

	/*
	 *	find the return address for mcount,
	 *	and the return address for mcount's caller.
	 */

	if(!already_setup) {
          extern char etext[];
	  extern char _start[];
	  extern char _init[];
	  already_setup = 1;
	  monstartup(_start < _init ? _start : _init, etext);
#ifdef USE_ONEXIT
	  on_exit(_mcleanup, 0);
#else
	  atexit(_mcleanup);
#endif
	}
	/*
	 *	check that we are profiling
	 *	and that we aren't recursively invoked.
	 */
	if (profiling) {
		goto out;
	}
	profiling++;
	/*
	 *	check that frompcindex is a reasonable pc value.
	 *	for example:	signal catchers get called from the stack,
	 *			not from text space.  too bad.
	 */
	frompcindex = (unsigned short *)((long)frompcindex - (long)s_lowpc);
	if ((unsigned long)frompcindex > s_textsize) {
		goto done;
	}
	frompcindex =
	    &froms[((long)frompcindex) / (HASHFRACTION * sizeof(*froms))];
	toindex = *frompcindex;
	if (toindex == 0) {
		/*
		 *	first time traversing this arc
		 */
		toindex = ++tos[0].link;
		if (toindex >= tolimit) {
			goto overflow;
		}
		*frompcindex = toindex;
		top = &tos[toindex];
		top->selfpc = selfpc;
		top->count = 1;
		top->link = 0;
		goto done;
	}
	top = &tos[toindex];
	if (top->selfpc == selfpc) {
		/*
		 *	arc at front of chain; usual case.
		 */
		top->count++;
		goto done;
	}
	/*
	 *	have to go looking down chain for it.
	 *	top points to what we are looking at,
	 *	prevtop points to previous top.
	 *	we know it is not at the head of the chain.
	 */
	for (; /* goto done */; ) {
		if (top->link == 0) {
			/*
			 *	top is end of the chain and none of the chain
			 *	had top->selfpc == selfpc.
			 *	so we allocate a new tostruct
			 *	and link it to the head of the chain.
			 */
			toindex = ++tos[0].link;
			if (toindex >= tolimit) {
				goto overflow;
			}
			top = &tos[toindex];
			top->selfpc = selfpc;
			top->count = 1;
			top->link = *frompcindex;
			*frompcindex = toindex;
			goto done;
		}
		/*
		 *	otherwise, check the next arc on the chain.
		 */
		prevtop = top;
		top = &tos[top->link];
		if (top->selfpc == selfpc) {
			/*
			 *	there it is.
			 *	increment its count
			 *	move it to the head of the chain.
			 */
			top->count++;
			toindex = prevtop->link;
			prevtop->link = top->link;
			top->link = *frompcindex;
			*frompcindex = toindex;
			goto done;
		}

	}
done:
	profiling--;
	/* and fall through */
out:
	return;		/* normal return restores saved registers */

overflow:
	profiling++; /* halt further profiling */
#   define	TOLIMIT	"mcount: tos overflow\n"
	write(2, TOLIMIT, sizeof(TOLIMIT));
	goto out;
}

/*
 * Control profiling
 *	profiling is what mcount checks to see if
 *	all the data structures are ready.
 */
static void moncontrol(int mode)
{
    if (mode) {
	/* start */
	profil((unsigned short *)(sbuf + sizeof(struct phdr)),
	       ssiz - sizeof(struct phdr),
	       (long)s_lowpc, s_scale);
	profiling = 0;
    } else {
	/* stop */
	profil((unsigned short *)0, 0, 0, 0);
	profiling = 3;
    }
}