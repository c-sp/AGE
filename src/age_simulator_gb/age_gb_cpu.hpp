//
// Copyright (c) 2010-2017 Christoph Sprenger
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

#ifndef AGE_GB_CPU_HPP
#define AGE_GB_CPU_HPP

//!
//! \file
//!

#include "age_gb_bus.hpp"



namespace age
{

class gb_cpu : public non_copyable
{
public:

    void simulate_instruction();

    gb_cpu(gb_core &core, gb_bus &bus);

private:

    gb_core &m_core;
    gb_bus &m_bus;

    uint m_zero_indicator = 0;
    uint m_carry_indicator = 0;
    uint m_hcs_flags = 0; //!< first operand and additional flags of the last instruction relevant for subtract- and half-carry-flag
    uint m_hcs_operand = 0; //!< second operand of the last instruction relevant for subtract- and half-carry-flag

    bool m_next_byte_twice = false;
    uint16 m_pc = 0;
    uint16 m_sp = 0;

    uint8 m_a = 0;
    uint8 m_b = 0;
    uint8 m_c = 0;
    uint8 m_d = 0;
    uint8 m_e = 0;
    uint8 m_h = 0;
    uint8 m_l = 0;
};

} // namespace age



#endif // AGE_GB_CPU_HPP
