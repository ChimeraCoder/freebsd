
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

#include <string.h>
#include "ui_locl.h"

int UI_UTIL_read_pw_string(char *buf,int length,const char *prompt,int verify)
	{
	char buff[BUFSIZ];
	int ret;

	ret=UI_UTIL_read_pw(buf,buff,(length>BUFSIZ)?BUFSIZ:length,prompt,verify);
	OPENSSL_cleanse(buff,BUFSIZ);
	return(ret);
	}

int UI_UTIL_read_pw(char *buf,char *buff,int size,const char *prompt,int verify)
	{
	int ok = 0;
	UI *ui;

	if (size < 1)
		return -1;

	ui = UI_new();
	if (ui)
		{
		ok = UI_add_input_string(ui,prompt,0,buf,0,size-1);
		if (ok >= 0 && verify)
			ok = UI_add_verify_string(ui,prompt,0,buff,0,size-1,
				buf);
		if (ok >= 0)
			ok=UI_process(ui);
		UI_free(ui);
		}
	if (ok > 0)
		ok = 0;
	return(ok);
	}