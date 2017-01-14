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

#include "interrupt_controller.hpp"

#include <cassert>

std::uint8_t InterruptController::accept_interrupt()
{
    std::uint8_t vec = 0x40;
    std::uint8_t bit = 0x01;

    while (bit)
    {
        if (IE & IF & bit)
        {
            IF &= ~bit;
            return vec;
        }
        bit <<= 1;
        vec += 0x08;
    }

    assert(false);
    return 0;
}