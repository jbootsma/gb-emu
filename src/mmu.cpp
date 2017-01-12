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

#include "mmu.hpp"
#include <iostream>

#include "interrupt_controller.hpp"

std::uint8_t MMU::read_mem(std::uint16_t adr)
{
    switch (adr)
    {
    case IF_ADR:
        return ic->getIF();
    case IE_ADR:
        return ic->getIE();
    default:
        return mem[adr];
    }
}

void MMU::write_mem(std::uint16_t adr, std::uint8_t val)
{
    switch (adr)
    {
    case 0xFF01:
        std::cout << (char)val;
        mem[adr] = val;
        break;
    case IF_ADR:
        ic->setIF(val);
        break;
    case IE_ADR:
        ic->setIE(val);
        break;
    default:
        mem[adr] = val;
        break;
    }
}
