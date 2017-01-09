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

#include "instructions.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

static void reset(CPU_Control &ctrl)
{
    memset(&ctrl, 0, sizeof(ctrl));
}

static void add_fetch(CPU_Control &ctrl)
{
    ctrl.decode = true;
    ctrl.read = true;
    ctrl.adr = REG16::PC;
}

static REG8 map8(int bits)
{
    switch (bits)
    {
    case 0: return REG8::B;
    case 1: return REG8::C;
    case 2: return REG8::D;
    case 3: return REG8::E;
    case 4: return REG8::H;
    case 5: return REG8::L;
    case 6: return REG8::F;
    case 7: return REG8::A;
    default:
        assert(false);
        return REG8::none;
    }
}

static REG16 map16(int bits)
{
    switch (bits)
    {
    case 0: return REG16::BC;
    case 1: return REG16::DE;
    case 2: return REG16::HLP;
    case 3: return REG16::HLM;
    default:
        assert(false);
        return REG16::none;
    }
}

static CONDITION map_cond(int bits)
{
    switch (bits)
    {
    case 0: return CONDITION::NZ;
    case 1: return CONDITION::Z;
    case 2: return CONDITION::NC;
    case 3: return CONDITION::C;
    default:
        assert(false);
        return CONDITION::none;
    }
}

static void map_alu_op(CPU_Control& ctrl, int bits)
{
    switch (bits)
    {
    case 1:
        ctrl.with_carry = true;
        // FALL-THROUGH
    case 0:
        ctrl.alu_op = ALU_OP::add;
        break;
    case 3:
        ctrl.with_carry = true;
        // FALL-THROUGH
    case 2:
        ctrl.alu_op = ALU_OP::sub;
        break;
    case 4:
        ctrl.alu_op = ALU_OP::and_op;
        break;
    case 5:
        ctrl.alu_op = ALU_OP::xor_op;
        break;
    case 6:
        ctrl.alu_op = ALU_OP:: or_op;
        break;
    case 7:
        ctrl.alu_op = ALU_OP::cp;
        break;
    default:
        assert(false);
    }
}

static void check_op(const CPU_Control& ctrl)
{
    // Can't read and write in the same machine cycle.
    assert(!ctrl.read || !ctrl.write);

    // Need an address for reads/writes.
    if (ctrl.read || ctrl.write) assert(ctrl.adr != REG16::none);

    // Only one instruction decode at a time.
    assert(!ctrl.decode || !ctrl.decode_cb);

    // Data must be read to be able to decode it.
    if (ctrl.decode || ctrl.decode_cb) assert(ctrl.read);

    // Load and ALU 8-bit ops can't occur simultaneously.
    assert(!ctrl.ld || ctrl.alu_r8 == REG8::none);

    // ld needs valid ops.
    if (ctrl.ld)
    {
        assert(ctrl.src != REG8::none);
        assert(ctrl.dst != REG8::none);
    }
    else
    {
        assert(ctrl.src == REG8::none);
        assert(ctrl.dst == REG8::none);
    }

    // Check alu ops
    switch (ctrl.alu_op)
    {
    case ALU_OP::and_op:
    case ALU_OP::xor_op:
    case ALU_OP::or_op:
    case ALU_OP::cp:
    case ALU_OP::inc:
    case ALU_OP::dec:
    case ALU_OP::sla:
    case ALU_OP::sra:
    case ALU_OP::srl:
    case ALU_OP::swap:
        assert(!ctrl.with_carry);
        // FALL THROUGH
    case ALU_OP::add:
    case ALU_OP::sub:
        assert(!ctrl.ignore_zero);
        // FALL THROUGH
    case ALU_OP::rr:
    case ALU_OP::rl:
        assert(ctrl.mask == 0);
        assert(ctrl.alu_r16 == REG16::none);
        assert(ctrl.alu_r8 != REG8::none);
        break;
    case ALU_OP::daa:
    case ALU_OP::cpl:
    case ALU_OP::scf:
    case ALU_OP::ccf:
        assert(!ctrl.with_carry);
        assert(!ctrl.ignore_zero);
        assert(ctrl.mask == 0);
        assert(ctrl.alu_r16 == REG16::none);
        assert(ctrl.alu_r8 == REG8::none);
        break;
    case ALU_OP::bit:
    case ALU_OP::res:
    case ALU_OP::set:
        assert(!ctrl.with_carry);
        assert(!ctrl.ignore_zero);
        assert(ctrl.mask != 0);
        // This looks complicated, but it checks to make sure there is only 1 bit set.
        assert((ctrl.mask & (ctrl.mask - 1)) == 0);
        assert(ctrl.alu_r16 == REG16::none);
        assert(ctrl.alu_r8 != REG8::none);
        break;
    case ALU_OP::pc_adjust:
    case ALU_OP::pc_reset:
        assert(!ctrl.with_carry);
        assert(!ctrl.ignore_zero);
        if (ctrl.alu_op != ALU_OP::pc_reset) assert(ctrl.mask == 0);
        assert(ctrl.alu_r16 == REG16::none);
        assert(ctrl.alu_r8 == REG8::none);
        break;
    case ALU_OP::pc_set:
        assert(ctrl.alu_r16 == REG16::T || ctrl.alu_r16 == REG16::HL1);
        // FALL THROUGH
    case ALU_OP::sp_adjust:
    case ALU_OP::inc16:
    case ALU_OP::dec16:
    case ALU_OP::add16:
        assert(!ctrl.with_carry);
        assert(!ctrl.ignore_zero);
        assert(ctrl.mask == 0);
        assert(ctrl.alu_r16 != REG16::none);
        assert(ctrl.alu_r8 == REG8::none);
        break;
    case ALU_OP::none:
        assert(!ctrl.with_carry);
        assert(!ctrl.ignore_zero);
        assert(ctrl.mask == 0);
        assert(ctrl.alu_r16 == REG16::none);
        assert(ctrl.alu_r8 == REG8::none);
        break;
    }
}

