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

#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include "system.hpp"

int main(int argc, char **argv)
{
    try
    {
        if (argc < 2) throw std::runtime_error("A rom must be supplied.");

        std::ifstream rom_file(argv[1], std::ios::in | std::ios::binary);
        if (!rom_file) throw std::runtime_error("Could not open supplied rom.");

        System sys;
        sys.cart.loadCart(rom_file);
        rom_file.close();

        CartHeader header = sys.cart.getHeader();
        std::cout << "Loaded " << header.title <<
            " v" << std::dec << (int)header.version <<
            (header.international ? "" : " (J)") <<
            "\nL: " << std::hex << std::setfill('0') << std::setw(2) << (int)header.licensee <<
            " L2: " << header.new_licensee <<
            " M: " << header.manufacturer <<
            "\nROM: " << std::dec << (header.rom_size / 1024) << 'K' <<
            " RAM: " << std::dec << (header.ram_size / 1024) << 'K' <<
            "\nChecksum: " << std::hex << std::setw(2) << (int)header.checksum <<
            (header.checksum_passed ? "" : " (BAD)") <<
            " Global: " << header.global_checksum << std::hex << std::setw(4) << header.global_checksum <<
            (header.global_checksum_passed ? "" : " (BAD)") << '\n';

        if (!header.dmg_compat) std::cout << "WARN: ROM appears to be incomaptible with DMG.\n";
        if (!header.logo_check_passed) std::cout << "WARN: Logo data in header appears to be corrupt.\n";

        while (true)
        {
            std::cout << std::endl;

            CPUState state = sys.cpu.getState();

            std::cout <<
                "A: " << std::setfill('0') << std::setw(2) << std::hex << (int)state.A << '\n' <<
                "F: " << std::setfill('0') << std::setw(2) << std::hex << (int)state.F << '\n' <<
                "B: " << std::setfill('0') << std::setw(2) << std::hex << (int)state.B << '\n' <<
                "C: " << std::setfill('0') << std::setw(2) << std::hex << (int)state.C << '\n' <<
                "D: " << std::setfill('0') << std::setw(2) << std::hex << (int)state.D << '\n' <<
                "E: " << std::setfill('0') << std::setw(2) << std::hex << (int)state.E << '\n' <<
                "H: " << std::setfill('0') << std::setw(2) << std::hex << (int)state.H << '\n' <<
                "L: " << std::setfill('0') << std::setw(2) << std::hex << (int)state.L << '\n' <<
                "PC: " << std::setfill('0') << std::setw(4) << std::hex << (int)state.PC << '\n' <<
                "SP: " << std::setfill('0') << std::setw(4) << std::hex << (int)state.SP << '\n' <<
                "ime: " << state.ime << '\n' << std::endl;

            char c;
            std::cin >> c;
            switch (c)
            {
            case 's':
                sys.step();
                break;
            case 'r':
                sys.reset();
                break;
            case 'c':
                sys.run();
                break;
            case 'b':
                {
                    int idx;
                    int adr;
                    std::cin >> idx >> std::hex >> adr;
                    sys.breakpoints[idx] = adr;
                }
                break;
            case 'm':
                {
                    int idx;
                    int adr;
                    std::cin >> idx >> std::hex >> adr;
                    sys.mmu.breakpoints[idx] = adr;
                }
                break;
            case 'd':
                {
                    int adr;
                    int count;
                    std::cin >> std::hex >> adr >> count;

                    for (std::uint16_t i = 0; i < count; i++)
                    {
                        std::cout << std::setw(2) << std::hex << (int)sys.mmu.read_mem((std::uint16_t)adr + i) << ' ';
                    }
                    std::cout << std::endl;
                }
            }
        }
    }
    catch (std::exception &e)
    {
        std::cout << "\nError: " << e.what() << std::endl;
    }
}
