
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

#include <mp.h>
#include <stdio.h>
#include <string.h>
#include <sysexits.h>

MINT *c0, *c1, *c2, *c3, *c5, *c6, *c8, *c10, *c14, *c15, *c25, \
    *c42,*c43, *c44, *c45, *t0, *t1;
static int tnr = 0;

static void
testmcmp(const MINT *mp1, const MINT *mp2, const char *tname)
{

	if (mp_mcmp(mp1, mp2) == 0)
		printf("ok %d - %s\n", ++tnr, tname);
	else
		printf("not ok - %d %s\n", ++tnr, tname);
}

static int
testsimpel(void)
{
	const char str42[] = "2a";
	MINT *t2;
	char *s;

	mp_madd(c42, c1, t0);
	testmcmp(c43, t0, "madd0");
	mp_madd(t0, c1, t0);
	testmcmp(c44, t0, "madd1");
	mp_msub(t0, c1, t0);
	testmcmp(c43, t0, "msub0");
	mp_msub(t0, c1, t0);
	testmcmp(c42, t0, "msub1");
	mp_move(c42, t0);
	testmcmp(c42, t0, "move0");

	t2 = mp_xtom(str42);
	testmcmp(c42, t2, "xtom");
	s = mp_mtox(t2);
	if (strcmp(str42, s) == 0)
		printf("ok %d - %s\n", ++tnr, "mtox0");
	else
		printf("not ok %d - %s\n", ++tnr, "mtox0");
	mp_mfree(t2);
}

static int
testgcd(void)
{

	mp_gcd(c10, c15, t0);
	testmcmp(t0, c5, "gcd0");
}

static int
testmsqrt(void)
{

	mp_msqrt(c25, t0, t1);
	testmcmp(t0, c5, "msqrt0");
	testmcmp(t1, c0, "msqrt1");
	mp_msqrt(c42, t0, t1);
	testmcmp(t0, c6, "msqrt2");
	testmcmp(t1, c6, "msqrt3");
}

static int
testdiv(void)
{
	short ro;
	MINT *t2;

	mp_mdiv(c42, c5, t0, t1);
	testmcmp(t0, c8, "mdiv0");
	testmcmp(t1, c2, "mdiv1");

	mp_mdiv(c10, c8, t0, t1);
	testmcmp(t0, c1, "mdiv2");
	testmcmp(t1, c2, "mdiv3");

	mp_sdiv(c42, 5, t0, &ro);
	testmcmp(t0, c8, "sdiv0");
	t2 = mp_itom(ro); // Simpler to use common testmcmp()
	testmcmp(t2, c2, "sdiv1");
	mp_mfree(t2);

	mp_sdiv(c10, 8, t0, &ro);
	testmcmp(t0, c1, "sdiv2");
	t2 = mp_itom(ro); // Simpler to use common testmcmp()
	testmcmp(t2, c2, "sdiv3");
	mp_mfree(t2);
}

static int
testmult(void)
{

	mp_mult(c5, c2, t0);
	testmcmp(t0, c10, "mmult0");
	mp_mult(c3, c14, t0);
	testmcmp(t0, c42, "mmult1");
}

static int
testpow(void)
{

	mp_pow(c2, c3, c10, t0);
	testmcmp(t0, c8, "pow0");
	mp_pow(c2, c3, c3, t0);
	testmcmp(t0, c2, "pow1");
	mp_rpow(c2, 3, t0);
	testmcmp(t0, c8, "rpow0");
}

/*
 * This program performs some very basic tests of libmp(3).  It is by
 * no means expected to perform a complete test of the library for
 * correctness, but is meant to test the API to make sure libmp (or
 * libcrypto) updates don't totally break the library.
 */
int
main(int argc, char *argv[])
{

	printf("1..25\n");

	/*
	 * Init "constants" variables - done in this somewhat
	 * cumbersome way to in theory be able to check for memory
	 * leaks.
	 */
	c0 = mp_itom(0);
	c1 = mp_itom(1);
	c2 = mp_itom(2);
	c3 = mp_itom(3);
	c5 = mp_itom(5);
	c6 = mp_itom(6);
	c8 = mp_itom(8);
	c10 = mp_itom(10);
	c14 = mp_itom(14);
	c15 = mp_itom(15);
	c25 = mp_itom(25);
	c42 = mp_itom(42);
	c43 = mp_itom(43);
	c44 = mp_itom(44);
	c45 = mp_itom(45);

	// Init temp variables
	t0 = mp_itom(0);
	t1 = mp_itom(0);

	// Run tests
	testsimpel();
	testgcd();
	testdiv();
	testmult();
	testpow();
	testmsqrt();

	// Cleanup
	mp_mfree(c0);
	mp_mfree(c1);
	mp_mfree(c2);
	mp_mfree(c3);
	mp_mfree(c5);
	mp_mfree(c6);
	mp_mfree(c8);
	mp_mfree(c10);
	mp_mfree(c14);
	mp_mfree(c15);
	mp_mfree(c25);
	mp_mfree(c42);
	mp_mfree(c43);
	mp_mfree(c44);
	mp_mfree(c45);
	mp_mfree(t0);
	mp_mfree(t1);

	return (EX_OK);
}