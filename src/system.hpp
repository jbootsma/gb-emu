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

#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include <cstdint>
#include <iosfwd>

#include "cart.hpp"
#include "config.hpp"
#include "cpu.hpp"
#include "gpu.hpp"
#include "interrupt_controller.hpp"
#include "mmu.hpp"
#include "timer.hpp"

class System
{
public:
    System();

    void reset();
    void step();
    void run();

    std::int32_t breakpoints[NUM_BREAKPOINTS];

    Cart cart;
    GPU gpu;
    InterruptController ic;
    Timer timer;
    MMU mmu;
    CPU cpu;

private:
    bool checkBreakpoints();
};

#endif
