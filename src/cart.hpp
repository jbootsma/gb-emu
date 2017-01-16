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

#ifndef CART_HPP
#define CART_HPP

#include <array>
#include <cstdint>
#include <iosfwd>
#include <string>
#include <vector>

enum class CartController
{
    ROM,
    MBC1,
    MBC2,
    MMM01,
    MBC3,
    MBC5,
    MBC6,
    MBC7,
    CAMERA,
    TAMA5,
    HuC3,
    HuC1,
    Unknown,
};

struct CartType
{
    CartController controller;
    bool ram;
    bool battery;
    bool timer;
    bool rumble;
    bool accel;
};

struct CartHeader
{
    std::string title;
    std::uint8_t version;
    bool international;
    std::uint8_t licensee;
    std::string new_licensee;
    std::string manufacturer;
    bool dmg_compat;
    bool gbc_compat;
    bool sgb_compat;
    CartType cart_type;
    std::size_t rom_size;
    std::size_t ram_size;
    bool unknown_ram_size;
    std::uint8_t checksum;
    std::uint16_t global_checksum;
    bool checksum_passed;
    bool global_checksum_passed;
    bool logo_check_passed;

    CartHeader();
};

class Cart
{
public:
    void loadCart(std::istream &cartFile);
    const CartHeader& getHeader() { return header; }

    void reset();

    std::uint8_t readROM(std::uint16_t adr);
    void writeROM(std::uint16_t adr, std::uint8_t val);

    std::uint8_t readRAM(std::uint16_t adr);
    void writeRAM(std::uint16_t adr, std::uint8_t val);

private:
    CartHeader header;

    enum class RamMode
    {
        OFF,
        HALF,
        ON,
    };

    std::vector<uint8_t> rom;
    std::size_t rom_bank_base;
    std::vector<uint8_t> ram;
    RamMode ram_mode;
    std::size_t ram_bank_base;

    std::array<std::uint8_t, 4> ctrl_regs;
};

#endif
