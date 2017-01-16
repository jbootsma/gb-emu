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

#include "cpu.hpp"

#include <cassert>

#include "interrupt_controller.hpp"
#include "mmu.hpp"

static std::uint16_t make16(std::uint8_t hi, std::uint8_t lo)
{
    return hi << 8 | lo;
}

static void break16(std::uint16_t val, std::uint8_t &hi, std::uint8_t &lo)
{
    lo = (std::uint8_t)val;
    hi = (std::uint8_t)(val >> 8);
}

void CPU::reset()
{
    F = 0;

    A = 0;
    B = 0;
    C = 0;
    D = 0;
    E = 0;
    H = 0;
    L = 0;

    PC = 0x100;
    SP = 0;

    ime = false;
    halting = false;
    halt_bug = false;

    ctrl = &instr.ops[0].front();
}

std::uint8_t CPU::getr8(REG8 reg)
{
    switch (reg)
    {
    case REG8::A: return A;
    case REG8::F: return F;
    case REG8::B: return B;
    case REG8::C: return C;
    case REG8::D: return D;
    case REG8::E: return E;
    case REG8::H: return H;
    case REG8::L: return L;
    case REG8::PCH: return (std::uint8_t)(PC >> 8);
    case REG8::PCL: return (std::uint8_t)(PC);
    case REG8::SPH: return (std::uint8_t)(SP >> 8);
    case REG8::SPL: return (std::uint8_t)(SP);
    case REG8::TH: return (std::uint8_t)(T >> 8);
    case REG8::TL: return (std::uint8_t)(T);
    default:
        assert(false);
        return 0;
    }
}

void CPU::setr8(REG8 reg, std::uint8_t val)
{
    switch (reg)
    {
    case REG8::A: A = val; break;
    case REG8::F: F = val & all_flags_mask; break;
    case REG8::B: B = val; break;
    case REG8::C: C = val; break;
    case REG8::D: D = val; break;
    case REG8::E: E = val; break;
    case REG8::H: H = val; break;
    case REG8::L: L = val; break;
    case REG8::PCH: PC = (PC & 0x00FF) | (val << 8); break;
    case REG8::PCL: PC = (PC & 0xFF00) | val; break;
    case REG8::SPH: SP = (SP & 0x00FF) | (val << 8); break;
    case REG8::SPL: SP = (SP & 0xFF00) | val; break;
    case REG8::TH: T = (T & 0x00FF) | (val << 8); break;
    case REG8::TL: T = (T & 0xFF00) | val; break;
    default:
        assert(false);
    }
}

std::uint16_t CPU::getr16(REG16 reg)
{
    switch (reg)
    {
    case REG16::AF: return make16(A, F);
    case REG16::BC: return make16(B, C);
    case REG16::DE: return make16(D, E);
    case REG16::HL: return make16(H, L);
    case REG16::HLP:
        {
            std::uint16_t val = make16(H, L);
            break16(val + 1, H, L);
            return val;
        }
    case REG16::HLM:
        {
            std::uint16_t val = make16(H, L);
            break16(val - 1, H, L);
            return val;
        }
    case REG16::PC:
        {
            std::uint16_t val = PC++;
            if (halt_bug) PC--;
            return val;
        }
    case REG16::SP: return SP;
    case REG16::T: return T;
    case REG16::TP1: return T + 1;
    case REG16::OTL: return 0xFF00 + (T & 0xFF);
    case REG16::OC: return 0xFF00 + C;
    default:
        assert(false);
        return 0;
    }
}

void CPU::setr16(REG16 reg, std::uint16_t val)
{
    switch (reg)
    {
    case REG16::AF: break16(val & (0xFF00 | all_flags_mask), A, F); break;
    case REG16::BC: break16(val, B, C); break;
    case REG16::DE: break16(val, D, E); break;
    case REG16::HL: break16(val, H, L); break;
    case REG16::PC: PC = val; break;
    case REG16::SP: SP = val; break;
    case REG16::T: T = val; break;
    default:
        assert(false);
    }
}

