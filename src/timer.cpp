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