std::vector<Instructions::Instruction> Instructions::make_ops()
{
    std::vector<Instruction> ops;

    ops.resize((std::size_t)UINT8_MAX + 1);

    for (std::size_t op_code = 0; op_code < ops.size(); op_code++)
    {
        CPU_Control ctrl;
        Instruction &op = ops.at(op_code);

        reset(ctrl);
        switch (op_code)
        {
        case 0x00:
            // NOP
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0xcb:
            // CB-Prefix
            ctrl.read = true;
            ctrl.adr = REG16::PC;
            ctrl.decode_cb = true;
            op.push_back(ctrl);
            break;

        case 0x06:
        case 0x0e:
        case 0x16:
        case 0x1e:
        case 0x26:
        case 0x2e:
        case 0x3e:
            // LD R8, d8
            ctrl.read = true;
            ctrl.adr = REG16::PC;
            ctrl.ld = true;
            ctrl.src = REG8::DATA;
            ctrl.dst = map8((op_code >> 3) & 7);
            op.push_back(ctrl);

            reset(ctrl);
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0x40:
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x47:
        case 0x48:
        case 0x49:
        case 0x4a:
        case 0x4b:
        case 0x4c:
        case 0x4d:
        case 0x4f:
        case 0x50:
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x55:
        case 0x57:
        case 0x58:
        case 0x59:
        case 0x5a:
        case 0x5b:
        case 0x5c:
        case 0x5d:
        case 0x5f:
        case 0x60:
        case 0x61:
        case 0x62:
        case 0x63:
        case 0x64:
        case 0x65:
        case 0x67:
        case 0x68:
        case 0x69:
        case 0x6a:
        case 0x6b:
        case 0x6c:
        case 0x6d:
        case 0x6f:
        case 0x78:
        case 0x79:
        case 0x7a:
        case 0x7b:
        case 0x7c:
        case 0x7d:
        case 0x7f:
            // LD R8, R8
            ctrl.ld = true;
            ctrl.src = map8(op_code & 7);
            ctrl.dst = map8((op_code >> 3) & 7);
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0x02:
        case 0x12:
        case 0x22:
        case 0x32:
            // LD (R16), A
            ctrl.ld = true;
            ctrl.src = REG8::A;
            ctrl.dst = REG8::DATA;
            ctrl.write = true;
            ctrl.adr = map16((op_code >> 4) & 3);
            op.push_back(ctrl);

            reset(ctrl);
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0x0a:
        case 0x1a:
        case 0x2a:
        case 0x3a:
            // LD A, (R16)
            ctrl.ld = true;
            ctrl.src = REG8::DATA;
            ctrl.dst = REG8::A;
            ctrl.read = true;
            ctrl.adr = map16((op_code >> 4) & 3);
            op.push_back(ctrl);

            reset(ctrl);
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0x70:
        case 0x71:
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x75:
        case 0x77:
            // LD (HL), R8
            ctrl.ld = true;
            ctrl.src = map8(op_code & 7);
            ctrl.dst = REG8::DATA;
            ctrl.write = true;
            ctrl.adr = REG16::HL;
            op.push_back(ctrl);

            reset(ctrl);
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0x46:
        case 0x4e:
        case 0x56:
        case 0x5e:
        case 0x66:
        case 0x6e:
        case 0x7e:
            // LD R8, (HL)
            ctrl.ld = true;
            ctrl.src = REG8::DATA;
            ctrl.dst = map8((op_code >> 3) & 7);
            ctrl.read = true;
            ctrl.adr = REG16::HL;
            op.push_back(ctrl);

            reset(ctrl);
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0x36:
            // LD (HL), d8
            ctrl.ld = true;
            ctrl.src = REG8::DATA;
            ctrl.dst = REG8::TL;
            ctrl.read = true;
            ctrl.adr = REG16::PC;
            op.push_back(ctrl);

            reset(ctrl);
            ctrl.ld = true;
            ctrl.src = REG8::TL;
            ctrl.dst = REG8::DATA;
            ctrl.write = true;
            ctrl.adr = REG16::HL;
            op.push_back(ctrl);

            reset(ctrl);
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0xea:
        case 0xfa:
            // LD (a16) <==> A
            ctrl.ld = true;
            ctrl.src = REG8::DATA;
            ctrl.dst = REG8::TL;
            ctrl.read = true;
            ctrl.adr = REG16::PC;
            op.push_back(ctrl);

            ctrl.dst = REG8::TH;
            op.push_back(ctrl);

            reset(ctrl);
            ctrl.ld = true;
            if (op_code & 0x10)
            {
                ctrl.src = REG8::DATA;
                ctrl.dst = REG8::A;
                ctrl.read = true;
            }
            else
            {
                ctrl.src = REG8::A;
                ctrl.dst = REG8::DATA;
                ctrl.write = true;
            }
            ctrl.adr = REG16::T;
            op.push_back(ctrl);

            reset(ctrl);
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0xe0:
        case 0xf0:
            // LDH (a8) <=> A
            ctrl.ld = true;
            ctrl.src = REG8::DATA;
            ctrl.dst = REG8::TL;
            ctrl.read = true;
            ctrl.adr = REG16::PC;
            op.push_back(ctrl);

            reset(ctrl);
            ctrl.ld = true;
            if (op_code & 0x10)
            {
                ctrl.src = REG8::DATA;
                ctrl.dst = REG8::A;
                ctrl.read = true;
            }
            else
            {
                ctrl.src = REG8::A;
                ctrl.dst = REG8::DATA;
                ctrl.write = true;
            }
            ctrl.adr = REG16::OTL;
            op.push_back(ctrl);

            reset(ctrl);
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0xe2:
        case 0xf2:
            // LD (C) <==> A
            ctrl.ld = true;
            if (op_code & 0x10)
            {
                ctrl.src = REG8::DATA;
                ctrl.dst = REG8::A;
                ctrl.read = true;
            }
            else
            {
                ctrl.src = REG8::A;
                ctrl.dst = REG8::DATA;
                ctrl.write = true;
            }
            ctrl.adr = REG16::OC;
            op.push_back(ctrl);

            reset(ctrl);
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0x01:
        case 0x11:
        case 0x21:
        case 0x31:
            // LD R16, d16
            ctrl.ld = true;
            ctrl.src = REG8::DATA;
            ctrl.dst = map8(((op_code >> 3) & 6) | 1);
            if (ctrl.dst == REG8::A) ctrl.dst = REG8::SPL;
            ctrl.read = true;
            ctrl.adr = REG16::PC;            
            op.push_back(ctrl);

            ctrl.dst = map8((op_code >> 3) & 6);
            if (ctrl.dst == REG8::F) ctrl.dst = REG8::SPH;
            op.push_back(ctrl);

            reset(ctrl);
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0xC1:
        case 0xD1:
        case 0xE1:
        case 0xF1:
            // POP R16
            ctrl.ld = true;
            ctrl.src = REG8::DATA;
            ctrl.dst = map8(((op_code >> 3) & 6) | 1);
            if (ctrl.dst == REG8::A) ctrl.dst = REG8::F;
            ctrl.read = true;
            ctrl.adr = REG16::SP;
            op.push_back(ctrl);

            ctrl.dst = map8((op_code >> 3) & 6);
            if (ctrl.dst == REG8::F) ctrl.dst = REG8::A;
            op.push_back(ctrl);

            reset(ctrl);
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0xC5:
        case 0xD5:
        case 0xE5:
        case 0xF5:
            // PUSH R16
            // Dummy cycle, I'm guessing the original hardware needed this to pre-decrement SP
            op.push_back(ctrl);

            ctrl.ld = true;
            ctrl.src = map8((op_code >> 3) & 6);
            if (ctrl.src == REG8::F) ctrl.src = REG8::A;
            ctrl.dst = REG8::DATA;
            ctrl.write = true;
            ctrl.adr = REG16::SP;
            op.push_back(ctrl);

            ctrl.src = map8(((op_code >> 3) & 6) | 1);
            if (ctrl.src == REG8::A) ctrl.src = REG8::F;
            op.push_back(ctrl);

            reset(ctrl);
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0x08:
            // LD (a16), SP
            ctrl.ld = true;
            ctrl.src = REG8::DATA;
            ctrl.dst = REG8::TL;
            ctrl.read = true;
            ctrl.adr = REG16::PC;
            op.push_back(ctrl);

            ctrl.dst = REG8::TH;
            op.push_back(ctrl);

            reset(ctrl);
            ctrl.ld = true;
            ctrl.src = REG8::SPL;
            ctrl.dst = REG8::DATA;
            ctrl.write = true;
            ctrl.adr = REG16::T;
            op.push_back(ctrl);

            ctrl.src = REG8::SPH;
            ctrl.adr = REG16::TP1;
            op.push_back(ctrl);

            reset(ctrl);
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0xF8:
            // LD HL, SP+r8
            ctrl.ld = true;
            ctrl.src = REG8::DATA;
            ctrl.dst = REG8::TL;
            ctrl.read = true;
            ctrl.adr = REG16::PC;
            op.push_back(ctrl);

            reset(ctrl);
            ctrl.alu_op = ALU_OP::sp_adjust;
            ctrl.alu_r16 = REG16::HL;
            op.push_back(ctrl);

            reset(ctrl);
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0xF9:
            // LD SP, HL
            ctrl.ld = true;
            ctrl.src = REG8::L;
            ctrl.dst = REG8::SPL;
            op.push_back(ctrl);

            ctrl.src = REG8::H;
            ctrl.dst = REG8::SPH;
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0x80:
        case 0x81:
        case 0x82:
        case 0x83:
        case 0x84:
        case 0x85:
        case 0x87:
        case 0x88:
        case 0x89:
        case 0x8a:
        case 0x8b:
        case 0x8c:
        case 0x8d:
        case 0x8f:
        case 0x90:
        case 0x91:
        case 0x92:
        case 0x93:
        case 0x94:
        case 0x95:
        case 0x97:
        case 0x98:
        case 0x99:
        case 0x9a:
        case 0x9b:
        case 0x9c:
        case 0x9d:
        case 0x9f:
        case 0xa0:
        case 0xa1:
        case 0xa2:
        case 0xa3:
        case 0xa4:
        case 0xa5:
        case 0xa7:
        case 0xa8:
        case 0xa9:
        case 0xaa:
        case 0xab:
        case 0xac:
        case 0xad:
        case 0xaf:
        case 0xb0:
        case 0xb1:
        case 0xb2:
        case 0xb3:
        case 0xb4:
        case 0xb5:
        case 0xb7:
        case 0xb8:
        case 0xb9:
        case 0xba:
        case 0xbb:
        case 0xbc:
        case 0xbd:
        case 0xbf:
            // ALU A, R8
            map_alu_op(ctrl, (op_code >> 3) & 7);
            ctrl.alu_r8 = map8(op_code & 7);
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0x86:
        case 0x8e:
        case 0x96:
        case 0x9e:
        case 0xa6:
        case 0xae:
        case 0xb6:
        case 0xbe:
            // ALU A, (HL)
            ctrl.read = true;
            ctrl.adr = REG16::HL;
            map_alu_op(ctrl, (op_code >> 3) & 7);
            ctrl.alu_r8 = REG8::DATA;
            op.push_back(ctrl);

            reset(ctrl);
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0xc6:
        case 0xce:
        case 0xd6:
        case 0xde:
        case 0xe6:
        case 0xee:
        case 0xf6:
        case 0xfe:
            // ALU A, d8
            ctrl.read = true;
            ctrl.adr = REG16::PC;
            map_alu_op(ctrl, (op_code >> 3) & 7);
            ctrl.alu_r8 = REG8::DATA;
            op.push_back(ctrl);

            reset(ctrl);
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;
        case 0x04:
        case 0x05:
        case 0x0c:
        case 0x0d:
        case 0x14:
        case 0x15:
        case 0x1c:
        case 0x1d:
        case 0x24:
        case 0x25:
        case 0x2c:
        case 0x2d:
        case 0x3c:
        case 0x3d:
            // INC/DEC R8
            ctrl.alu_op = (op_code & 0x1) ? ALU_OP::dec : ALU_OP::inc;
            ctrl.alu_r8 = map8((op_code >> 3) & 7);
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0x34:
        case 0x35:
            // INC/DEC (HL)
            ctrl.read = true;
            ctrl.adr = REG16::HL;
            ctrl.alu_op = (op_code & 0x1) ? ALU_OP::dec : ALU_OP::inc;
            ctrl.alu_r8 = REG8::DATA;
            op.push_back(ctrl);

            reset(ctrl);
            ctrl.write = true;
            ctrl.adr = REG16::HL;
            op.push_back(ctrl);

            reset(ctrl);
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0x27:
            // DAA
            ctrl.alu_op = ALU_OP::daa;
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0x2f:
            // CPL
            ctrl.alu_op = ALU_OP::cpl;
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0x37:
            // SCF
            ctrl.alu_op = ALU_OP::scf;
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0x3f:
            // CCF
            ctrl.alu_op = ALU_OP::ccf;
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;
                
        case 0x07:
        case 0x0f:
        case 0x17:
        case 0x1f:
            // R(R/L)[C]A
            ctrl.alu_op = ((op_code & 0x08) != 0) ? ALU_OP::rr : ALU_OP::rl;
            ctrl.alu_r8 = REG8::A;
            ctrl.with_carry = (op_code & 0x10) == 0;
            ctrl.ignore_zero = true;
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0x03:
        case 0x0b:
        case 0x13:
        case 0x1b:
        case 0x23:
        case 0x2b:
        case 0x33:
        case 0x3b:
            // INC/DEC R16
            ctrl.alu_op = (op_code & 0x80) ? ALU_OP::dec16 : ALU_OP::inc16;
            ctrl.alu_r16 = map16((op_code >> 4) & 3);
            if (ctrl.alu_r16 == REG16::HLP) ctrl.alu_r16 = REG16::HL;
            if (ctrl.alu_r16 == REG16::HLM) ctrl.alu_r16 = REG16::SP;
            op.push_back(ctrl);

            reset(ctrl);
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0x09:
        case 0x19:
        case 0x29:
        case 0x39:
            // ADD HL, R16
            ctrl.alu_op = ALU_OP::add16;
            ctrl.alu_r16 = map16((op_code >> 4) & 3);
            if (ctrl.alu_r16 == REG16::HLP) ctrl.alu_r16 = REG16::HL;
            if (ctrl.alu_r16 == REG16::HLM) ctrl.alu_r16 = REG16::SP;
            op.push_back(ctrl);

            reset(ctrl);
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0xe8:
            // ADD SP, r8
            ctrl.ld = true;
            ctrl.src = REG8::DATA;
            ctrl.dst = REG8::TL;
            ctrl.adr = REG16::PC;
            op.push_back(ctrl);

            reset(ctrl);
            ctrl.alu_op = ALU_OP::sp_adjust;
            ctrl.alu_r16 = REG16::SP;
            op.push_back(ctrl);

            reset(ctrl);
            op.push_back(ctrl);

            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0xc7:
        case 0xcf:
        case 0xd7:
        case 0xdf:
        case 0xe7:
        case 0xef:
        case 0xf7:
        case 0xff:
            // RST
            ctrl.ld = true;
            ctrl.src = REG8::DATA;
            ctrl.dst = REG8::TL;
            op.push_back(ctrl);

            ctrl.write = true;
            ctrl.adr = REG16::SP;
            ctrl.ld = true;
            ctrl.src = REG8::PCH;
            ctrl.dst = REG8::DATA;
            op.push_back(ctrl);

            ctrl.src = REG8::PCL;
            ctrl.alu_op = ALU_OP::pc_reset;
            ctrl.mask = op_code & 0x38;
            op.push_back(ctrl);

            reset(ctrl);
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0x18:
        case 0x20:
        case 0x28:
        case 0x30:
        case 0x38:
            // JR r8
            ctrl.read = true;
            ctrl.adr = REG16::PC;
            ctrl.ld = true;
            ctrl.src = REG8::DATA;
            ctrl.dst = REG8::TL;
            ctrl.cond_op = (op_code == 0x18) ? CONDITION::always : map_cond((op_code >> 3) & 3);
            op.push_back(ctrl);

            reset(ctrl);
            ctrl.cond_op = CONDITION::check;
            ctrl.alu_op = ALU_OP::pc_adjust;
            op.push_back(ctrl);

            reset(ctrl);
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0xc2:
        case 0xc3:
        case 0xca:
        case 0xd2:
        case 0xda:
            // JP a16
            ctrl.read = true;
            ctrl.adr = REG16::PC;
            ctrl.ld = true;
            ctrl.src = REG8::DATA;
            ctrl.dst = REG8::TL;
            op.push_back(ctrl);

            ctrl.dst = REG8::TH;
            ctrl.cond_op = (op_code == 0xc3) ? CONDITION::always : map_cond((op_code >> 3) & 3);
            op.push_back(ctrl);

            reset(ctrl);
            ctrl.cond_op = CONDITION::check;
            ctrl.alu_op = ALU_OP::pc_set;
            ctrl.alu_r16 = REG16::T;
            op.push_back(ctrl);

            reset(ctrl);
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0xe9:
            // JP HL
            ctrl.read = true;
            ctrl.adr = REG16::HL;
            ctrl.decode = true;
            ctrl.alu_op = ALU_OP::pc_set;
            ctrl.alu_r16 = REG16::HL1;
            op.push_back(ctrl);
            break;

        case 0xc4:
        case 0xcc:
        case 0xcd:
        case 0xd4:
        case 0xdc:
            // CALL a16
            ctrl.read = true;
            ctrl.adr = REG16::PC;
            ctrl.ld = true;
            ctrl.src = REG8::DATA;
            ctrl.dst = REG8::TL;
            op.push_back(ctrl);

            ctrl.dst = REG8::TH;
            ctrl.cond_op = (op_code == 0xcd) ? CONDITION::always : map_cond((op_code >> 3) & 3);
            op.push_back(ctrl);

            reset(ctrl);
            ctrl.cond_op = CONDITION::check;
            // Dummy op, see notes in push.
            op.push_back(ctrl);

            ctrl.write = true;
            ctrl.adr = REG16::SP;
            ctrl.ld = true;
            ctrl.src = REG8::PCH;
            ctrl.dst = REG8::DATA;
            op.push_back(ctrl);

            ctrl.src = REG8::PCL;
            ctrl.alu_op = ALU_OP::pc_set;
            ctrl.alu_r16 = REG16::T;
            op.push_back(ctrl);

            reset(ctrl);
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0xc0:
        case 0xc8:
        case 0xc9:
        case 0xd9:
        case 0xd0:
        case 0xd8:
            // RET
            // This one is a little weird. The first cycle is needed to check the condition code but for nothing else,
            // so it seems that the always version skips that cycle completely.
            if ((op_code & 0xf) != 0x9)
            {
                ctrl.cond_op = map_cond((op_code >> 3) & 3);
                op.push_back(ctrl);

                reset(ctrl);
            }

            if ((op_code & 0xf) != 0x9) ctrl.cond_op = CONDITION::check;
            ctrl.read = true;
            ctrl.adr = REG16::SP;
            ctrl.ld = true;
            ctrl.src = REG8::DATA;
            ctrl.dst = REG8::PCL;
            op.push_back(ctrl);

            ctrl.dst = REG8::PCH;
            op.push_back(ctrl);

            reset(ctrl);
            if ((op_code & 0xf) != 0x9) ctrl.cond_op = CONDITION::check;
            // Dummy cycle. Actual hardware probably used this to set the PC from the work register
            // (maybe no 8 bit path from data port to PC reg?)
            op.push_back(ctrl);

            reset(ctrl);
            add_fetch(ctrl);
            if (op_code == 0xd9) ctrl.sys_op = SYS_OP::ei;
            op.push_back(ctrl);
            break;

        case 0x10:
            // STOP
            ctrl.sys_op = SYS_OP::stop;
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0x76:
            // HALT
            ctrl.sys_op = SYS_OP::halt;
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0xf3:
            // DI
            ctrl.sys_op = SYS_OP::di;
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;

        case 0xfb:
            // EI
            ctrl.sys_op = SYS_OP::ei;
            add_fetch(ctrl);
            op.push_back(ctrl);
            break;
        }
    }

    std::size_t unimplemented = 0;

    // Safety Checks.
    for (const Instruction &op : ops)
    {
        // Skip empty ops.
        if (op.empty())
        {
            unimplemented++;
            continue;
        }

        bool condition_set = false;
        bool decoded = false;
        for (const CPU_Control &ctrl : op)
        {
            check_op(ctrl);

            switch (ctrl.cond_op)
            {
            case CONDITION::NZ:
            case CONDITION::Z:
            case CONDITION::NC:
            case CONDITION::C:
            case CONDITION::always:
                // Can only set the condition once in an instruction.
                assert(!condition_set);
                condition_set = true;
                break;
            case CONDITION::check:
                // The condition needs to be set before it's checked.
                assert(condition_set);

                // decode can't be conditional.
                assert(!ctrl.decode && !ctrl.decode_cb);
                break;
            }

            // Decode can't occur until the end.
            assert(!decoded);
            decoded = ctrl.decode | ctrl.decode_cb;
        }

        // Every instruction must decode at some point.
        assert(decoded);
    }

    assert(unimplemented == 11);

    return ops;
}

