/*
Copyright (C) 2017 James Bootsma <jrbootsma@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef GPU_HPP
#define GPU_HPP

#include <array>
#include <cstdint>

class GPU
{
public:
    std::uint8_t readVRAM(std::uint16_t adr);
    void writeVRAM(std::uint16_t adr, std::uint8_t val);

    std::uint8_t readOAM(std::uint16_t adr);
    void writeOAM(std::uint16_t adr, std::uint8_t val);

private:
    std::array<std::uint8_t, 0x2000> vram;
    std::array<std::uint8_t, 0xa0> oam;
};

#endif
