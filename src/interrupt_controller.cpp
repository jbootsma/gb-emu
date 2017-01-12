#include "interrupt_controller.hpp"

#include <cassert>

std::uint8_t InterruptController::accept_interrupt()
{
    std::uint8_t vec = 0x40;
    std::uint8_t bit = 0x01;

    while (bit)
    {
        if (IE & IF & bit)
        {
            IF &= ~bit;
            return vec;
        }
        bit <<= 1;
        vec += 0x08;
    }

    assert(false);
    return 0;
}