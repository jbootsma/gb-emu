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

#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP

#include <vector>

enum class REG8
{
    none,
    A,      // Accumulator
    F,      // Flags
    B,      // General purpose
    C,      // General purpose
    D,      // General purpose
    E,      // General purpose
    H,      // General purpose
    L,      // General purpose
    PCH,    // High word of program counter
    PCL,    // Low word of program counter
    SPH,    // High word of stack pointer
    SPL,    // Low word of stack pointer
    TH,     // High temp byte
    TL,     // Low temp byte
};

enum class REG16
{
    none,
    AF,     // Core
    BC,     // Core
    DE,     // Core
    HL,     // Core
    HLP,    // HL with increment
    HLM,    // HL with decrement
    PC,     // Core
    SP,     // Core
    T,      // Temp reg
    TP1,    // Temp + 1
    OTL,    // Offset with temp lo (0xFF00 + temp_lo)
    OC,     // Offset with C (0xFF00 + C)
};

enum class ALU_OP
{
    none,
    add,
    sub,
    and_op,
    xor_op,
    or_op,
    cp,
    inc,
    dec,
    daa,
    cpl,
    scf,
    ccf,
    rl,
    rr,
    sla,
    sra,
    swap,
    srl,
    bit,
    res,
    set,
    sp_adjust,
    pc_adjust,
    pc_set,
    pc_reset,
    inc16,
    dec16,
    add16,
};

enum class CONDITION
{
    none,
    NZ,
    Z,
    NC,
    C,
    always,
    check,
};

enum class SYS_OP
{
    none,
    ei,
    di,
    halt,
    stop,
};

struct CPU_Control
{
    bool read;
    bool write;
    REG16 adr;
    REG8 mem_reg;

    bool decode;
    bool decode_cb;

    bool ld;
    REG8 src;
    REG8 dst;

    ALU_OP alu_op;
    bool with_carry;
    bool ignore_zero;
    std::uint8_t mask;
    REG16 alu_r16;
    REG8 alu_r8;

    CONDITION cond_op;
    SYS_OP sys_op;
};

class Instructions
{
public:
    using Instruction = std::vector<CPU_Control>;

    const std::vector<Instruction> ops;
    const std::vector<Instruction> cb_ops;

    Instructions() :
        ops(make_ops()),
        cb_ops(make_cb_ops())
    {}

private:
    static std::vector<Instruction> make_ops();
    static std::vector<Instruction> make_cb_ops();
};

#endif
