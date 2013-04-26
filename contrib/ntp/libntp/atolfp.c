
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
 * atolfp - convert an ascii string to an l_fp number
 */#include <stdio.h>
#include <ctype.h>

#include "ntp_fp.h"
#include "ntp_string.h"

/*
 * Powers of 10
 */
static u_long ten_to_the_n[10] = {
	0,
	10,
	100,
	1000,
	10000,
	100000,
	1000000,
	10000000,
	100000000,
	1000000000,
};


int
atolfp(
	const char *str,
	l_fp *lfp
	)
{
	register const char *cp;
	register u_long dec_i;
	register u_long dec_f;
	char *ind;
	int ndec;
	int isneg;
	static const char *digits = "0123456789";

	isneg = 0;
	dec_i = dec_f = 0;
	ndec = 0;
	cp = str;

	/*
	 * We understand numbers of the form:
	 *
	 * [spaces][-|+][digits][.][digits][spaces|\n|\0]
	 */
	while (isspace((int)*cp))
	    cp++;
	
	if (*cp == '-') {
		cp++;
		isneg = 1;
	}
	
	if (*cp == '+')
	    cp++;

	if (*cp != '.' && !isdigit((int)*cp))
	    return 0;

	while (*cp != '\0' && (ind = strchr(digits, *cp)) != NULL) {
		dec_i = (dec_i << 3) + (dec_i << 1);	/* multiply by 10 */
		dec_i += (ind - digits);
		cp++;
	}

	if (*cp != '\0' && !isspace((int)*cp)) {
		if (*cp++ != '.')
		    return 0;
	
		while (ndec < 9 && *cp != '\0'
		       && (ind = strchr(digits, *cp)) != NULL) {
			ndec++;
			dec_f = (dec_f << 3) + (dec_f << 1);	/* *10 */
			dec_f += (ind - digits);
			cp++;
		}

		while (isdigit((int)*cp))
		    cp++;
		
		if (*cp != '\0' && !isspace((int)*cp))
		    return 0;
	}

	if (ndec > 0) {
		register u_long tmp;
		register u_long bit;
		register u_long ten_fact;

		ten_fact = ten_to_the_n[ndec];

		tmp = 0;
		bit = 0x80000000;
		while (bit != 0) {
			dec_f <<= 1;
			if (dec_f >= ten_fact) {
				tmp |= bit;
				dec_f -= ten_fact;
			}
			bit >>= 1;
		}
		if ((dec_f << 1) > ten_fact)
		    tmp++;
		dec_f = tmp;
	}

	if (isneg)
	    M_NEG(dec_i, dec_f);
	
	lfp->l_ui = dec_i;
	lfp->l_uf = dec_f;
	return 1;
}