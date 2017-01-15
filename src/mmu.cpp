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

#include "mmu.hpp"

#include <cassert>
#include <iostream>

#include "cart.hpp"
#include "gpu.hpp"
#include "interrupt_controller.hpp"
#include "timer.hpp"

std::uint8_t MMU::read_mem(std::uint16_t adr)
{
    if (adr < 0x8000) return cart->readROM(adr);
    if (adr < 0xa000) return gpu->readVRAM(adr - 0x8000);
    if (adr < 0xc000) return cart->readRAM(adr - 0xa000);
    if (adr < 0xe000) return loram.at(adr - 0xc000);
    if (adr < 0xfe00) return loram.at(adr - 0xe000);
    if (adr < 0xfea0) return gpu->readOAM(adr - 0xfe00);
    if (adr < 0xff00) { assert(false); return 0; }
    if (adr < 0xff80)
    {
        switch (adr)
        {
        case IF_ADR:
            return ic->getIF();
        case DIV_ADR:
            return timer->getDIV();
        case TIMA_ADR:
            return timer->getTIMA();
        case TMA_ADR:
            return timer->getTMA();
        case TAC_ADR:
            return timer->getTAC();
        default:
            return ioshadow.at(adr - 0xff00);
        }
    }
    if (adr < 0xffff) return hiram.at(adr - 0xff80);
    return ic->getIE();
}

void MMU::write_mem(std::uint16_t adr, std::uint8_t val)
{
    for (std::int32_t bp : breakpoints) break_req |= bp == adr;

    if (adr < 0x8000) { cart->writeROM(adr, val); return; }
    if (adr < 0xa000) { gpu->writeVRAM(adr - 0x8000, val); return; }
    if (adr < 0xc000) { cart->writeRAM(adr - 0xa000, val); return; }
    if (adr < 0xe000) { loram.at(adr - 0xc000) = val; return; }
    if (adr < 0xfe00) { loram.at(adr - 0xe000) = val; return; }
    if (adr < 0xfea0) { gpu->writeOAM(adr - 0xfe00, val); return; }
    if (adr < 0xff00) { assert(false); return; }
    if (adr < 0xff80)
    {
        switch (adr)
        {
        case IF_ADR:
            ic->setIF(val);
            return;
        case DIV_ADR:
            timer->setDIV(val);
            return;
        case TIMA_ADR:
            timer->setTIMA(val);
            return;
        case TMA_ADR:
            timer->setTMA(val);
            return;
        case TAC_ADR:
            timer->setTAC(val);
            return;
        default:
            if (adr == 0xff01) std::cout << (char)val;
            ioshadow.at(adr - 0xff00) = val;
            return;
        }
    }
    if (adr < 0xffff) { hiram.at(adr - 0xff80) = val; return; }
    ic->setIE(val);
}
