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

#include <memory>
#include <fstream>

#include "cpu.hpp"
#include "mmu.hpp"

static const std::uint8_t test_code[] = {0xd3};

int main(int, char **argv)
{
    std::unique_ptr<MMU> mmu(new MMU());
    std::unique_ptr<CPU> cpu(new CPU(mmu.get()));

    std::ifstream infile(argv[1], std::ios::in | std::ios::binary);

    std::vector<std::uint8_t> rom;
    rom.resize(0x8000);
    infile.read((char*)&rom.front(), 0x8000);

    for (int i = 0; i < rom.size(); i++)
    {
        mmu->write_mem((std::uint16_t)i, rom[i]);
    }

    infile.close();
    rom.clear();

    /*(void)argv;
    for (std::uint16_t i = 0; i < sizeof(test_code); i++)
    {
        mmu->write_mem(i, test_code[i]);
    }

    mmu->write_mem(0x100, 0xc3);*/

    cpu->reset();
    volatile std::uint16_t break_point = 0x100;

    while (1)
    {
        if (cpu->PC == break_point && cpu->ctrl->decode)
        {
            volatile bool b;
            // Put a breakpoint on the below line, then can set the value of break_point in the debugger to get a virtual breakpoint in the emulated code.
            b = false;
        }
        cpu->step();
    }
}
