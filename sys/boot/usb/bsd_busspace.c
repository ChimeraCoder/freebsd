
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

#include <bsd_kernel.h>

struct burst {
	uint32_t dw0;
	uint32_t dw1;
	uint32_t dw2;
	uint32_t dw3;
	uint32_t dw4;
	uint32_t dw5;
	uint32_t dw6;
	uint32_t dw7;
};

void
bus_space_read_multi_1(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t offset, uint8_t *datap, bus_size_t count)
{
	while (count--) {
		*datap++ = bus_space_read_1(t, h, offset);
	}
}

void
bus_space_read_multi_2(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t offset, uint16_t *datap, bus_size_t count)
{
	while (count--) {
		*datap++ = bus_space_read_2(t, h, offset);
	}
}

void
bus_space_read_multi_4(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t offset, uint32_t *datap, bus_size_t count)
{
	h += offset;

	while (count--) {
		*datap++ = *((volatile uint32_t *)h);
	}
}

void
bus_space_write_multi_1(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t offset, uint8_t *datap, bus_size_t count)
{
	while (count--) {
		uint8_t temp = *datap++;

		bus_space_write_1(t, h, offset, temp);
	}
}

void
bus_space_write_multi_2(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t offset, uint16_t *datap, bus_size_t count)
{
	while (count--) {
		uint16_t temp = *datap++;

		bus_space_write_2(t, h, offset, temp);
	}
}

void
bus_space_write_multi_4(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t offset, uint32_t *datap, bus_size_t count)
{
	h += offset;

	while (count--) {
		*((volatile uint32_t *)h) = *datap++;
	}
}

void
bus_space_write_1(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t offset, uint8_t data)
{
	*((volatile uint8_t *)(h + offset)) = data;
}

void
bus_space_write_2(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t offset, uint16_t data)
{
	*((volatile uint16_t *)(h + offset)) = data;
}

void
bus_space_write_4(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t offset, uint32_t data)
{
	*((volatile uint32_t *)(h + offset)) = data;
}

uint8_t
bus_space_read_1(bus_space_tag_t t, bus_space_handle_t h, bus_size_t offset)
{
	return (*((volatile uint8_t *)(h + offset)));
}

uint16_t
bus_space_read_2(bus_space_tag_t t, bus_space_handle_t h, bus_size_t offset)
{
	return (*((volatile uint16_t *)(h + offset)));
}

uint32_t
bus_space_read_4(bus_space_tag_t t, bus_space_handle_t h, bus_size_t offset)
{
	return (*((volatile uint32_t *)(h + offset)));
}

void
bus_space_read_region_1(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t offset, uint8_t *datap, bus_size_t count)
{
	h += offset;

	while (count--) {
		*datap++ = *((volatile uint8_t *)h);
		h += 1;
	}
}

void
bus_space_write_region_1(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t offset, uint8_t *datap, bus_size_t count)
{
	h += offset;

	while (count--) {
		*((volatile uint8_t *)h) = *datap++;
		h += 1;
	}
}

void
bus_space_read_region_4(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t offset, uint32_t *datap, bus_size_t count)
{
	enum { BURST = sizeof(struct burst) / 4 };

	h += offset;

	while (count >= BURST) {
		*(struct burst *)datap = *((/* volatile */ struct burst *)h);

		h += BURST * 4;
		datap += BURST;
		count -= BURST;
	}

	while (count--) {
		*datap++ = *((volatile uint32_t *)h);
		h += 4;
	}
}

void
bus_space_write_region_4(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t offset, uint32_t *datap, bus_size_t count)
{
	enum { BURST = sizeof(struct burst) / 4 };

	h += offset;

	while (count >= BURST) {
		*((/* volatile */ struct burst *)h) = *(struct burst *)datap;

		h += BURST * 4;
		datap += BURST;
		count -= BURST;
	}

	while (count--) {
		*((volatile uint32_t *)h) = *datap++;
		h += 4;
	}
}