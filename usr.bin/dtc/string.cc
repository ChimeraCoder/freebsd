
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

#include "string.hh"
#include <ctype.h>
#include <stdio.h>

namespace
{
/**
 * The source files are ASCII, so we provide a non-locale-aware version of
 * isalpha.  This is a class so that it can be used with a template function
 * for parsing strings.
 */
struct is_alpha 
{
	static inline bool check(const char c)
	{
		return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') &&
			(c <= 'Z'));
	}
};
/**
 * Check whether a character is in the set allowed for node names.  This is a
 * class so that it can be used with a template function for parsing strings.
 */
struct is_node_name_character
{
	static inline bool check(const char c)
	{
		switch(c)
		{
			default:
				return false;
			case 'a'...'z': case 'A'...'Z': case '0'...'9':
			case ',': case '.': case '+': case '-':
			case '_':
				return true;
		}
	}
};
/**
 * Check whether a character is in the set allowed for property names.  This is
 * a class so that it can be used with a template function for parsing strings.
 */
struct is_property_name_character
{
	static inline bool check(const char c)
	{
		switch(c)
		{
			default:
				return false;
			case 'a'...'z': case 'A'...'Z': case '0'...'9':
			case ',': case '.': case '+': case '-':
			case '_': case '#':
				return true;
		}
	}
};

}

namespace dtc
{

template<class T> string
string::parse(input_buffer &s)
{
	const char *start = s;
	int l=0;
	while (T::check(*s)) { l++; ++s; }
	return string(start, l);
}

string::string(input_buffer &s) : start((const char*)s), length(0)
{
	while(s[length] != '\0')
	{
		length++;
	}
}

string
string::parse_node_name(input_buffer &s)
{
	return parse<is_node_name_character>(s);
}

string
string::parse_property_name(input_buffer &s)
{
	return parse<is_property_name_character>(s);
}
string
string::parse_node_or_property_name(input_buffer &s, bool &is_property)
{
	if (is_property)
	{
		return parse_property_name(s);
	}
	const char *start = s;
	int l=0;
	while (is_node_name_character::check(*s))
	{
		l++;
		++s;
	}
	while (is_property_name_character::check(*s))
	{
		l++;
		++s;
		is_property = true;
	}
	return string(start, l);
}

bool
string::operator==(const string& other) const
{
	return (length == other.length) &&
	       (memcmp(start, other.start, length) == 0);
}

bool
string::operator==(const char *other) const
{
	return strncmp(other, start, length) == 0;
}

bool
string::operator<(const string& other) const
{
	if (length < other.length) { return true; }
	if (length > other.length) { return false; }
	return memcmp(start, other.start, length) < 0;
}

void
string::push_to_buffer(byte_buffer &buffer, bool escapes)
{
	for (int i=0 ; i<length ; ++i)
	{
		uint8_t c = start[i];
		if (escapes && c == '\\' && i+1 < length)
		{
			c = start[++i];
			switch (c)
			{
				// For now, we just ignore invalid escape sequences.
				default:
				case '"':
				case '\'':
				case '\\':
					break;
				case 'a':
					c = '\a';
					break;
				case 'b':
					c = '\b';
					break;
				case 't':
					c = '\t';
					break;
				case 'n':
					c = '\n';
					break;
				case 'v':
					c = '\v';
					break;
				case 'f':
					c = '\f';
					break;
				case 'r':
					c = '\r';
					break;
				case '0'...'7':
				{
					int v = digittoint(c);
					if (i+1 < length && start[i+1] <= '7' && start[i+1] >= '0')
					{
						v <<= 3;
						v |= digittoint(start[i+1]);
						i++;
						if (i+1 < length && start[i+1] <= '7' && start[i+1] >= '0')
						{
							v <<= 3;
							v |= digittoint(start[i+1]);
						}
					}
					c = (uint8_t)v;
					break;
				}
				case 'x':
				{
					++i;
					if (i >= length)
					{
						break;
					}
					int v = digittoint(start[i]);
					if (i+1 < length && ishexdigit(start[i+1]))
					{
						v <<= 4;
						v |= digittoint(start[++i]);
					}
					c = (uint8_t)v;
					break;
				}
			}
		}
		buffer.push_back(c);
	}
}

void
string::print(FILE *file)
{
	fwrite(start, length, 1, file);
}

void
string::dump()
{
	print(stderr);
}

} // namespace dtc