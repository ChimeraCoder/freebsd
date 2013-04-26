
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

#define _LIBCPP_BUILDING_MUTEX
#include "mutex"
#include "limits"
#include "system_error"
#include "cassert"

_LIBCPP_BEGIN_NAMESPACE_STD

const defer_lock_t  defer_lock = {};
const try_to_lock_t try_to_lock = {};
const adopt_lock_t  adopt_lock = {};

mutex::~mutex()
{
    pthread_mutex_destroy(&__m_);
}

void
mutex::lock()
{
    int ec = pthread_mutex_lock(&__m_);
    if (ec)
        __throw_system_error(ec, "mutex lock failed");
}

bool
mutex::try_lock() _NOEXCEPT
{
    return pthread_mutex_trylock(&__m_) == 0;
}

void
mutex::unlock() _NOEXCEPT
{
    int ec = pthread_mutex_unlock(&__m_);
    assert(ec == 0);
}

// recursive_mutex

recursive_mutex::recursive_mutex()
{
    pthread_mutexattr_t attr;
    int ec = pthread_mutexattr_init(&attr);
    if (ec)
        goto fail;
    ec = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    if (ec)
    {
        pthread_mutexattr_destroy(&attr);
        goto fail;
    }
    ec = pthread_mutex_init(&__m_, &attr);
    if (ec)
    {
        pthread_mutexattr_destroy(&attr);
        goto fail;
    }
    ec = pthread_mutexattr_destroy(&attr);
    if (ec)
    {
        pthread_mutex_destroy(&__m_);
        goto fail;
    }
    return;
fail:
    __throw_system_error(ec, "recursive_mutex constructor failed");
}

recursive_mutex::~recursive_mutex()
{
    int e = pthread_mutex_destroy(&__m_);
    assert(e == 0);
}

void
recursive_mutex::lock()
{
    int ec = pthread_mutex_lock(&__m_);
    if (ec)
        __throw_system_error(ec, "recursive_mutex lock failed");
}

void
recursive_mutex::unlock() _NOEXCEPT
{
    int e = pthread_mutex_unlock(&__m_);
    assert(e == 0);
}

bool
recursive_mutex::try_lock() _NOEXCEPT
{
    return pthread_mutex_trylock(&__m_) == 0;
}

// timed_mutex

timed_mutex::timed_mutex()
    : __locked_(false)
{
}

timed_mutex::~timed_mutex()
{
    lock_guard<mutex> _(__m_);
}

void
timed_mutex::lock()
{
    unique_lock<mutex> lk(__m_);
    while (__locked_)
        __cv_.wait(lk);
    __locked_ = true;
}

bool
timed_mutex::try_lock() _NOEXCEPT
{
    unique_lock<mutex> lk(__m_, try_to_lock);
    if (lk.owns_lock() && !__locked_)
    {
        __locked_ = true;
        return true;
    }
    return false;
}

void
timed_mutex::unlock() _NOEXCEPT
{
    lock_guard<mutex> _(__m_);
    __locked_ = false;
    __cv_.notify_one();
}

// recursive_timed_mutex

recursive_timed_mutex::recursive_timed_mutex()
    : __count_(0),
      __id_(0)
{
}

recursive_timed_mutex::~recursive_timed_mutex()
{
    lock_guard<mutex> _(__m_);
}

void
recursive_timed_mutex::lock()
{
    pthread_t id = pthread_self();
    unique_lock<mutex> lk(__m_);
    if (pthread_equal(id, __id_))
    {
        if (__count_ == numeric_limits<size_t>::max())
            __throw_system_error(EAGAIN, "recursive_timed_mutex lock limit reached");
        ++__count_;
        return;
    }
    while (__count_ != 0)
        __cv_.wait(lk);
    __count_ = 1;
    __id_ = id;
}

bool
recursive_timed_mutex::try_lock() _NOEXCEPT
{
    pthread_t id = pthread_self();
    unique_lock<mutex> lk(__m_, try_to_lock);
    if (lk.owns_lock() && (__count_ == 0 || pthread_equal(id, __id_)))
    {
        if (__count_ == numeric_limits<size_t>::max())
            return false;
        ++__count_;
        __id_ = id;
        return true;
    }
    return false;
}

void
recursive_timed_mutex::unlock() _NOEXCEPT
{
    unique_lock<mutex> lk(__m_);
    if (--__count_ == 0)
    {
        __id_ = 0;
        lk.unlock();
        __cv_.notify_one();
    }
}

// If dispatch_once_f ever handles C++ exceptions, and if one can get to it
// without illegal macros (unexpected macros not beginning with _UpperCase or
// __lowercase), and if it stops spinning waiting threads, then call_once should
// call into dispatch_once_f instead of here. Relevant radar this code needs to
// keep in sync with:  7741191.

static pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  cv  = PTHREAD_COND_INITIALIZER;

void
__call_once(volatile unsigned long& flag, void* arg, void(*func)(void*))
{
    pthread_mutex_lock(&mut);
    while (flag == 1)
        pthread_cond_wait(&cv, &mut);
    if (flag == 0)
    {
#ifndef _LIBCPP_NO_EXCEPTIONS
        try
        {
#endif  // _LIBCPP_NO_EXCEPTIONS
            flag = 1;
            pthread_mutex_unlock(&mut);
            func(arg);
            pthread_mutex_lock(&mut);
            flag = ~0ul;
            pthread_mutex_unlock(&mut);
            pthread_cond_broadcast(&cv);
#ifndef _LIBCPP_NO_EXCEPTIONS
        }
        catch (...)
        {
            pthread_mutex_lock(&mut);
            flag = 0ul;
            pthread_mutex_unlock(&mut);
            pthread_cond_broadcast(&cv);
            throw;
        }
#endif  // _LIBCPP_NO_EXCEPTIONS
    }
    else
        pthread_mutex_unlock(&mut);
}

_LIBCPP_END_NAMESPACE_STD