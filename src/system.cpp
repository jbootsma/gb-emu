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

#include <array>
#include <istream>

#include "system.hpp"

System::System() :
    timer(&ic),
    mmu(&cart, &gpu, &ic, &timer),
    cpu(&mmu, &ic)
{
    for (std::int32_t &bp : breakpoints) bp = -1;
    reset();
}

void System::reset()
{
    cart.reset();
    timer.reset();
    cpu.reset();
}

void System::step()
{
    do
    {
        timer.step();
        cpu.step();
    } while (!cpu.isFetching());
}

void System::run()
{
    do
    {
        step();
    } while (!checkBreakpoints());
}

bool System::checkBreakpoints()
{
    if (mmu.break_req)
    {
        mmu.break_req = false;
        return true;
    }
    for (std::int32_t adr : breakpoints)
    {
        if (cpu.getPC() == adr) return true;
    }
    return false;
}
