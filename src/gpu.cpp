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

#include <cstdint>

#include "gpu.hpp"

std::uint8_t GPU::readVRAM(std::uint16_t adr)
{
    return vram.at(adr);
}

void GPU::writeVRAM(std::uint16_t adr, std::uint8_t val)
{
    vram.at(adr) = val;
}

std::uint8_t GPU::readOAM(std::uint16_t adr)
{
    return oam.at(adr);
}

void GPU::writeOAM(std::uint16_t adr, std::uint8_t val)
{
    oam.at(adr) = val;
}
