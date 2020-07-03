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

#include <algorithm>

#include <age_debug.hpp>

#include "age_gb_lcd.hpp"

namespace
{

int add_total_frames(int clk_last, int clk_current)
{
    AGE_ASSERT(clk_last <= clk_current);

    int clk_diff = clk_current - clk_last;
    int frames = 1 + clk_diff / age::gb_clock_cycles_per_frame;

    return frames * age::gb_clock_cycles_per_frame;
}

}



age::gb_lcd_irqs::gb_lcd_irqs(const gb_clock &clock,
                              const gb_lcd_scanline &scanline,
                              gb_events &events,
                              gb_interrupt_trigger &interrupts)
    : m_clock(clock),
      m_scanline(scanline),
      m_events(events),
      m_interrupts(interrupts)
{
    lcd_on(0); // schedule inital irq events
}



age::uint8_t age::gb_lcd_irqs::read_stat() const {
    return m_stat;
}

void age::gb_lcd_irqs::write_stat(uint8_t value, int scx)
{
    m_stat = 0x80 | (value & ~(gb_stat_modes | gb_stat_ly_match));

    if (m_scanline.current_scanline() != gb_scanline_lcd_off)
    {
        schedule_irq_lyc();
        schedule_irq_mode2();
        schedule_irq_mode0(scx);
    }
}



void age::gb_lcd_irqs::lcd_on(int scx)
{
    schedule_irq_vblank();
    schedule_irq_lyc();
    schedule_irq_mode2();
    schedule_irq_mode0(scx);
}

void age::gb_lcd_irqs::lcd_off()
{
    m_clk_next_irq_vblank = gb_no_clock_cycle;
    m_clk_next_irq_lyc = gb_no_clock_cycle;
    m_clk_next_irq_mode2 = gb_no_clock_cycle;
    m_clk_next_irq_mode0 = gb_no_clock_cycle;

    m_events.remove_event(gb_event::lcd_interrupt_vblank);
    m_events.remove_event(gb_event::lcd_interrupt_lyc);
    m_events.remove_event(gb_event::lcd_interrupt_mode2);
    m_events.remove_event(gb_event::lcd_interrupt_mode0);
}

void age::gb_lcd_irqs::set_back_clock(int clock_cycle_offset)
{
    AGE_GB_SET_BACK_CLOCK(m_clk_next_irq_vblank, clock_cycle_offset);
    AGE_GB_SET_BACK_CLOCK(m_clk_next_irq_lyc, clock_cycle_offset);
    AGE_GB_SET_BACK_CLOCK(m_clk_next_irq_mode2, clock_cycle_offset);
    AGE_GB_SET_BACK_CLOCK(m_clk_next_irq_mode0, clock_cycle_offset);
}



//---------------------------------------------------------
//
//   v-blank irq
//
//---------------------------------------------------------

void age::gb_lcd_irqs::trigger_irq_vblank()
{
    int clk_current = m_clock.get_clock_cycle();

    AGE_ASSERT(m_clk_next_irq_vblank != gb_no_clock_cycle);
    AGE_ASSERT(m_clk_next_irq_vblank <= clk_current);

    m_interrupts.trigger_interrupt(gb_interrupt::vblank);
    if (m_stat & gb_stat_irq_mode1)
    {
        m_interrupts.trigger_interrupt(gb_interrupt::lcd);
    }
    AGE_GB_CLOG_IRQS("    * v-blank IRQ was scheduled for clock cycle "
                     << m_clk_next_irq_vblank);

    m_clk_next_irq_vblank += add_total_frames(m_clk_next_irq_vblank, clk_current);
    int clk_diff = m_clk_next_irq_vblank - clk_current;

    AGE_GB_CLOG_IRQS("    * next v-blank IRQ in " << clk_diff
                     << " clock cycles (" << m_clk_next_irq_vblank << ")");

    m_events.schedule_event(gb_event::lcd_interrupt_vblank, clk_diff);
}



