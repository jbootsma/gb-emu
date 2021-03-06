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

#ifndef MMU_HPP
#define MMU_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <iosfwd>

#include "config.hpp"

class Cart;
class InterruptController;
class GPU;
class Timer;

class MMU
{
public:

    std::uint8_t read_mem(std::uint16_t adr);
    void write_mem(std::uint16_t adr, std::uint8_t val);

    MMU(Cart *cart,
        GPU *gpu,
        InterruptController *ic,
        Timer *timer) :
        cart(cart), gpu(gpu), ic(ic), timer(timer),
        break_req(false)
    {
        for (std::int32_t &adr : breakpoints) adr = -1;
    }

    bool break_req;
    std::int32_t breakpoints[NUM_MEM_BREAKPOINTS];

private:
    Cart *cart;
    GPU *gpu;
    InterruptController *ic;
    Timer *timer;
    std::array<std::uint8_t, 0x2000> loram;
    std::array<std::uint8_t, 0x79> hiram;

    // Temporary until all registers are implemented.
    std::array<std::uint8_t, 0x80> ioshadow;
};

#endif