std::vector<Instructions::Instruction> Instructions::make_cb_ops()
{
    std::vector<Instruction> ops;

    ops.resize((std::size_t)UINT8_MAX + 1);

    for (std::size_t op_code = 0; op_code < ops.size(); op_code++)
    {
        CPU_Control ctrl;
        Instruction &op = ops.at(op_code);

        reset(ctrl);

        if (op_code < 0x40)
        {
            switch (op_code >> 3)
            {
            case 0:
                ctrl.with_carry = true;
                // FALL-THROUGH
            case 2:
                ctrl.alu_op = ALU_OP::rl;
                break;
            case 1:
                ctrl.with_carry = true;
                // FALL-THROUGH
            case 3:
                ctrl.alu_op = ALU_OP::rr;
                break;
            case 4:
                ctrl.alu_op = ALU_OP::sla;
                break;
            case 5:
                ctrl.alu_op = ALU_OP::sra;
                break;
            case 6:
                ctrl.alu_op = ALU_OP::swap;
                break;
            case 7:
                ctrl.alu_op = ALU_OP::srl;
                break;
            }
        }
        else if (op_code < 0x80)
        {
            ctrl.alu_op = ALU_OP::bit;
        }
        else if (op_code < 0xc0)
        {
            ctrl.alu_op = ALU_OP::res;
        }
        else
        {
            ctrl.alu_op = ALU_OP::set;
        }

        if (op_code >= 0x40)
        {
            ctrl.mask = (1 << ((op_code >> 3) & 7));
        }

        ctrl.alu_r8 = map8(op_code & 7);

        if (ctrl.alu_r8 == REG8::F)
        {
            ctrl.read = true;
            ctrl.adr = REG16::HL;
            ctrl.alu_r8 = REG8::DATA;
            op.push_back(ctrl);

            reset(ctrl);
            ctrl.write = true;
            ctrl.adr = REG16::HL;
            op.push_back(ctrl);

            reset(ctrl);
            add_fetch(ctrl);
            op.push_back(ctrl);
        }
        else
        {
            add_fetch(ctrl);
            op.push_back(ctrl);
        }
    }

    // Safety Checks.
    for (const Instruction &op : ops)
    {
        bool decoded = false;
        bool op_present = false;
        for (const CPU_Control &ctrl : op)
        {
            check_op(ctrl);

            if (ctrl.alu_op != ALU_OP::none)
            {
                op_present = true;
                // All CB ops are 8-bit
                assert(ctrl.alu_r8 != REG8::none);
            }

            // Conditionals not allowed.
            assert(ctrl.cond_op == CONDITION::none);

            // Sysops not allowed.
            assert(ctrl.sys_op == SYS_OP::none);

            // CB prefix not allowed.
            assert(!ctrl.decode_cb);

            // Decode can't occur until the end.
            assert(!decoded);
            decoded = ctrl.decode;
        }

        // Every instruction must decode at some point.
        assert(decoded);
        // Every instruction needs to do an operation.
        assert(op_present);
    }

    return ops;
}