void age::gb_lcd_irqs::schedule_irq_vblank()
{
    int clk_current = m_clock.get_clock_cycle();
    int clk_frame_start = m_scanline.clk_frame_start();
    AGE_ASSERT(clk_frame_start != gb_no_clock_cycle);
    AGE_ASSERT(clk_frame_start <= clk_current);

    m_clk_next_irq_vblank = clk_frame_start
            + gb_screen_height * gb_clock_cycles_per_scanline;
    AGE_ASSERT(m_clk_next_irq_vblank > clk_current);

    m_events.schedule_event(gb_event::lcd_interrupt_vblank, m_clk_next_irq_vblank - clk_current);
}



//---------------------------------------------------------
//
//   lyc irq
//
//---------------------------------------------------------

void age::gb_lcd_irqs::lyc_update()
{
    if (m_scanline.current_scanline() != gb_scanline_lcd_off)
    {
        schedule_irq_lyc();
    }
}



void age::gb_lcd_irqs::trigger_irq_lyc()
{
    int clk_current = m_clock.get_clock_cycle();

    AGE_ASSERT(m_clk_next_irq_lyc != gb_no_clock_cycle);
    AGE_ASSERT(m_clk_next_irq_lyc <= clk_current);

    m_interrupts.trigger_interrupt(gb_interrupt::lcd);
    AGE_GB_CLOG_IRQS("    * lcd IRQ (LYC) was scheduled for clock cycle "
                     << m_clk_next_irq_lyc);

    m_clk_next_irq_lyc += add_total_frames(m_clk_next_irq_lyc, clk_current);
    int clk_diff = m_clk_next_irq_lyc - clk_current;

    AGE_GB_CLOG_IRQS("    * next LYC IRQ in " << clk_diff
                     << " clock cycles (" << m_clk_next_irq_lyc << ")");

    m_events.schedule_event(gb_event::lcd_interrupt_lyc, clk_diff);
}



void age::gb_lcd_irqs::schedule_irq_lyc()
{
    if (!(m_stat & gb_stat_irq_ly_match) || (m_scanline.m_lyc >= gb_scanline_count))
    {
        m_clk_next_irq_lyc = gb_no_clock_cycle;
        m_events.remove_event(gb_event::lcd_interrupt_lyc);
        return;
    }

    // schedule next LYC irq
    int clk_frame_start = m_scanline.clk_frame_start();
    AGE_ASSERT(clk_frame_start != gb_no_clock_cycle);

    m_clk_next_irq_lyc = clk_frame_start + m_scanline.m_lyc * gb_clock_cycles_per_scanline;

    int clk_current = m_clock.get_clock_cycle();
    if (m_clk_next_irq_lyc <= clk_current)
    {
        m_clk_next_irq_lyc += gb_clock_cycles_per_frame;
    }

    AGE_ASSERT(m_clk_next_irq_lyc > clk_current);
    m_events.schedule_event(gb_event::lcd_interrupt_lyc, m_clk_next_irq_lyc - clk_current);
}



//---------------------------------------------------------
//
//   mode 2 irq
//
//---------------------------------------------------------

void age::gb_lcd_irqs::trigger_irq_mode2()
{
    int clk_current = m_clock.get_clock_cycle();

    AGE_ASSERT(m_clk_next_irq_mode2 != gb_no_clock_cycle);
    AGE_ASSERT(m_clk_next_irq_mode2 <= clk_current);

    m_interrupts.trigger_interrupt(gb_interrupt::lcd);
    AGE_GB_CLOG_IRQS("    * lcd IRQ (mode 2) was scheduled for clock cycle "
                     << m_clk_next_irq_mode2);

    // handle all the details in schedule_irq_mode2()
    // (irq for specific scanlines at specific clock cycles)
    schedule_irq_mode2();

    AGE_GB_CLOG_IRQS("    * next mode 2 IRQ in " << (m_clk_next_irq_mode2 - clk_current)
                     << " clock cycles (" << m_clk_next_irq_mode2 << ")");
}



