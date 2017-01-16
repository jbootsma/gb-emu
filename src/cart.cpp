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

#include <cassert>
#include <cstdint>
#include <istream>

#include "cart.hpp"

#include "config.hpp"

static const std::size_t rom_min_size = 32 * 1024;

static const std::array<std::uint8_t, 48> logo_data = {
    0xce, 0xed, 0x66, 0x66, 0xcc, 0x0d, 0x00, 0x0b, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0c, 0x00, 0x0d,
    0x00, 0x08, 0x11, 0x1f, 0x88, 0x89, 0x00, 0x0e, 0xdc, 0xcc, 0x6e, 0xe6, 0xdd, 0xdd, 0xd9, 0x99,
    0xbb, 0xbb, 0x67, 0x63, 0x6e, 0x0e, 0xec, 0xcc, 0xdd, 0xdc, 0x99, 0x9f, 0xbb, 0xb9, 0x33, 0x3e,
};

CartHeader::CartHeader() :
    title("NO ROM"),
    version(0),
    international(false),
    licensee(0),
    dmg_compat(true),
    gbc_compat(false),
    sgb_compat(false),
    rom_size(32 * 1024),
    ram_size(0),
    unknown_ram_size(false),
    checksum(0),
    global_checksum(0),
    checksum_passed(false),
    logo_check_passed(false)
{
    memset(&cart_type, 0, sizeof(cart_type));
}

void Cart::loadCart(std::istream &cartFile)
{
    header = CartHeader();

    rom.resize(rom_min_size);
    ram.clear();
    cartFile.read((char*)&rom[0], rom.size());

    header.logo_check_passed = !memcmp(&logo_data.front(), &rom.at(0x104), logo_data.size());

    if (rom.at(0x143) & 0x80)
    {
        header.gbc_compat = true;
        header.dmg_compat = !(rom.at(0x143) & 0x40);
    }
    else
    {
        header.dmg_compat = true;
    }

    {
        std::size_t title_len = 15;
        const char* title = (const char*)&rom.at(0x134);
        title_len = strnlen(title, title_len);
        header.title.assign(title, title + title_len);
    }
    if (header.gbc_compat)
    {
        const char* manuf = (const char*)&rom.at(0x13f);
        header.manufacturer.assign(manuf, manuf + 4);
    }

    if (rom.at(0x146) == 0x03) header.sgb_compat = true;

    switch (rom.at(0x147))
    {
    case 0x09:
        header.cart_type.battery = true;
        // FALL-THROUGH
    case 0x08:
        header.cart_type.ram = true;
        // FALL_THROUGH
    case 0x00:
        header.cart_type.controller = CartController::ROM;
        break;
    case 0x03:
        header.cart_type.battery = true;
        // FALL-THROUGH
    case 0x02:
        header.cart_type.ram = true;
        // FALL-THROUGH
    case 0x01:
        header.cart_type.controller = CartController::MBC1;
        break;
    case 0x04:
        header.cart_type.battery = true;
        header.cart_type.ram = true;
        // FALL-THROUGH
    case 0x05:
        header.cart_type.controller = CartController::MBC2;
        break;
    case 0x0d:
        header.cart_type.battery = true;
        // FALL-THROUGH
    case 0x0c:
        header.cart_type.ram = true;
        // FALL-THROUGH
    case 0x0b:
        header.cart_type.controller = CartController::MMM01;
        break;
    case 0x10:
        header.cart_type.ram = true;
        // FALL-THROUGH
    case 0x0f:
        header.cart_type.controller = CartController::MBC3;
        header.cart_type.battery = true;
        header.cart_type.timer = true;
        break;
    case 0x13:
        header.cart_type.battery = true;
        // FALL-THROUGH
    case 0x12:
        header.cart_type.ram = true;
        // FALL-THROUGH
    case 0x11:
        header.cart_type.controller = CartController::MBC3;
        break;
    case 0x1b:
        header.cart_type.battery = true;
        // FALL-THROUGH
    case 0x1a:
        header.cart_type.ram = true;
        // FALL-THROUGH
    case 0x19:
        header.cart_type.controller = CartController::MBC5;
        break;
    case 0x1e:
        header.cart_type.battery = true;
        // FALL-THROUGH
    case 0x1d:
        header.cart_type.ram = true;
        // FALL-THROUGH
    case 0x1c:
        header.cart_type.controller = CartController::MBC5;
        header.cart_type.rumble = true;
        break;
    case 0x20:
        header.cart_type.battery = true;
        header.cart_type.ram = true;
        header.cart_type.controller = CartController::MBC6;
        break;
    case 0x22:
        header.cart_type.accel = true;
        header.cart_type.battery = true;
        header.cart_type.ram = true;
        header.cart_type.controller = CartController::MBC7;
        break;
    case 0xfc:
        header.cart_type.controller = CartController::CAMERA;
        break;
    case 0xfd:
        header.cart_type.controller = CartController::TAMA5;
        break;
    case 0xfe:
        header.cart_type.controller = CartController::HuC3;
        break;
    case 0xff:
        header.cart_type.battery = true;
        header.cart_type.ram = true;
        header.cart_type.controller = CartController::HuC1;
        break;
    }

    header.rom_size = (32 * 1024) << rom.at(0x148);
    switch (rom.at(0x149))
    {
    case 0x00:
        header.ram_size = 0;
        break;
    case 0x01:
        header.ram_size = 2 * 1024;
        break;
    case 0x02:
        header.ram_size = 8 * 1024;
        break;
    case 0x03:
        header.ram_size = 32 * 1024;
        break;
    case 0x04:
        header.ram_size = 128 * 1024;
        break;
    case 0x05:
        header.ram_size = 64 * 1024;
        break;
    default:
        header.unknown_ram_size = true;
        header.ram_size = 0;
    }

    if (header.cart_type.controller == CartController::MBC2)
    {
        assert(header.ram_size == 0);
        header.ram_size = 512;
    }

    header.international = rom.at(0x14a) == 0x01;
    header.licensee = rom.at(0x14b);
    if (header.licensee == 0x33)
    {
        const char* new_l = (const char*)&rom.at(0x144);
        header.new_licensee.assign(new_l, new_l + 1);
    }
    header.version = rom.at(0x14c);
    header.checksum = rom.at(0x14d);
    header.global_checksum = (rom.at(0x14e) << 8) | rom.at(0x14f);

    std::uint8_t checksum = 0;
    for (std::size_t i = 0x134; i < 0x14d; i++)
    {
        checksum = checksum - rom.at(i) - 1;
    }
    header.checksum_passed = checksum == header.checksum;

    if (!header.checksum_passed) return;

    // A value of 0 is caused by shift overflow.
    if (header.rom_size > MAX_ROM_SIZE || header.rom_size == 0)
    {
        rom.resize(MAX_ROM_SIZE);
    }
    else
    {
        rom.resize(header.rom_size);
    }
    
    ram.resize(header.ram_size);

    if (rom.size() > rom_min_size)
    {
        cartFile.read((char*)&rom.at(rom_min_size), rom.size() - rom_min_size);
    }

    std::uint16_t global_checksum = 0;
    for (std::size_t i = 0; i < rom.size(); i++)
    {
        global_checksum += rom.at(i);
    }

    // Take out the checksum bytes themselves.
    global_checksum -= (std::uint16_t)rom.at(0x14e) + rom.at(0x14f);
    header.global_checksum_passed = global_checksum == header.global_checksum;
}

