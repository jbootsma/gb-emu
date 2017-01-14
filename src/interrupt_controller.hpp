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

#ifndef INTERRUPT_CONTROLLER_HPP
#define INTERRUPT_CONTROLLER_HPP

#include <cstdint>

static const std::uint16_t IF_ADR = 0xff0f;
static const std::uint16_t IE_ADR = 0xffff;

class InterruptController
{
public:
    bool interrupt_pending() const { return !!(IE & IF); }
    std::uint8_t accept_interrupt();

    void setIF(std::uint8_t val) { IF = val & ~unused; }
    void setIE(std::uint8_t val) { IE = val & ~unused; }
    std::uint8_t getIF() const { return IF | unused; }
    std::uint8_t getIE() const { return IE; }

    void signal_v_blank_irq() { IF |= 0x01; }
    void signal_lcd_stat_irq() { IF |= 0x02; }
    void signal_timer_irq() { IF |= 0x04; }
    void signal_serial_irq() { IF |= 0x08; }
    void signal_joypad_irq() { IF |= 0x10; }

private:
    static const std::uint8_t unused = 0xe0;

    std::uint8_t IE = 0;
    std::uint8_t IF = 0;
};

#endif
