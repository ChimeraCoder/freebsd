
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

#ifdef HAVE_KERNEL_OPTION_HEADERS
#include "opt_snd.h"
#endif

#include <dev/sound/pcm/sound.h>
#include <dev/sound/pcm/ac97.h>
#include <dev/sound/pcm/ac97_patch.h>

SND_DECLARE_FILE("$FreeBSD$");

void ad1886_patch(struct ac97_info* codec)
{
#define AC97_AD_JACK_SPDIF 0x72
	/*
	 *    Presario700 workaround
	 *     for Jack Sense/SPDIF Register misetting causing
	 *    no audible output
	 *    by Santiago Nullo 04/05/2002
	 */
	ac97_wrcd(codec, AC97_AD_JACK_SPDIF, 0x0010);
}

void ad198x_patch(struct ac97_info* codec)
{
	switch (ac97_getsubvendor(codec)) {
	case 0x11931043:	/* Not for ASUS A9T (probably else too). */
		break;
	default:
		ac97_wrcd(codec, 0x76, ac97_rdcd(codec, 0x76) | 0x0420);
		break;
	}
}

void ad1981b_patch(struct ac97_info* codec)
{
	/*
	 * Enable headphone jack sensing.
	 */
	switch (ac97_getsubvendor(codec)) {
	case 0x02d91014:	/* IBM Thinkcentre */
	case 0x099c103c:	/* HP nx6110 */
		ac97_wrcd(codec, AC97_AD_JACK_SPDIF,
		    ac97_rdcd(codec, AC97_AD_JACK_SPDIF) | 0x0800);
		break;
	default:
		break;
	}
}

void cmi9739_patch(struct ac97_info* codec)
{
	/*
	 * Few laptops need extra register initialization
	 * to power up the internal speakers.
	 */
	switch (ac97_getsubvendor(codec)) {
	case 0x18431043:	/* ASUS W1000N */
		ac97_wrcd(codec, AC97_REG_POWER, 0x000f);
		ac97_wrcd(codec, AC97_MIXEXT_CLFE, 0x0000);
		ac97_wrcd(codec, 0x64, 0x7110);
		break;
	default:
		break;
	}
}

void alc655_patch(struct ac97_info* codec)
{
	/*
	 * MSI (Micro-Star International) specific EAPD quirk.
	 */
	switch (ac97_getsubvendor(codec)) {
	case 0x00611462:	/* MSI S250 */
	case 0x01311462:	/* MSI S270 */
	case 0x01611462:	/* LG K1 Express */
	case 0x03511462:	/* MSI L725 */
		ac97_wrcd(codec, 0x7a, ac97_rdcd(codec, 0x7a) & 0xfffd);
		break;
	case 0x10ca1734:
		/*
		 * Amilo Pro V2055 with ALC655 has phone out by default
		 * disabled (surround on), leaving us only with internal
		 * speakers. This should really go to mixer. We write the
		 * Data Flow Control reg.
		 */
		ac97_wrcd(codec, 0x6a, ac97_rdcd(codec, 0x6a) | 0x0001);
		break;
	default:
		break;
	}
}