void age::gb_lcd_irqs::schedule_irq_mode2()
{
    if (!(m_stat & gb_stat_irq_mode2))
    {
        m_clk_next_irq_mode2 = gb_no_clock_cycle;
        m_events.remove_event(gb_event::lcd_interrupt_mode2);
        return;
    }

    int scanline, scanline_clks;
    m_scanline.calculate_scanline(scanline, scanline_clks);
    int m2_scanline = scanline % gb_scanline_count; // scanline during current frame

    AGE_ASSERT(m2_scanline < gb_scanline_count);
    AGE_ASSERT(scanline_clks < gb_clock_cycles_per_scanline);

    // scanline and m2_scanline are supposed to be the scanline DURING WHICH
    // the irq is triggered.
    // They are NOT supposed to be the scanline FOR WHICH the irq is triggered.
    //
    // Mode 2 irq is triggered 2 cycles before a scanline begins.
    // If we're too late, jump to the next scanline.
    // (not for scanline 0 though)
    if (scanline_clks >= gb_clock_cycles_per_scanline - 2)
    {
        ++scanline;
        ++m2_scanline;
    }

    // There is a mode-2-irq triggered 2 cycles before v-blank.
    // After that the next mode-2-irq is triggered right before scanline 0.
    if (m2_scanline >= gb_screen_height)
    {
        // skip v-blank and trigger at the end of scanline 153
        if (m2_scanline < gb_scanline_count)
        {
            int scanline_add = gb_scanline_count - 1 - m2_scanline;
            scanline += scanline_add;
        }
        // else trigger during scanline 0
    }

    // calculate next mode 2 irq clock cycle
    int clk_current = m_clock.get_clock_cycle();
    int clk_frame_start = m_scanline.clk_frame_start();

    m_clk_next_irq_mode2 = clk_frame_start
            + ((scanline + 1) * gb_clock_cycles_per_scanline)
            // the mode 2 irq for scanline 0 is not triggered early
            - ((scanline == 153) ? 0 : 2);

    AGE_ASSERT(m_clk_next_irq_mode2 > clk_current);

    m_events.schedule_event(gb_event::lcd_interrupt_mode2, m_clk_next_irq_mode2 - clk_current);
}



//---------------------------------------------------------
//
//   mode 0 irq
//
//---------------------------------------------------------

void age::gb_lcd_irqs::trigger_irq_mode0(int scx)
{
    int clk_current = m_clock.get_clock_cycle();

    AGE_ASSERT(m_clk_next_irq_mode0 != gb_no_clock_cycle);
    AGE_ASSERT(m_clk_next_irq_mode0 <= clk_current);

    m_interrupts.trigger_interrupt(gb_interrupt::lcd);
    AGE_GB_CLOG_IRQS("    * lcd IRQ (mode 0) was scheduled for clock cycle "
                     << m_clk_next_irq_mode0);

    // handle all the details in schedule_irq_mode0()
    // (irq for specific scanlines at specific clock cycles)
    schedule_irq_mode0(scx);

    AGE_GB_CLOG_IRQS("    * next mode 0 IRQ in " << (m_clk_next_irq_mode0 - clk_current)
                     << " clock cycles (" << m_clk_next_irq_mode0 << ")");
}



void age::gb_lcd_irqs::schedule_irq_mode0(int scx)
{
    if (!(m_stat & gb_stat_irq_mode0))
    {
        m_clk_next_irq_mode0 = gb_no_clock_cycle;
        m_events.remove_event(gb_event::lcd_interrupt_mode0);
        return;
    }

    int scanline, scanline_clks;
    m_scanline.calculate_scanline(scanline, scanline_clks);
    int m0_scanline = scanline % gb_scanline_count; // scanline during current frame

    AGE_ASSERT(m0_scanline < gb_scanline_count);
    AGE_ASSERT(scanline_clks < gb_clock_cycles_per_scanline);

    // mode 0 already began, skip this scanline
    //! \todo too simple: mode 3 timing also depends on sprites & window
    int m3_end = 80 + 172 + (scx & 7);
    if (scanline_clks >= m3_end)
    {
        ++scanline;
        ++m0_scanline;
    }

    // skip v-blank
    if (m0_scanline >= gb_screen_height)
    {
        int scanline_add = gb_scanline_count - m0_scanline;
        scanline += scanline_add;
    }

    // calculate next mode 0 irq clock cycle
    int clk_current = m_clock.get_clock_cycle();
    int clk_frame_start = m_scanline.clk_frame_start();

    m_clk_next_irq_mode0 = clk_frame_start
            + (scanline * gb_clock_cycles_per_scanline)
            + m3_end;

    AGE_ASSERT(m_clk_next_irq_mode0 > clk_current);

    m_events.schedule_event(gb_event::lcd_interrupt_mode0, m_clk_next_irq_mode0 - clk_current);
}