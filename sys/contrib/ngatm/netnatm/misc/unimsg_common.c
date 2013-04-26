
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

#include <netnatm/unimsg.h>

/*
 * Make sure there is enough space in front of the data for
 * len bytes, and update the read pointer.
 */
int
uni_msg_prepend(struct uni_msg *msg, size_t len)
{
	size_t need;

	if (uni_msg_leading(msg) >= len) {
		msg->b_rptr -= len;
		return (0);
	}
	need = len - uni_msg_leading(msg);
	if (uni_msg_ensure(msg, need))
		return (-1);
	memcpy(msg->b_rptr + need, msg->b_rptr, uni_msg_len(msg));
	msg->b_rptr += need - len;
	msg->b_wptr += need;
	return (0);
}