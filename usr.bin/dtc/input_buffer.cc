
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

#include "input_buffer.hh"
#include <ctype.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <sys/stat.h>
#include <sys/mman.h>
#include <assert.h>

#ifndef MAP_PREFAULT_READ
#define MAP_PREFAULT_READ 0
#endif

namespace dtc
{

void
input_buffer::skip_spaces()
{
	if (cursor >= size) { return; }
	if (cursor < 0) { return; }
	char c = buffer[cursor];
	while ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\f')
	       || (c == '\v') || (c == '\r'))
	{
		cursor++;
		if (cursor > size)
		{
			c = '\0';
		}
		else
		{
			c = buffer[cursor];
		}
	}
}

input_buffer
input_buffer::buffer_from_offset(int offset, int s)
{
	if (s == 0)
	{
		s = size - offset;
	}
	if (offset > size)
	{
		return input_buffer();
	}
	if (s > (size-offset))
	{
		return input_buffer();
	}
	return input_buffer(&buffer[offset], s);
}

bool
input_buffer::consume(const char *str)
{
	int len = strlen(str);
	if (len > size - cursor)
	{
		return false;
	}
	else
	{
		for (int i=0 ; i<len ; ++i)
		{
			if (str[i] != buffer[cursor + i])
			{
				return false;
			}
		}
		cursor += len;
		return true;
	}
	return false;
}

bool
input_buffer::consume_integer(long long &outInt)
{
	// The first character must be a digit.  Hex and octal strings
	// are prefixed by 0 and 0x, respectively.
	if (!isdigit((*this)[0]))
	{
		return false;
	}
	char *end=0;
	outInt = strtoll(&buffer[cursor], &end, 0);
	if (end == &buffer[cursor])
	{
		return false;
	}
	cursor = end - buffer;
	return true;
}

bool
input_buffer::consume_hex_byte(uint8_t &outByte)
{
	if (!ishexdigit((*this)[0]) && !ishexdigit((*this)[1]))
	{
		return false;
	}
	outByte = (digittoint((*this)[0]) << 4) | digittoint((*this)[1]);
	cursor += 2;
	return true;
}

input_buffer&
input_buffer::next_token()
{
	int start;
	do {
		start = cursor;
		skip_spaces();
		// Parse /* comments
		if (((*this)[0] == '/') && ((*this)[1] == '*'))
		{
			// eat the start of the comment
			++(*this);
			++(*this);
			do {
				// Find the ending * of */
				while ((**this != '\0') && (**this != '*'))
				{
					++(*this);
				}
				// Eat the *
				++(*this);
			} while ((**this != '\0') && (**this != '/'));
			// Eat the /
			++(*this);
		}
		// Parse // comments
		if (((*this)[0] == '/') && ((*this)[1] == '/'))
		{
			// eat the start of the comment
			++(*this);
			++(*this);
			// Find the ending * of */
			while (**this != '\n')
			{
				++(*this);
			}
			// Eat the \n
			++(*this);
		}
	} while (start != cursor);
	return *this;
}

void
input_buffer::parse_error(const char *msg)
{
	int line_count = 1;
	int line_start = 0;
	int line_end = cursor;
	for (int i=cursor ; i>0 ; --i)
	{
		if (buffer[i] == '\n')
		{
			line_count++;
			if (line_start == 0)
			{
				line_start = i+1;
			}
		}
	}
	for (int i=cursor+1 ; i<size ; ++i)
	{
		if (buffer[i] == '\n')
		{
			line_end = i;
			break;
		}
	}
	fprintf(stderr, "Error on line %d: %s\n", line_count, msg);
	fwrite(&buffer[line_start], line_end-line_start, 1, stderr);
	putc('\n', stderr);
	for (int i=0 ; i<(cursor-line_start) ; ++i)
	{
		putc(' ', stderr);
	}
	putc('^', stderr);
	putc('\n', stderr);
}
void
input_buffer::dump()
{
	fprintf(stderr, "Current cursor: %d\n", cursor);
	fwrite(&buffer[cursor], size-cursor, 1, stderr);
}

mmap_input_buffer::mmap_input_buffer(int fd) : input_buffer(0, 0)
{
	struct stat sb;
	if (fstat(fd, &sb))
	{
		perror("Failed to stat file");
	}
	size = sb.st_size;
	buffer = (const char*)mmap(0, size, PROT_READ,
		MAP_PREFAULT_READ, fd, 0);
	if (buffer == 0)
	{
		perror("Failed to mmap file");
	}
}

mmap_input_buffer::~mmap_input_buffer()
{
	if (buffer != 0)
	{
		munmap((void*)buffer, size);
	}
}

stream_input_buffer::stream_input_buffer() : input_buffer(0, 0)
{
	int c;
	while ((c = fgetc(stdin)) != EOF)
	{
		b.push_back(c);
	}
	buffer = b.data();
	size = b.size();
}

} // namespace dtc