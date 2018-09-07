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

#ifndef AGE_GB_SOUND_UTILITIES_HPP
#define AGE_GB_SOUND_UTILITIES_HPP

//!
//! \file
//!

#include <algorithm> // std::min

#include <age_debug.hpp>
#include <age_types.hpp>
#include <pcm/age_pcm_sample.hpp>



namespace age
{

constexpr int8_t gb_sample_cycle_shift = 1; // 2097152 samples per second for easier emulation (will be downsampled later on)
constexpr int gb_cycles_per_sample = 1 << gb_sample_cycle_shift;
constexpr int gb_cycle_sample_mask = ~(gb_cycles_per_sample - 1);

constexpr uint8_t gb_nrX4_initialize = 0x80;



template<typename TYPE>
class gb_sample_generator
{
public:

    int get_samples_next_item() const
    {
        return m_samples_next_item;
    }

    bool get_last_sample_new_item()
    {
        return m_last_sample_new_item;
    }

    void set_channel_multiplier(uint32_t channel_multiplier)
    {
        AGE_ASSERT((channel_multiplier & 0xFFFF) <= 8);
        AGE_ASSERT((channel_multiplier >> 16) <= 8);
        m_channel_multiplier = channel_multiplier;
        calculate_sample();
    }

    void set_volume(uint8_t volume)
    {
        AGE_ASSERT(volume <= 60);
        m_volume = volume;
        calculate_sample();
    }

    void generate_samples(pcm_vector &buffer, int buffer_index, int samples_to_generate)
    {
        AGE_ASSERT((buffer_index >= 0) && (samples_to_generate > 0));
        AGE_ASSERT(int_max - samples_to_generate >= buffer_index);
        AGE_ASSERT(m_samples_per_item > 0);
        AGE_ASSERT(m_samples_next_item >= 0); //! \todo zero only for the very first sample

        // we assume to generate at least one sample
        m_last_sample_new_item = false;

        for (int samples_remaining = samples_to_generate; samples_remaining > 0; )
        {
            // write the sample until we have to calculate the next item
            int samples = std::min(samples_remaining, m_samples_next_item);

            for (int max = buffer_index + samples; buffer_index < max; ++buffer_index)
            {
                buffer[buffer_index].m_stereo_sample += m_current_sample;
            }

            samples_remaining -= samples;
            m_samples_next_item -= samples;

            // check for next wave pattern item
            if (m_samples_next_item == 0)
            {
                m_last_sample_new_item = true;
                m_samples_next_item = m_samples_per_item;
                m_current_item = static_cast<TYPE*>(this)->next_item();
                calculate_sample();
            }
        }

        m_last_sample_new_item &= m_samples_next_item == m_samples_per_item;
    }

protected:

    void set_samples_per_item(int samples_per_item)
    {
        AGE_ASSERT(samples_per_item > 0);
        m_samples_per_item = samples_per_item;
    }

    void offset_item(int sample_offset)
    {
        m_samples_next_item = m_samples_per_item + sample_offset;
    }

private:

    void calculate_sample()
    {
        AGE_ASSERT(m_current_item <= 15);
        AGE_ASSERT(m_volume <= 60);
        //
        // divider:
        //
        //   / 15  ->  max amplitude
        //   / 32  ->  4 channels, s0x volume up to 8
        //   / 60  ->  combining volume 1-15 (channels 1,2,4) and volume 0, 1, 0.5, 0.25 (channel 3)
        //
        int value = (2 * m_current_item - 15) * int16_t_max * m_volume;
        value /= 15 * 32 * 60;
        m_current_sample = value * m_channel_multiplier;
    }

    int m_samples_per_item = 0;
    int m_samples_next_item = 0;
    bool m_last_sample_new_item = false;

    uint32_t m_channel_multiplier = 0;
    uint8_t m_volume = 15;

    uint8_t m_current_item = 0;
    uint32_t m_current_sample = 0;
};





template<typename TYPE>
class gb_volume_sweep : public TYPE
{
public:

    uint8_t read_nrX2() const
    {
        return m_nrX2;
    }

    bool write_nrX2(uint8_t nrX2)
    {
        // "zombie" update
        size_t volume = m_volume;
        if ((m_period == 0) && (m_period_counter > 0))
        {
            ++volume;
        }
        else if (!m_sweep_up)
        {
            volume += 2;
        }

        if (m_sweep_up != ((nrX2 & 0x08) > 0))
        {
            volume = 16 - volume;
        }

        volume &= 0x0F;
        switch_volume(volume);

        // store new value
        m_nrX2 = nrX2;
        m_period = m_nrX2 & 0x07;
        m_sweep_up = (m_nrX2 & 0x08) > 0;

        return channel_off();
    }

    bool init_volume_sweep()
    {
        m_period_counter = (m_period == 0) ? 8 : m_period;
        switch_volume(m_nrX2 >> 4);
        return channel_off();
    }

    void sweep_volume()
    {
        if (m_period_counter > 0)
        {
            --m_period_counter;
            if (m_period_counter == 0)
            {
                if (m_period > 0)
                {
                    if (sweep())
                    {
                        m_period_counter = m_period;
                    }
                    // else m_period_counter remains unchanged (= 0) -> stop sweeping
                }
                // if we cannot sweep at the moment, retry later
                else
                {
                    m_period_counter = 8;
                }
            }
        }
    }

