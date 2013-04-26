
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
#include <dlfcn.h>

int main(int argc, char **argv)
{
    void *handle;
    void (*__gl_tls_test)(void);
    const char *error;

    handle = dlopen("libtls-test.so.1", RTLD_NOW);
    if (!handle) {
        error = dlerror();
        printf("dlopen failed (%s)!\n", error);
        exit(1);
    }

    dlerror();
    __gl_tls_test = dlsym(handle, "__gl_tls_test");
    error = dlerror();

    if (error) {
        dlclose(handle);
        printf("dlsym failed (%s)!\n", error);
        exit(1);
    }

    __gl_tls_test(); /* print TLS values */
    dlclose(handle);

    return 0;
}