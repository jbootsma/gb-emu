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

System::System(std::istream &romfile) :
    timer(&ic),
    mmu(&ic, &timer),
    cpu(&mmu, &ic)
{
    std::array<std::uint8_t, 0x8000> rom;
    romfile.read((char*)&rom[0], rom.size());

    for (std::uint16_t i = 0; i < rom.size(); i++)
    {
        mmu.write_mem(i, rom[i]);
    }

    reset();
}

void System::reset()
{
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
    for (std::uint16_t adr : breakpoints)
    {
        if (cpu.getPC() == adr) return true;
    }
    return false;
}
