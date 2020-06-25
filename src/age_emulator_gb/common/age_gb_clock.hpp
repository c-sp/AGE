//
// Copyright 2019 Christoph Sprenger
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#ifndef AGE_GB_CLOCK_HPP
#define AGE_GB_CLOCK_HPP

//!
//! \file
//!

#include <age_debug.hpp>
#include <age_types.hpp>

#include "age_gb_device.hpp"



namespace age
{

constexpr int gb_no_clock_cycle = -1;
constexpr int gb_clock_cycles_per_second = 4194304;



class gb_clock
{
public:

    gb_clock(const gb_device &device);

    //! \brief Get the current 4Mhz cycle.
    //!
    //! This clock runs at 4Mhz regardless of the current
    //! Gameboy Color speed setting.
    int get_clock_cycle() const;
    int8_t get_machine_cycle_clocks() const;
    bool is_double_speed() const;

    void tick_machine_cycle();
    void tick_2_clock_cycles();
    void set_back_clock(int clock_cycle_offset);

    bool trigger_speed_change();
    uint8_t read_key1() const;
    void write_key1(uint8_t value);

private:

    int m_clock_cycle = 0;
    int8_t m_machine_cycle_clocks = 4;
    uint8_t m_key1 = 0x7E;
};

} // namespace age



//! \todo remove this once it's no longer used (no negative cycle numbers!)
#define AGE_GB_SET_BACK_CLOCK_OVERFLOW(value, offset) \
    if (value != gb_no_clock_cycle) \
    { \
        /*AGE_LOG("set back " << #value << ": " << value << " -> " << (value - offset));*/ \
        AGE_ASSERT(offset >= gb_clock_cycles_per_second); \
        AGE_ASSERT(0 == (offset & (gb_clock_cycles_per_second - 1))); \
        value -= offset; \
    } \
    else (void)0 // no-op to force semicolon when using this macro

#define AGE_GB_SET_BACK_CLOCK(value, offset) \
    AGE_ASSERT((value == gb_no_clock_cycle) || (value >= offset)); \
    AGE_GB_SET_BACK_CLOCK_OVERFLOW(value, offset)



#define AGE_GB_CLOG_CLOCK 0
#define AGE_GB_CLOG_CPU 0
#define AGE_GB_CLOG_DIV 0
#define AGE_GB_CLOG_EVENTS 0
#define AGE_GB_CLOG_INTERRUPTS 0
#define AGE_GB_CLOG_SERIAL 0
#define AGE_GB_CLOG_TIMER 0

#define _CONCAT(a, ...) a ## __VA_ARGS__
#define AGE_GB_CLOG(type) _CONCAT(AGE_GB_CLOG_, type)
#define AGE_GB_CLOG_0(log)
#define AGE_GB_CLOG_1(log) AGE_LOG("clock " << m_clock.get_clock_cycle() << " - " << log)

//! \todo replacae AGE_GB_CLOCK_LOG with AGE_GB_CLOG
#ifdef AGE_DEBUG
#define AGE_GB_CLOCK_LOG(x) AGE_LOG("clock " << m_clock.get_clock_cycle() << " - " << x)
#else
#define AGE_GB_CLOCK_LOG(x)
#endif



#endif // AGE_GB_CLOCK_HPP