void CPU::step()
{
    std::uint8_t data_in = 0;

    if (halting && !ic->interrupt_pending()) return;
    halting = false;

    if (ctrl->decode &&         // Can only interrupt at the end of an instruction.
        ime &&                  // and only if interrupts are enabled.
        ic->interrupt_pending())  // and only if an interrupt is actually pending.
    {
        T = ic->accept_interrupt();
        ctrl = &instr.interrupt_op[0];
    }

    // Note, data is likely sampled at end of 3rd sub cycle
    // https://forums.nesdev.com/viewtopic.php?f=20&t=14014
    if (ctrl->read)
    {
        data_in = mmu->read_mem(getr16(ctrl->adr));
        // Need an explicit check as here the reg can be none as a special case.
        if (ctrl->mem_reg != REG8::none) setr8(ctrl->mem_reg, data_in);
        if (ctrl->adr == REG16::SP) ++SP;
    }

    if (ctrl->write)
    {
        if (ctrl->adr == REG16::SP) --SP;
        mmu->write_mem(getr16(ctrl->adr), getr8(ctrl->mem_reg));
    }

    if (ctrl->decode)
    {
        try
        {
            ctrl = &instr.ops[data_in].at(0);
        }
        catch (std::out_of_range&)
        {
            throw std::runtime_error("Unimplemented op code");
        }
    }
    else if (ctrl->decode_cb)
    {
        ctrl = &instr.cb_ops[data_in][0];
    }
    else
    {
        ctrl++;
    }

    switch (ctrl->cond_op)
    {
    case CONDITION::none:
        break;
    case CONDITION::NZ:
        cond_flag = !(F & z_mask);
        break;
    case CONDITION::Z:
        cond_flag = !!(F & z_mask);
        break;
    case CONDITION::NC:
        cond_flag = !(F & c_mask);
        break;
    case CONDITION::C:
        cond_flag = !!(F& c_mask);
        break;
    case CONDITION::always:
        cond_flag = true;
        break;
    case CONDITION::check:
        while (!cond_flag && ctrl->cond_op == CONDITION::check) ctrl++;
        break;
    }

    if (ctrl->ld)
    {
        setr8(ctrl->dst, getr8(ctrl->src));
    }

    switch (ctrl->alu_op)
    {
    case ALU_OP::none:
        break;

    case ALU_OP::cp:
    case ALU_OP::sub:
    case ALU_OP::add:
        {
            bool carry = (ctrl->with_carry) ? !!(F & c_mask) : 0;
            std::uint8_t src = getr8(ctrl->alu_r8);
            F = 0;

            if (ctrl->alu_op != ALU_OP::add)
            {
                // This does a two's complement, the plus one is effectivly done by flipping the carry bit.
                src = ~src;
                carry = !carry;
                F |= n_mask | h_mask | c_mask;
            }

            std::uint16_t result = (std::uint16_t)src + A + (carry ? 1 : 0);
            if (result & 0x100) F ^= c_mask;
            if (0x10 & (result ^ src ^ A)) F ^= h_mask;
            if ((result & 0xFF) == 0) F |= z_mask;

            if (ctrl->alu_op != ALU_OP::cp) A = (std::uint8_t)result;
        }
        break;

    case ALU_OP::and_op:
    case ALU_OP::xor_op:
    case ALU_OP::or_op:
        {
            std::uint8_t src = getr8(ctrl->alu_r8);
            F = 0;

            switch (ctrl->alu_op)
            {
            case ALU_OP::and_op:
                F |= h_mask;
                A &= src;
                break;
            case ALU_OP::xor_op:
                A ^= src;
                break;
            case ALU_OP::or_op:
                A |= src;
                break;
            }

            if (A == 0) F |= z_mask;
        }
        break;

    case ALU_OP::inc:
    case ALU_OP::dec:
        {
            std::uint8_t val = getr8(ctrl->alu_r8);
            F &= ~(z_mask | n_mask | h_mask);

            if (ctrl->alu_op == ALU_OP::inc)
            {
                ++val;
                if ((val & 0x0f) == 0) F |= h_mask;
            }
            else
            {
                F |= n_mask;
                --val;
                if ((val & 0xf) == 0xf) F |= h_mask;
            }

            if (val == 0) F |= z_mask;
            setr8(ctrl->alu_r8, val);
        }
        break;

    case ALU_OP::daa:
        {
            std::uint16_t temp = A;

            if (F & n_mask)
            {
                if (F & h_mask)
                {
                    temp -= 6;
                }

                if (F & c_mask)
                {
                    temp -= 6 << 4;
                }
            }
            else
            {
                if ((F & h_mask) || (temp & 0xF) > 9)
                {
                    temp += 6;
                }

                if ((F & c_mask) || (temp & 0xFFF0) > (9 << 4))
                {
                    temp += 6 << 4;
                }
                if (temp & 0x100) F |= c_mask;
            }

            A = (std::uint8_t) temp;
            F &= ~(z_mask | h_mask);
            if (A == 0) F |= z_mask;
        }
        break;

    case ALU_OP::cpl:
        F |= n_mask | h_mask;
        A = ~A;
        break;

    case ALU_OP::scf:
        F &= ~(n_mask | h_mask);
        F |= c_mask;
        break;

    case ALU_OP::ccf:
        F &= ~(n_mask | h_mask);
        F ^= c_mask;
        break;

    case ALU_OP::rl:
    case ALU_OP::rr:
    case ALU_OP::sla:
    case ALU_OP::sra:
    case ALU_OP::srl:
        {
            bool left = false;
            if (ctrl->alu_op == ALU_OP::rl || ctrl->alu_op == ALU_OP::sla) left = true;

            std::uint8_t val = getr8(ctrl->alu_r8);
            bool carry_out = !!(val & (left ? 0x80 : 0x01));
            bool carry_in = ctrl->with_carry ? carry_out : !!(F & c_mask);

            if (ctrl->alu_op == ALU_OP::sra) carry_in = !!(val & 0x80);

            switch (ctrl->alu_op)
            {
            case ALU_OP::rl:
                val = (val << 1) | (carry_in ? 0x01 : 0);
                break;
            case ALU_OP::sra:
            case ALU_OP::rr:
                val = (val >> 1) | (carry_in ? 0x80 : 0);
                break;
            case ALU_OP::sla:
                val = val << 1;
                break;
            case ALU_OP::srl:
                val = val >> 1;
                break;
            }

            F = 0;
            if (!ctrl->ignore_zero && val == 0) F |= z_mask;
            if (carry_out) F |= c_mask;
            setr8(ctrl->alu_r8, val);
        }
        break;

    case ALU_OP::swap:
        {
            std::uint8_t val = getr8(ctrl->alu_r8);
            F = 0;
            val = (val >> 4) | (val << 4);
            if (val == 0) F |= z_mask;
            setr8(ctrl->alu_r8, val);
        }
        break;

    case ALU_OP::bit:
        {
            std::uint8_t val = getr8(ctrl->alu_r8);
            F &= ~(z_mask | n_mask);
            F |= h_mask;
            if ((val & ctrl->mask) == 0) F |= z_mask;
        }
        break;

    case ALU_OP::res:
        {
            std::uint8_t val = getr8(ctrl->alu_r8);
            val &= ~(ctrl->mask);
            setr8(ctrl->alu_r8, val);
        }
        break;

    case ALU_OP::set:
        {
            std::uint8_t val = getr8(ctrl->alu_r8);
            val |= ctrl->mask;
            setr8(ctrl->alu_r8, val);
        }
        break;

    case ALU_OP::sp_adjust:
        {
            std::uint16_t adjust = T & 0xFF;
            if (adjust & 0x80) adjust |= 0xFF00;

            F = 0;
            std::uint16_t result = SP + adjust;
            if (0x0010 & (result ^ adjust ^ SP)) F |= h_mask;
            if (0x0100 & (result ^ adjust ^ SP)) F |= c_mask;

            setr16(ctrl->alu_r16, result);
        }
        break;

    case ALU_OP::pc_adjust:
        {
            std::uint16_t adjust = T & 0xFF;
            if (adjust & 0x80) adjust |= 0xFF00;
            PC += adjust;
        }
        break;

    case ALU_OP::pc_set:
        PC = getr16(ctrl->alu_r16);
        break;

    case ALU_OP::pc_reset:
        PC = ctrl->mask;
        break;

    case ALU_OP::inc16:
        setr16(ctrl->alu_r16, getr16(ctrl->alu_r16) + 1);
        break;

    case ALU_OP::dec16:
        setr16(ctrl->alu_r16, getr16(ctrl->alu_r16) - 1);
        break;

    case ALU_OP::add16:
        {
            F &= ~(n_mask | h_mask | c_mask);
            std::uint16_t src = getr16(ctrl->alu_r16);
            std::uint32_t result = src + make16(H, L);

            if (result & 0x10000) F |= c_mask;
            if (0x1000 & (result ^ src ^ (H << 8))) F |= h_mask;

            break16((std::uint16_t)result, H, L);
        }
        break;
    }

    switch (ctrl->sys_op)
    {
    case SYS_OP::ei:
        ime = true;
        break;
    case SYS_OP::di:
        ime = false;
        break;
    case SYS_OP::halt:
        if (!ime && ic->interrupt_pending()) halt_bug = true;
        halting = !ic->interrupt_pending();
        break;
    case SYS_OP::stop:
        assert(false);
        break;
    }
}

CPUState CPU::getState()
{
    // Can only get the state between instructions.
    assert(ctrl->decode);
    return CPUState{ A, F, B, C, D, E, H, L, PC, SP, ime };
}

void CPU::setState(const CPUState& state)
{
    // Can only change state between instructions.
    assert(ctrl->decode);
    A = state.A;
    F = state.F;
    B = state.B;
    C = state.C;
    D = state.D;
    E = state.E;
    H = state.H;
    L = state.L;
    PC = state.PC;
    SP = state.SP;
    ime = state.ime;
}
