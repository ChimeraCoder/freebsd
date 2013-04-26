
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

#include "dtb.hh"
#include <sys/types.h>
#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>


namespace dtc
{
namespace dtb
{

void output_writer::write_data(byte_buffer b)
{
	for (byte_buffer::iterator i=b.begin(), e=b.end(); i!=e ; i++)
	{
		write_data(*i);
	}
}

void
binary_writer::write_string(string name)
{
	name.push_to_buffer(buffer);
	// Trailing nul
	buffer.push_back(0);
}

void
binary_writer::write_data(uint8_t v)
{
	buffer.push_back(v);
}

void
binary_writer::write_data(uint32_t v)
{
	while (buffer.size() % 4 != 0)
	{
		buffer.push_back(0);
	}
	push_big_endian(buffer, v);
}

void
binary_writer::write_data(uint64_t v)
{
	while (buffer.size() % 8 != 0)
	{
		buffer.push_back(0);
	}
	push_big_endian(buffer, v);
}

void
binary_writer::write_to_file(int fd)
{
	// FIXME: Check return
	write(fd, buffer.data(), buffer.size());
}

uint32_t
binary_writer::size()
{
	return buffer.size();
}

void
asm_writer::write_string(const char *c)
{
	while (*c)
	{
		buffer.push_back((uint8_t)*(c++));
	}
}

void
asm_writer::write_line(const char *c)
{
	if (byte_count != 0)
	{
		byte_count = 0;
		buffer.push_back('\n');
	}
	write_string(c);
}

void
asm_writer::write_byte(uint8_t b)
{
	char out[3] = {0};
	if (byte_count++ == 0)
	{
		buffer.push_back('\t');
	}
	write_string(".byte 0x");
	snprintf(out, 3, "%.2hhx", b);
	buffer.push_back(out[0]);
	buffer.push_back(out[1]);
	if (byte_count == 4)
	{
		buffer.push_back('\n');
		byte_count = 0;
	}
	else
	{
		buffer.push_back(';');
		buffer.push_back(' ');
	}
}

void
asm_writer::write_label(string name)
{
	write_line("\t.globl ");
	name.push_to_buffer(buffer);
	buffer.push_back('\n');
	name.push_to_buffer(buffer);
	buffer.push_back(':');
	buffer.push_back('\n');
	buffer.push_back('_');
	name.push_to_buffer(buffer);
	buffer.push_back(':');
	buffer.push_back('\n');
	
}

void
asm_writer::write_comment(string name)
{
	write_line("\t/* ");
	name.push_to_buffer(buffer);
	write_string(" */\n");
}

void
asm_writer::write_string(string name)
{
	write_line("\t.string \"");
	name.push_to_buffer(buffer);
	write_line("\"\n");
	bytes_written += name.size() + 1;
}

void
asm_writer::write_data(uint8_t v)
{
	write_byte(v);
	bytes_written++;
}

void
asm_writer::write_data(uint32_t v)
{
	if (bytes_written % 4 != 0)
	{
		write_line("\t.balign 4\n");
		bytes_written += (4 - (bytes_written % 4));
	}
	write_byte((v >> 24) & 0xff);
	write_byte((v >> 16) & 0xff);
	write_byte((v >> 8) & 0xff);
	write_byte((v >> 0) & 0xff);
	bytes_written += 4;
}

void
asm_writer::write_data(uint64_t v)
{
	if (bytes_written % 8 != 0)
	{
		write_line("\t.balign 8\n");
		bytes_written += (8 - (bytes_written % 8));
	}
	write_byte((v >> 56) & 0xff);
	write_byte((v >> 48) & 0xff);
	write_byte((v >> 40) & 0xff);
	write_byte((v >> 32) & 0xff);
	write_byte((v >> 24) & 0xff);
	write_byte((v >> 16) & 0xff);
	write_byte((v >> 8) & 0xff);
	write_byte((v >> 0) & 0xff);
	bytes_written += 8;
}

void
asm_writer::write_to_file(int fd)
{
	// FIXME: Check return
	write(fd, buffer.data(), buffer.size());
}

uint32_t
asm_writer::size()
{
	return bytes_written;
}

void
header::write(output_writer &out)
{
	out.write_label(string("dt_blob_start"));
	out.write_label(string("dt_header"));
	out.write_comment("magic");
	out.write_data(magic);
	out.write_comment("totalsize");
	out.write_data(totalsize);
	out.write_comment("off_dt_struct");
	out.write_data(off_dt_struct);
	out.write_comment("off_dt_strings");
	out.write_data(off_dt_strings);
	out.write_comment("off_mem_rsvmap");
	out.write_data(off_mem_rsvmap);
	out.write_comment("version");
	out.write_data(version);
	out.write_comment("last_comp_version");
	out.write_data(last_comp_version);
	out.write_comment("boot_cpuid_phys");
	out.write_data(boot_cpuid_phys);
	out.write_comment("size_dt_strings");
	out.write_data(size_dt_strings);
	out.write_comment("size_dt_struct");
	out.write_data(size_dt_struct);
}

bool
header::read_dtb(input_buffer &input)
{
	if (!(input.consume_binary(magic) && magic == 0xd00dfeed))
	{
		fprintf(stderr, "Missing magic token in header.  Got %" PRIx32
		                " expected 0xd00dfeed\n", magic);
		return false;
	}
	return input.consume_binary(totalsize) &&
	       input.consume_binary(off_dt_struct) &&
	       input.consume_binary(off_dt_strings) &&
	       input.consume_binary(off_mem_rsvmap) &&
	       input.consume_binary(version) &&
	       input.consume_binary(last_comp_version) &&
	       input.consume_binary(boot_cpuid_phys) &&
	       input.consume_binary(size_dt_strings) &&
	       input.consume_binary(size_dt_struct);
}
uint32_t
string_table::add_string(string str)
{
	std::map<string, uint32_t>::iterator old = string_offsets.find(str);
	if (old == string_offsets.end())
	{
		uint32_t start = size;
		// Don't forget the trailing nul
		size += str.size() + 1;
		string_offsets.insert(std::make_pair(str, start));
		strings.push_back(str);
		return start;
	}
	else
	{
		return old->second;
	}
}

void
string_table::write(dtb::output_writer &writer)
{
	writer.write_comment(string("Strings table."));
	writer.write_label(string("dt_strings_start"));
	for (std::vector<string>::iterator i=strings.begin(), e=strings.end() ;
	     i!=e ; ++i)
	{
		writer.write_string(*i);
	}
	writer.write_label(string("dt_strings_end"));
}

} // namespace dtb

} // namespace dtc