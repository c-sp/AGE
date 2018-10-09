//
// Copyright 2018 Christoph Sprenger
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

#ifndef AGE_GB_SOUND_HPP
#define AGE_GB_SOUND_HPP

//!
//! \file
//!

#include <array>

#include <age_debug.hpp>
#include <age_types.hpp>
#include <pcm/age_pcm_sample.hpp>

#include "age_gb_core.hpp"
#include "age_gb_sound_utilities.hpp"



namespace age
{

typedef gb_frequency_sweep<gb_volume_envelope<gb_duty_source>> gb_sound_channel1;
typedef gb_volume_envelope<gb_duty_source> gb_sound_channel2;
typedef gb_wave_source gb_sound_channel3;
typedef gb_volume_envelope<gb_noise_source> gb_sound_channel4;



constexpr unsigned gb_channel_1 = 0;
constexpr unsigned gb_channel_2 = 1;
constexpr unsigned gb_channel_3 = 2;
constexpr unsigned gb_channel_4 = 3;

constexpr uint8_t gb_channel_bit(unsigned channel)
{
    return static_cast<uint8_t>(0x01 << channel);
}



class gb_sound
{
    AGE_DISABLE_COPY(gb_sound);

public:

    gb_sound(const gb_core &core, pcm_vector &samples);

    uint8_t read_nr10() const;
    uint8_t read_nr11() const;
    uint8_t read_nr12() const;
    uint8_t read_nr13() const;
    uint8_t read_nr14() const;
    uint8_t read_nr21() const;
    uint8_t read_nr22() const;
    uint8_t read_nr23() const;
    uint8_t read_nr24() const;
    uint8_t read_nr30() const;
    uint8_t read_nr31() const;
    uint8_t read_nr32() const;
    uint8_t read_nr33() const;
    uint8_t read_nr34() const;
    uint8_t read_nr41() const;
    uint8_t read_nr42() const;
    uint8_t read_nr43() const;
    uint8_t read_nr44() const;
    uint8_t read_nr50() const;
    uint8_t read_nr51() const;
    uint8_t read_nr52();

    void write_nr10(uint8_t value);
    void write_nr11(uint8_t value);
    void write_nr12(uint8_t value);
    void write_nr13(uint8_t value);
    void write_nr14(uint8_t value);
    void write_nr21(uint8_t value);
    void write_nr22(uint8_t value);
    void write_nr23(uint8_t value);
    void write_nr24(uint8_t value);
    void write_nr30(uint8_t value);
    void write_nr31(uint8_t value);
    void write_nr32(uint8_t value);
    void write_nr33(uint8_t value);
    void write_nr34(uint8_t value);
    void write_nr41(uint8_t value);
    void write_nr42(uint8_t value);
    void write_nr43(uint8_t value);
    void write_nr44(uint8_t value);
    void write_nr50(uint8_t value);
    void write_nr51(uint8_t value);
    void write_nr52(uint8_t value);

    uint8_t read_wave_ram(unsigned offset);
    void write_wave_ram(unsigned offset, uint8_t value);

    void update_state();
    void set_back_cycles();



private:

    const gb_core &m_core;
    const bool m_is_cgb;

    pcm_vector &m_samples;
    int m_sample_count = 0;

    int m_sample_next_apu_event = 0;
    int8_t m_next_frame_sequencer_step = 0;
    bool m_delayed_disable_c1 = false;
    bool m_skip_frame_sequencer_step = false;

    // channel control

    uint8_t m_nr50 = 0x77, m_nr51 = 0xF3;
    bool m_master_on = true;

    // channels

    std::array<gb_length_counter, 4> m_length_counter = {{ {0x3F}, {0x3F}, {0xFF}, {0x3F} }};

    uint8_t m_nr10 = 0, m_nr11 = 0x80, m_nr14 = 0;
    gb_sound_channel1 m_c1;

    uint8_t m_nr21 = 0, m_nr24 = 0;
    gb_sound_channel2 m_c2;

    uint8_t m_nr30 = 0, m_nr32 = 0, m_nr34 = 0;
    uint8_array<16> m_c3_wave_ram;
    gb_sound_channel3 m_c3;

    uint8_t m_nr44 = 0;
    gb_sound_channel4 m_c4;



    // private methods

    bool inc_period() const;
    int apu_event();
    void generate_samples(int sample_count);
    void set_wave_ram_byte(unsigned offset, uint8_t value);

    // channel template methods

    template<unsigned>
    inline void deactivate_channel()
    {
        AGE_ASSERT(false); // implementation for each channel see below
    }

    template<unsigned channel>
    inline void length_counter_tick()
    {
        if (m_length_counter[channel].decrement_length_counter())
        {
            deactivate_channel<channel>();
        }
    }

    template<unsigned channel>
    inline void length_counter_write_nrX4(uint8_t value)
    {
        gb_length_counter &lc = m_length_counter[channel];

        bool deactivate = lc.init_length_counter(value, m_next_frame_sequencer_step & 1);
        if (deactivate)
        {
            deactivate_channel<channel>();
        }
    }

    template<unsigned channel>
    inline void length_counter_write_nrX1(uint8_t value)
    {
        // length counter always writable for DMG
        if (m_master_on || !m_is_cgb)
        {
            update_state();
            m_length_counter[channel].write_nrX1(value);
        }
    }
};



template<> inline void gb_sound::deactivate_channel<gb_channel_1>()
{
    m_c1.deactivate();
}

template<> inline void gb_sound::deactivate_channel<gb_channel_2>()
{
    m_c2.deactivate();
}

template<> inline void gb_sound::deactivate_channel<gb_channel_3>()
{
    m_c3.deactivate();
}

template<> inline void gb_sound::deactivate_channel<gb_channel_4>()
{
    m_c4.deactivate();
}

} // namespace age



#endif // AGE_GB_SOUND_HPP
