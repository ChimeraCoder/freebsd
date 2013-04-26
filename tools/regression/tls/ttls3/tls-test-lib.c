
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

#define __G_TLS_OFFSETS_SIZE    8
unsigned long int __gl_tls_offsets[__G_TLS_OFFSETS_SIZE];

void __gl_tls_init_offsets();

#ifdef __GL_TLS_SINGLE_INSTRUCTION
#define THREAD_GETMEM(num)                  \
({                                          \
    void *__value;                          \
    __asm__ __volatile__ (                  \
        "movl %%gs:(%1),%0"                 \
        : "=r" (__value)                    \
        : "r" (__gl_tls_offsets[num])       \
    );                                      \
    __value;                                \
})

#define THREAD_SETMEM(num, value)           \
do {                                        \
    void *__value = (value);                \
    __asm__ __volatile__ (                  \
        "movl %0,%%gs:(%1)"                 \
        :                                   \
        : "r" (__value),                    \
          "r" (__gl_tls_offsets[num])       \
    );                                      \
} while (0)
#else
#define __GL_TLS_GET(num)                   \
({                                          \
    void *__dummy, *__value;                \
    __asm__ __volatile__ (                  \
        "movl %%gs:0,%2     \n\t"           \
        "movl (%2,%1),%0    \n\t"           \
        : "=r" (__value)                    \
        : "r" (__gl_tls_offsets[num]),      \
          "r" (__dummy)                     \
    );                                      \
    __value;                                \
})

#define __GL_TLS_SET(num, value)            \
do {                                        \
    void *__dummy, *__value = (value);      \
    __asm__ __volatile__ (                  \
        "movl %%gs:0,%2     \n\t"           \
        "movl %0,(%2,%1)    \n\t"           \
        :                                   \
        : "r" (__value),                    \
          "r" (__gl_tls_offsets[num]),      \
          "r" (__dummy)                     \
    );                                      \
} while (0)
#endif

void _init(void)
{
    __gl_tls_init_offsets();

    __GL_TLS_SET(0, (void *) 0xff000000);
    __GL_TLS_SET(1, (void *) 0xff000001);
    __GL_TLS_SET(2, (void *) 0xff000002);
    __GL_TLS_SET(3, (void *) 0xff000003);
    __GL_TLS_SET(4, (void *) 0xff000004);
    __GL_TLS_SET(5, (void *) 0xff000005);
    __GL_TLS_SET(6, (void *) 0xff000006);
    __GL_TLS_SET(7, (void *) 0xff000007);
}

void __gl_tls_test(void)
{
    printf("__GL_TLS_GET(0) = %p\n", __GL_TLS_GET(0));
    printf("__GL_TLS_GET(1) = %p\n", __GL_TLS_GET(1));
    printf("__GL_TLS_GET(2) = %p\n", __GL_TLS_GET(2));
    printf("__GL_TLS_GET(3) = %p\n", __GL_TLS_GET(3));
    printf("__GL_TLS_GET(4) = %p\n", __GL_TLS_GET(4));
    printf("__GL_TLS_GET(5) = %p\n", __GL_TLS_GET(5));
    printf("__GL_TLS_GET(6) = %p\n", __GL_TLS_GET(6));
    printf("__GL_TLS_GET(7) = %p\n", __GL_TLS_GET(7));
}