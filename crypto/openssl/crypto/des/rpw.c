
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

#include <stdio.h>
#include <openssl/des.h>

int main(int argc, char *argv[])
	{
	DES_cblock k,k1;
	int i;

	printf("read passwd\n");
	if ((i=des_read_password(&k,"Enter password:",0)) == 0)
		{
		printf("password = ");
		for (i=0; i<8; i++)
			printf("%02x ",k[i]);
		}
	else
		printf("error %d\n",i);
	printf("\n");
	printf("read 2passwds and verify\n");
	if ((i=des_read_2passwords(&k,&k1,
		"Enter verified password:",1)) == 0)
		{
		printf("password1 = ");
		for (i=0; i<8; i++)
			printf("%02x ",k[i]);
		printf("\n");
		printf("password2 = ");
		for (i=0; i<8; i++)
			printf("%02x ",k1[i]);
		printf("\n");
		exit(1);
		}
	else
		{
		printf("error %d\n",i);
		exit(0);
		}
#ifdef LINT
	return(0);
#endif
	}