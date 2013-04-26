
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

extern "C" {
#include <signal.h>
#include <unistd.h>
}

#include <cerrno>

#include "atf-c++/detail/exceptions.hpp"
#include "atf-c++/detail/sanity.hpp"

#include "signals.hpp"

namespace impl = atf::atf_run;
#define IMPL_NAME "atf::atf_run"

const int impl::last_signo = LAST_SIGNO;

// ------------------------------------------------------------------------
// The "signal_holder" class.
// ------------------------------------------------------------------------

namespace {

static bool happened[LAST_SIGNO + 1];

static
void
holder_handler(const int signo)
{
    happened[signo] = true;
}

} // anonymous namespace

impl::signal_holder::signal_holder(const int signo) :
    m_signo(signo),
    m_sp(NULL)
{
    happened[signo] = false;
    m_sp = new signal_programmer(m_signo, holder_handler);
}

impl::signal_holder::~signal_holder(void)
{
    if (m_sp != NULL)
        delete m_sp;

    if (happened[m_signo])
        ::kill(::getpid(), m_signo);
}

void
impl::signal_holder::process(void)
{
    if (happened[m_signo]) {
        delete m_sp;
        m_sp = NULL;
        happened[m_signo] = false;
        ::kill(::getpid(), m_signo);
        m_sp = new signal_programmer(m_signo, holder_handler);
    }
}

// ------------------------------------------------------------------------
// The "signal_programmer" class.
// ------------------------------------------------------------------------

impl::signal_programmer::signal_programmer(const int signo, const handler h) :
    m_signo(signo),
    m_handler(h),
    m_programmed(false)
{
    struct ::sigaction sa;

    sa.sa_handler = m_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (::sigaction(m_signo, &sa, &m_oldsa) == -1)
        throw atf::system_error(IMPL_NAME, "Could not install handler for "
                                "signal", errno);
    m_programmed = true;
}

impl::signal_programmer::~signal_programmer(void)
{
    unprogram();
}

void
impl::signal_programmer::unprogram(void)
{
    if (m_programmed) {
        if (::sigaction(m_signo, &m_oldsa, NULL) == -1)
            UNREACHABLE;
        m_programmed = false;
    }
}

// ------------------------------------------------------------------------
// Free functions.
// ------------------------------------------------------------------------

void
impl::reset(const int signo)
{
    struct ::sigaction sa;

    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    (void)::sigaction(signo, &sa, NULL);
}