    void reset_volume_sweep()
    {
        // terminate any ongoing sweep
        m_period_counter = 0;
    }

private:

    bool channel_off() const
    {
        bool result = (m_nrX2 & 0xF8) == 0;
        return result;
    }

    void switch_volume(size_t new_volume)
    {
        AGE_ASSERT(new_volume < 0x10);
        if (m_volume != new_volume)
        {
            m_volume = new_volume;
            TYPE::set_volume(m_volume * 4);
        }
    }

    bool sweep()
    {
        size_t volume = m_sweep_up ? m_volume + 1 : m_volume - 1;

        bool adjust_volume = volume < 0x10;
        if (adjust_volume)
        {
            switch_volume(volume);
        }

        return adjust_volume;
    }

    uint8_t m_nrX2 = 0;

    bool m_sweep_up = false;
    size_t m_period = 0;
    size_t m_period_counter = 0;
    size_t m_volume = 0;
};





template<typename TYPE>
class gb_frequency_sweep : public TYPE
{
public:

    bool write_nrX0(uint8_t nrX0)
    {
        m_period = (nrX0 >> 4) & 7;
        m_shift = nrX0 & 7;
        m_sweep_up = (nrX0 & 8) == 0;

        bool deactivate = m_swept_down && m_sweep_up;
        return deactivate;
    }

    bool init_frequency_sweep()
    {
        m_swept_down = false;
        m_frequency_bits = TYPE::get_frequency_bits();

        // schedule event, if period or shift are set
        m_sweep_enabled = (m_period + m_shift) > 0;
        if (m_sweep_enabled)
        {
            set_period_counter();
        }

        // check for channel deactivation only, if shift is not zero
        bool deactivate = false;
        if (m_shift > 0)
        {
            size_t frequency_bits = sweep_frequency_bits();
            deactivate = invalid_frequency_bits(frequency_bits);
        }

        return deactivate;
    }

    bool sweep_frequency()
    {
        bool deactivate = false;

        if (m_sweep_enabled)
        {
            AGE_ASSERT(m_period_counter > 0);
            --m_period_counter;
            if (m_period_counter == 0)
            {
                if (m_period > 0)
                {
                    size_t frequency_bits = sweep_frequency_bits();
                    deactivate = invalid_frequency_bits(frequency_bits);

                    if (!deactivate && (m_shift > 0))
                    {
                        m_frequency_bits = frequency_bits;
                        TYPE::set_frequency_bits(m_frequency_bits);

                        size_t frequency_bits = sweep_frequency_bits();
                        deactivate = invalid_frequency_bits(frequency_bits);
                    }
                }
                set_period_counter();
            }
        }

        return deactivate;
    }

private:

    bool invalid_frequency_bits(size_t frequency_bits) const
    {
        return frequency_bits > 2047;
    }

    size_t sweep_frequency_bits()
    {
        size_t shifted = m_frequency_bits >> m_shift;
        size_t result = m_frequency_bits;

        if (m_sweep_up)
        {
            result += shifted;
        }
        else
        {
            result -= shifted;
            m_swept_down = true;
        }

        return result;
    }

    void set_period_counter()
    {
        m_period_counter = (m_period == 0) ? 8 : m_period;
    }

    size_t m_frequency_bits = 0;
    size_t m_period = 0;
    size_t m_shift = 0;
    bool m_sweep_up = false;

    bool m_sweep_enabled = false;
    bool m_swept_down = false;
    size_t m_period_counter = 0;
};





class gb_wave_generator : public gb_sample_generator<gb_wave_generator>
{
public:

    gb_wave_generator();
    gb_wave_generator(size_t frequency_counter_shift, uint8_t wave_pattern_index_mask);

    size_t get_frequency_bits() const;
    uint8_t get_wave_pattern_index() const;

    void set_frequency_bits(size_t frequency_bits);
    void set_low_frequency_bits(uint8_t nrX3);
    void set_high_frequency_bits(uint8_t nrX4);

    void reset_wave_pattern_index();
    void set_wave_pattern_byte(unsigned offset, uint8_t value);
    void set_wave_pattern_duty(uint8_t nrX1);

    uint8_t next_item();

private:

    const size_t m_frequency_counter_shift;
    size_t m_frequency_bits = 0;

    const uint8_t m_index_mask;
    uint8_t m_index = 0;

    uint8_array<32> m_wave_pattern;
};





class gb_noise_generator : public gb_sample_generator<gb_noise_generator>
{
public:

    uint8_t read_nrX3() const;

    void write_nrX3(uint8_t nrX3);
    void init_generator();

    uint8_t next_item();

private:

    uint8_t m_nrX3 = 0;
    bool m_7steps = false;
    bool m_allow_shift = true;
    uint16_t m_lfsr = 0;
};





class gb_length_counter
{
public:

    gb_length_counter(int16_t counter_mask);

    void write_nrX1(uint8_t nrX1);
    bool write_nrX4(uint8_t nrX4, bool next_frame_sequencer_step_odd);
    bool tick();

private:

    const int16_t m_counter_mask;

    int16_t m_counter = 0;
    bool m_counter_enabled = false;
};

} // namespace age



#endif // AGE_GB_SOUND_UTILITIES_HPP
