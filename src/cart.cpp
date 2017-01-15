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
#include <istream>

#include "cart.hpp"

void Cart::loadCart(std::istream &cartFile)
{
    rom.resize(0x8000);
    cartFile.read((char*)&rom[0], rom.size());

    ram.resize(0x2000);
}

std::uint8_t Cart::readROM(std::uint16_t adr)
{
    if (rom.empty()) return 0xFF;
    return rom.at(adr);
}

void Cart::writeROM(std::uint16_t, std::uint8_t)
{
    return;
}

std::uint8_t Cart::readRAM(std::uint16_t adr)
{
    if (ram.empty()) return 0xFF;
    return ram.at(adr);
}

void Cart::writeRAM(std::uint16_t adr, std::uint8_t val)
{
    if (ram.empty()) return;
    ram.at(adr) = val;
}
