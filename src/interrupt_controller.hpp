#ifndef INTERRUPT_CONTROLLER_HPP
#define INTERRUPT_CONTROLLER_HPP

#include <cstdint>

static const std::uint16_t IF_ADR = 0xFF0F;
static const std::uint16_t IE_ADR = 0xFFFF;

class InterruptController
{
private:
    std::uint8_t IE = 0;
    std::uint8_t IF = 0;

public:
    bool interrupt_pending() { return !!(IE & IF); }
    std::uint8_t accept_interrupt();

    void setIF(std::uint8_t val) { IF = val & 0x1f; }
    void setIE(std::uint8_t val) { IE = val & 0x1f; }
    std::uint8_t getIF() { return IF | 0xe0; }
    std::uint8_t getIE() { return IE; }

    void signal_v_blank_irq() { IF |= 0x01; }
    void signal_lcd_stat_irq() { IF |= 0x02; }
    void signal_timer_irq() { IF |= 0x04; }
    void signal_serial_irq() { IF |= 0x08; }
    void signal_joypad_irq() { IF |= 0x10; }
};

#endif
