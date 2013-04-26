
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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>

#define FRAME_SIZE	1024
#define FRAME_OVERHEAD	  40

struct args
{
	void	*top;	/* Top of thread's initial stack frame. */
	int	cur;	/* Recursion depth. */
	int	max;	/* Maximum recursion depth. */
};

void *
recurse(void *args)
{
	int		top;
	struct args	*parms = (struct args *)args;
	char		filler[FRAME_SIZE - FRAME_OVERHEAD];

	/* Touch the memory in this stack frame. */
	top = 0xa5;
	memset(filler, 0xa5, sizeof(filler));

	if (parms->top == NULL) {
		/* Initial stack frame. */
		parms->top = (void*)&top;
	}

	/*
	 * Make sure frame size is what we expect.  Getting this right involves
	 * hand tweaking, so just print a warning rather than aborting.
	 */
	if (parms->top - (void *)&top != FRAME_SIZE * parms->cur) {
		fprintf(stderr,
		    "Stack size (%ld) != expected (%ld), frame %ld\n",
		    (long)parms->top - (long)&top,
		    (long)(FRAME_SIZE * parms->cur), (long)parms->cur);
	}

	parms->cur++;
	if (parms->cur < parms->max)
		recurse(args);

	return NULL;
}


int
main(int argc, char **argv)
{
	size_t		def_stacksize, def_guardsize;
	size_t		stacksize, guardsize;
	pthread_t	thread;
	pthread_attr_t	attr;
	struct args	args;

	if (argc != 3) {
		fprintf(stderr, "usage: guard_b <stacksize> <guardsize>\n");
		exit(1);
	}
	fprintf(stderr, "Test begin\n");

	stacksize = strtoul(argv[1], NULL, 10);
	guardsize = strtoul(argv[2], NULL, 10);

	assert(pthread_attr_init(&attr) == 0);
	/*
	 * Exercise the attribute APIs more thoroughly than is strictly
	 * necessary for the meat of this test program.
	 */
	assert(pthread_attr_getstacksize(&attr, &def_stacksize) == 0);
	assert(pthread_attr_getguardsize(&attr, &def_guardsize) == 0);
	if (def_stacksize != stacksize) {
		assert(pthread_attr_setstacksize(&attr, stacksize) == 0);
		assert(pthread_attr_getstacksize(&attr, &def_stacksize) == 0);
		assert(def_stacksize == stacksize);
	}
	if (def_guardsize != guardsize) {
		assert(pthread_attr_setguardsize(&attr, guardsize) == 0);
		assert(pthread_attr_getguardsize(&attr, &def_guardsize) == 0);
		assert(def_guardsize >= guardsize);
	}

	/*
	 * Create a thread that will come just short of overflowing the thread
	 * stack.  We need to leave a bit of breathing room in case the thread
	 * is context switched, and we also have to take care not to call any
	 * functions in the deepest stack frame.
	 */
	args.top = NULL;
	args.cur = 0;
	args.max = (stacksize / FRAME_SIZE) - 1;
	fprintf(stderr, "No overflow:\n");
	assert(pthread_create(&thread, &attr, recurse, &args) == 0);
	assert(pthread_join(thread, NULL) == 0);
	
	/*
	 * Create a thread that will barely of overflow the thread stack.  This
	 * should cause a segfault.
	 */
	args.top = NULL;
	args.cur = 0;
	args.max = (stacksize / FRAME_SIZE) + 1;
	fprintf(stderr, "Overflow:\n");
	assert(pthread_create(&thread, &attr, recurse, &args) == 0);
	assert(pthread_join(thread, NULL) == 0);

	/* Not reached. */
	fprintf(stderr, "Unexpected success\n");
	abort();

	return 0;
}