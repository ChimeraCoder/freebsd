
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <string.h>
#include "windlocl.h"

static const unsigned base         = 36;
static const unsigned t_min        = 1;
static const unsigned t_max        = 26;
static const unsigned skew         = 38;
static const unsigned damp         = 700;
static const unsigned initial_n    = 128;
static const unsigned initial_bias = 72;

static unsigned
digit(unsigned n)
{
    return "abcdefghijklmnopqrstuvwxyz0123456789"[n];
}

static unsigned
adapt(unsigned delta, unsigned numpoints, int first)
{
    unsigned k;

    if (first)
	delta = delta / damp;
    else
	delta /= 2;
    delta += delta / numpoints;
    k = 0;
    while (delta > ((base - t_min) * t_max) / 2) {
	delta /= base - t_min;
	k += base;
    }
    return k + (((base - t_min + 1) * delta) / (delta + skew));
}

/**
 * Convert an UCS4 string to a puny-coded DNS label string suitable
 * when combined with delimiters and other labels for DNS lookup.
 *
 * @param in an UCS4 string to convert
 * @param in_len the length of in.
 * @param out the resulting puny-coded string. The string is not NUL
 * terminatied.
 * @param out_len before processing out_len should be the length of
 * the out variable, after processing it will be the length of the out
 * string.
 *
 * @return returns 0 on success, an wind error code otherwise
 * @ingroup wind
 */

int
wind_punycode_label_toascii(const uint32_t *in, size_t in_len,
			    char *out, size_t *out_len)
{
    unsigned n     = initial_n;
    unsigned delta = 0;
    unsigned bias  = initial_bias;
    unsigned h = 0;
    unsigned b;
    unsigned i;
    unsigned o = 0;
    unsigned m;

    for (i = 0; i < in_len; ++i) {
	if (in[i] < 0x80) {
	    ++h;
	    if (o >= *out_len)
		return WIND_ERR_OVERRUN;
	    out[o++] = in[i];
	}
    }
    b = h;
    if (b > 0) {
	if (o >= *out_len)
	    return WIND_ERR_OVERRUN;
	out[o++] = 0x2D;
    }
    /* is this string punycoded */
    if (h < in_len) {
	if (o + 4 >= *out_len)
	    return WIND_ERR_OVERRUN;
	memmove(out + 4, out, o);
	memcpy(out, "xn--", 4);
	o += 4;
    }

    while (h < in_len) {
	m = (unsigned)-1;
	for (i = 0; i < in_len; ++i)
	    if(in[i] < m && in[i] >= n)
		m = in[i];

	delta += (m - n) * (h + 1);
	n = m;
	for (i = 0; i < in_len; ++i) {
	    if (in[i] < n) {
		++delta;
	    } else if (in[i] == n) {
		unsigned q = delta;
		unsigned k;
		for (k = base; ; k += base) {
		    unsigned t;
		    if (k <= bias)
			t = t_min;
		    else if (k >= bias + t_max)
			t = t_max;
		    else
			t = k - bias;
		    if (q < t)
			break;
		    if (o >= *out_len)
			return WIND_ERR_OVERRUN;
		    out[o++] = digit(t + ((q - t) % (base - t)));
		    q = (q - t) / (base - t);
		}
		if (o >= *out_len)
		    return WIND_ERR_OVERRUN;
		out[o++] = digit(q);
		/* output */
		bias = adapt(delta, h + 1, h == b);
		delta = 0;
		++h;
	    }
	}
	++delta;
	++n;
    }

    *out_len = o;
    return 0;
}