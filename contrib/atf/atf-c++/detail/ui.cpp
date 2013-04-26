
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

extern "C" {
#include <sys/ioctl.h>

#include <termios.h>
#include <unistd.h>
}

#include <sstream>

#include "env.hpp"
#include "text.hpp"
#include "sanity.hpp"
#include "text.hpp"
#include "ui.hpp"

namespace impl = atf::ui;
#define IMPL_NAME "atf::ui"

static
size_t
terminal_width(void)
{
    static bool done = false;
    static size_t width = 0;

    if (!done) {
        if (atf::env::has("COLUMNS")) {
            const std::string cols = atf::env::get("COLUMNS");
            if (cols.length() > 0) {
                width = atf::text::to_type< size_t >(cols);
            }
        } else {
            struct winsize ws;
            if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != -1)
                width = ws.ws_col;
        }

        if (width >= 80)
            width -= 5;

        done = true;
    }

    return width;
}

static
std::string
format_paragraph(const std::string& text,
                 const std::string& tag,
                 const bool first,
                 const bool repeat,
                 const size_t col)
{
    PRE(text.find('\n') == std::string::npos);

    const std::string pad(col - tag.length(), ' ');
    const std::string fullpad(col, ' ');

    std::string formatted;
    if (first || repeat)
        formatted = tag + pad;
    else
        formatted = fullpad;
    INV(formatted.length() == col);
    size_t curcol = col;

    const size_t maxcol = terminal_width();

    std::vector< std::string > words = atf::text::split(text, " ");
    for (std::vector< std::string >::const_iterator iter = words.begin();
         iter != words.end(); iter++) {
        const std::string& word = *iter;

        if (iter != words.begin() && maxcol > 0 &&
            curcol + word.length() + 1 > maxcol) {
            if (repeat)
                formatted += '\n' + tag + pad;
            else
                formatted += '\n' + fullpad;
            curcol = col;
        } else if (iter != words.begin()) {
            formatted += ' ';
            curcol++;
        }

        formatted += word;
        curcol += word.length();
    }

    return formatted;
}

std::string
impl::format_error(const std::string& prog_name, const std::string& error)
{
    return format_text_with_tag("ERROR: " + error, prog_name + ": ", true);
}

std::string
impl::format_info(const std::string& prog_name, const std::string& msg)
{
    return format_text_with_tag(msg, prog_name + ": ", true);
}

std::string
impl::format_text(const std::string& text)
{
    return format_text_with_tag(text, "", false, 0);
}

std::string
impl::format_text_with_tag(const std::string& text, const std::string& tag,
                           bool repeat, size_t col)
{
    PRE(col == 0 || col >= tag.length());
    if (col == 0)
        col = tag.length();

    std::string formatted;

    std::vector< std::string > lines = atf::text::split(text, "\n");
    for (std::vector< std::string >::const_iterator iter = lines.begin();
         iter != lines.end(); iter++) {
        const std::string& line = *iter;

        formatted += format_paragraph(line, tag, iter == lines.begin(),
                                      repeat, col);
        if (iter + 1 != lines.end()) {
            if (repeat)
                formatted += "\n" + tag + "\n";
            else
                formatted += "\n\n";
        }
    }

    return formatted;
}

std::string
impl::format_warning(const std::string& prog_name, const std::string& error)
{
    return format_text_with_tag("WARNING: " + error, prog_name + ": ", true);
}