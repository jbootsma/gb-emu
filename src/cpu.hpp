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

#ifndef CPU_HPP
#define CPU_HPP

#include <cstddef>
#include <cstdint>

#include "instructions.hpp"

class MMU;
struct CPU_Control;

class CPU
{
public:
    CPU(MMU *mmu) :
        mmu(mmu)
    {}

    void reset();
    void step();

//private:
    // Not private right now so that main code can access for (simulated) break points.

    static const std::uint8_t z_mask = (1 << 7);
    static const std::uint8_t n_mask = (1 << 6);
    static const std::uint8_t h_mask = (1 << 5);
    static const std::uint8_t c_shift = 4;
    static const std::uint8_t c_mask = (1 << c_shift);

    static const std::uint8_t all_flags_mask =
        z_mask | n_mask | h_mask | c_mask;

    const Instructions instr;

    std::uint8_t A;
    std::uint8_t F;
    std::uint8_t B;
    std::uint8_t C;
    std::uint8_t D;
    std::uint8_t E;
    std::uint8_t H;
    std::uint8_t L;

    std::uint16_t PC;
    std::uint16_t SP;

    bool interrupts_enabled;

    MMU *mmu;

    const CPU_Control *ctrl;

    std::uint8_t data;
    std::uint16_t T;
    bool cond_flag;

    std::uint8_t getr8(REG8 reg);
    void setr8(REG8 reg, std::uint8_t val);
    std::uint16_t getr16(REG16 reg);
    void setr16(REG16 reg, std::uint16_t val);
};

#endif
