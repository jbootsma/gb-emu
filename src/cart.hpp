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

#ifndef CART_HPP
#define CART_HPP

#include <cstdint>
#include <iosfwd>
#include <vector>

class Cart
{
public:
    void loadCart(std::istream &cartFile);

    std::uint8_t readROM(std::uint16_t adr);
    void writeROM(std::uint16_t adr, std::uint8_t val);

    std::uint8_t readRAM(std::uint16_t adr);
    void writeRAM(std::uint16_t adr, std::uint8_t val);

private:
    std::vector<uint8_t> rom;
    std::vector<uint8_t> ram;
};

#endif
