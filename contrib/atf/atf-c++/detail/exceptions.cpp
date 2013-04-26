
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

#if defined(HAVE_CONFIG_H)
#include "bconfig.h"
#endif

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <new>

extern "C" {
#include "../../atf-c/error.h"
};

#include "exceptions.hpp"
#include "sanity.hpp"

// ------------------------------------------------------------------------
// The "system_error" type.
// ------------------------------------------------------------------------

atf::system_error::system_error(const std::string& who,
                                const std::string& message,
                                int sys_err) :
    std::runtime_error(who + ": " + message),
    m_sys_err(sys_err)
{
}

atf::system_error::~system_error(void)
    throw()
{
}

int
atf::system_error::code(void)
    const
    throw()
{
    return m_sys_err;
}

const char*
atf::system_error::what(void)
    const
    throw()
{
    try {
        if (m_message.length() == 0) {
            m_message = std::string(std::runtime_error::what()) + ": ";
            m_message += ::strerror(m_sys_err);
        }

        return m_message.c_str();
    } catch (...) {
        return "Unable to format system_error message";
    }
}

// ------------------------------------------------------------------------
// Free functions.
// ------------------------------------------------------------------------

static
void
throw_libc_error(atf_error_t err)
{
    PRE(atf_error_is(err, "libc"));

    const int ecode = atf_libc_error_code(err);
    const std::string msg = atf_libc_error_msg(err);
    atf_error_free(err);
    throw atf::system_error("XXX", msg, ecode);
}

static
void
throw_no_memory_error(atf_error_t err)
{
    PRE(atf_error_is(err, "no_memory"));

    atf_error_free(err);
    throw std::bad_alloc();
}

static
void
throw_unknown_error(atf_error_t err)
{
    PRE(atf_is_error(err));

    static char buf[4096];
    atf_error_format(err, buf, sizeof(buf));
    atf_error_free(err);
    throw std::runtime_error(buf);
}

void
atf::throw_atf_error(atf_error_t err)
{
    static struct handler {
        const char* m_name;
        void (*m_func)(atf_error_t);
    } handlers[] = {
        { "libc", throw_libc_error },
        { "no_memory", throw_no_memory_error },
        { NULL, throw_unknown_error },
    };

    PRE(atf_is_error(err));

    handler* h = handlers;
    while (h->m_name != NULL) {
        if (atf_error_is(err, h->m_name)) {
            h->m_func(err);
            UNREACHABLE;
        } else
            h++;
    }
    // XXX: I'm not sure that raising an "unknown" error is a wise thing
    // to do here.  The C++ binding is supposed to have feature parity
    // with the C one, so all possible errors raised by the C library
    // should have their counterpart in the C++ library.  Still, removing
    // this will require some code auditing that I can't afford at the
    // moment.
    INV(h->m_name == NULL && h->m_func != NULL);
    h->m_func(err);
    UNREACHABLE;
}