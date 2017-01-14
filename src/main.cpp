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

        System sys(rom_file);
        rom_file.close();

        sys.run();
    }
    catch (std::exception &e)
    {
        std::cout << "\nError: " << e.what() << std::endl;
    }
}
