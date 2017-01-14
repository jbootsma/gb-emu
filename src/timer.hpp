#ifndef TIMER_HPP
#define TIMER_HPP

#include <cstdint>

static const std::uint16_t DIV_ADR = 0xff04;
static const std::uint16_t TIMA_ADR = 0xff05;
static const std::uint16_t TMA_ADR = 0xff06;
static const std::uint16_t TAC_ADR = 0xff07;

class InterruptController;

class Timer
{
public:
    Timer(InterruptController *ic) :
        ic(ic)
    {}

    void reset();
    void step();

    void setDIV(std::uint8_t) { DIV = 0; }
    void setTIMA(std::uint8_t val) { TIMA = val; }
    void setTMA(std::uint8_t val) { TMA = val; }
    void setTAC(std::uint8_t val) { TAC = val; }
    std::uint8_t getDIV() const { return DIV; }
    std::uint8_t getTIMA() const { return TIMA; }
    std::uint8_t getTMA() const { return TMA; }
    std::uint8_t getTAC() const { return TAC | TAC_unused; }

private:
    static const std::uint8_t DIV_tick_mask = 64 - 1;
    static const std::uint8_t TIMA_tick_mask[4];
    static const std::uint8_t TAC_start_mask = 0x04;
    static const std::uint8_t TAC_speed_mask = 0x03;
    static const std::uint8_t TAC_unused = 0xF8;

    std::uint8_t DIV;
    std::uint8_t TIMA;
    std::uint8_t TMA;
    std::uint8_t TAC;

    std::uint8_t cycle;

    InterruptController *ic;
};

#endif
