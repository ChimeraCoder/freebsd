
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
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "if_wtapioctl.h"

static int dev = -1;

static void create(int id)
{
    if(ioctl(dev, WTAPIOCTLCRT, &id) < 0){
	printf("error creating wtap with id=%d\n", id);
    }
}

static void delete(int id)
{
    if(ioctl(dev, WTAPIOCTLDEL, &id) < 0){
	printf("error deleting wtap with id=%d\n", id);
    }
}

int main( int argc, const char* argv[])
{
    if(argc != 3){
      printf("usage: %s [c | d] wtap_id\n", argv[0]);
      return -1;
    }
    int id = atoi(argv[2]);
    if(!(id >= 0 && id < 64)){
	printf("wtap_id must be between 0 and 7\n");
	return -1;
    }
    dev = open("/dev/wtapctl", O_RDONLY);
    if(dev < 0){
      printf("error opening wtapctl cdev\n");
      return -1;
    }
    switch((char)*argv[1]){
      case 'c':
	create(id);
	break;
      case 'd':
	delete(id);
	break;
      default:
	printf("wtap ioctl: unkown command '%c'\n", *argv[1]);
	return -1;
    }
    return 0;
}