void Cart::reset()
{
    rom_bank_base = 0x4000;
    ram_mode = RamMode::OFF;
    ram_bank_base = 0;

    ctrl_regs.fill(0);
}

std::uint8_t Cart::readROM(std::uint16_t adr)
{
    if (adr < 0x4000) return rom.at(adr);
    return rom.at(rom_bank_base + adr - 0x4000);
}

void Cart::writeROM(std::uint16_t adr, std::uint8_t val)
{
    ctrl_regs.at(adr >> 13) = val;

    switch (header.cart_type.controller)
    {
    case CartController::MBC1:
        {
            rom_bank_base = ctrl_regs[1];
            if (rom_bank_base == 0) rom_bank_base = 1;
            ram_bank_base = 0;

            if (ctrl_regs[3] & 0x01)
            {
                ram_bank_base = ctrl_regs[2] & 0x03;
            }
            else
            {
                rom_bank_base += (ctrl_regs[2] & 0x03) << 8;
            }

            rom_bank_base *= 0x4000;
            ram_bank_base *= 0x2000;
        }
    case CartController::ROM:
        ram_mode = ((ctrl_regs[0] & 0x0f) == 0x0a) ? RamMode::ON : RamMode::OFF;
        break;
    default:
        assert(false);
    }
}

std::uint8_t Cart::readRAM(std::uint16_t adr)
{
    if (ram.empty()) return 0xff;

    switch (ram_mode)
    {
    case Cart::RamMode::OFF:
        return 0xff;
    case Cart::RamMode::HALF:
        return 0xf0 | ram.at(ram_bank_base + adr);
    case Cart::RamMode::ON:
        return ram.at(ram_bank_base + adr);
    }

    assert(false);
    return 0xff;
}

void Cart::writeRAM(std::uint16_t adr, std::uint8_t val)
{
    if (ram.empty()) return;

    switch (ram_mode)
    {
    case Cart::RamMode::OFF:
        return;
        break;
    case Cart::RamMode::HALF:
        ram.at(ram_bank_base + adr) = val & 0x0f;
        break;
    case Cart::RamMode::ON:
        ram.at(ram_bank_base + adr) = val & 0xf0;
        break;
    }

    assert(false);
}
