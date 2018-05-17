//
// Copyright (c) 2010-2018 Christoph Sprenger
//
// This file is part of AGE ("Another Gameboy Emulator").
// <https://gitlab.com/csprenger/AGE>
//
// AGE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// AGE is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with AGE.  If not, see <http://www.gnu.org/licenses/>.
//

#include "age_gb_emulator_impl.hpp"



bool age::gb_emulator_impl::is_cgb() const
{
    return m_memory.is_cgb();
}

age::gb_test_info age::gb_emulator_impl::get_test_info() const
{
    return m_cpu.get_test_info();
}

age::uint8_vector age::gb_emulator_impl::get_persistent_ram() const
{
    return m_memory.get_persistent_ram();
}

void age::gb_emulator_impl::set_persistent_ram(const uint8_vector &source)
{
    m_memory.set_persistent_ram(source);
}

void age::gb_emulator_impl::set_buttons_down(uint buttons)
{
    m_joypad.set_buttons_down(buttons);
}

void age::gb_emulator_impl::set_buttons_up(uint buttons)
{
    m_joypad.set_buttons_up(buttons);
}



age::uint64 age::gb_emulator_impl::inner_emulate(uint64 min_cycles_to_emulate)
{
    uint starting_cycle = m_core.get_oscillation_cycle();
    uint cycle_to_go = starting_cycle + min_cycles_to_emulate;

    while (m_core.get_oscillation_cycle() < cycle_to_go)
    {
        m_bus.handle_events(); // may change the current gb_mode
        switch (m_core.get_mode())
        {
            case gb_mode::halted:
                m_core.oscillate_cpu_cycle();
                break;

            case gb_mode::cpu_active:
                m_cpu.emulate_instruction();
                break;

            case gb_mode::dma:
                AGE_ASSERT(m_core.is_cgb());
                m_bus.handle_dma();
                m_core.finish_dma();
                break;
        }
    }

    m_sound.generate_samples();

    uint cycles_emulated = m_core.get_oscillation_cycle() - starting_cycle;
    return cycles_emulated;
}



std::string age::gb_emulator_impl::inner_get_emulator_title() const
{
    return m_memory.get_cartridge_title();
}



//---------------------------------------------------------
//
//   object creation & destruction
//
//---------------------------------------------------------

age::gb_emulator_impl::gb_emulator_impl(const uint8_vector &rom,
                                        bool force_dmg,
                                        bool dmg_green,
                                        pcm_vector &pcm_vec,
                                        screen_buffer &screen_buf)
    : m_memory(rom, force_dmg),
      m_core(m_memory.is_cgb()),
      m_sound(m_core, pcm_vec),
      m_lcd(m_core, m_memory, screen_buf, dmg_green),
      m_timer(m_core),
      m_joypad(m_core),
      m_serial(m_core),
      m_bus(m_core, m_memory, m_sound, m_lcd, m_timer, m_joypad, m_serial),
      m_cpu(m_core, m_bus)
{
}