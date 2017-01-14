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

#include "timer.hpp"

#include "interrupt_controller.hpp"

const std::uint8_t Timer::TIMA_tick_mask[4] = { 256-1, 4-1, 16-1, 64-1 };

void Timer::reset()
{
    cycle = 0;
    DIV = 0;
    TIMA = 0;
    TMA = 0;
    TAC = 0;
}

void Timer::step()
{
    ++cycle;

    if (0 == (cycle & DIV_tick_mask)) DIV++;

    if (TAC & TAC_start_mask)
    {
        if (0 == (cycle & TIMA_tick_mask[TAC & TAC_speed_mask]))
        {
            if (++TIMA == 0)
            {
                TIMA = TMA;
                ic->signal_timer_irq();
            }
        }
    }
}
