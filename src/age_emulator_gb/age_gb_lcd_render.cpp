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

#include <algorithm> // std::fill

#include <age_debug.hpp>

#include "age_gb_lcd.hpp"

namespace {

constexpr uint8_t bg_tile_palette = 0x07;
constexpr uint8_t bg_tile_vram_bank = 0x08;
constexpr uint8_t bg_tile_flip_x = 0x20;
constexpr uint8_t bg_tile_flip_y = 0x40;
constexpr uint8_t bg_tile_priority = 0x80;

void calculate_xflip(age::uint8_array<256> &xflip)
{
    for (unsigned byte = 0; byte < 256; ++byte)
    {
        age::uint8_t flip_byte = 0;

        for (unsigned bit = 0x01, flip_bit = 0x80;
             bit < 0x100;
             bit += bit, flip_bit >>= 1)
        {
            if ((byte & bit) > 0)
            {
                flip_byte |= flip_bit;
            }
        }

        xflip[byte] = flip_byte;
    }
}

}



age::gb_lcd_render::gb_lcd_render(const gb_device &device,
                                  const gb_lcd_palettes &palettes,
                                  gb_lcd_sprites &sprites,
                                  const uint8_t *video_ram,
                                  screen_buffer &screen_buffer)
    : m_device(device),
      m_palettes(palettes),
      m_sprites(sprites),
      m_screen_buffer(screen_buffer),
      m_video_ram(video_ram)
{
    calculate_xflip(m_xflip_cache);
    set_lcdc(0x91);
}



age::uint8_t age::gb_lcd_render::get_lcdc() const
{
    return m_lcdc;
}

void age::gb_lcd_render::set_lcdc(int lcdc)
{
    m_lcdc = lcdc;

    m_bg_tile_map_offset = (lcdc & gb_lcdc_bg_map) ? 0x1C00 : 0x1800;
    m_win_tile_map_offset = (lcdc & gb_lcdc_win_map) ? 0x1C00 : 0x1800;

    m_tile_data_offset = (lcdc & gb_lcdc_bg_win_data) ? 0x0000 : 0x0800;
    m_tile_xor = (lcdc & gb_lcdc_bg_win_data) ? 0 : 0x80;

    if (m_device.is_cgb())
    {
        // CGB: if LCDC bit 0 is 0, sprites are always displayed above
        // BG & window regardless of any priority flags
        m_priority_mask = (lcdc & gb_lcdc_bg_enable) ? 0xFF : 0x00;
    }
}



void age::gb_lcd_render::new_frame()
{
    m_screen_buffer.switch_buffers();
    m_rendered_scanlines = 0;
}

void age::gb_lcd_render::render(int until_scanline)
{
    AGE_ASSERT(until_scanline >= m_rendered_scanlines);
    AGE_ASSERT(m_rendered_scanlines <= gb_screen_height);

    int santized = std::min<int>(gb_screen_height, until_scanline);
    int to_render = santized - m_rendered_scanlines;

    // init window variables
    if (!m_rendered_scanlines)
    {
        m_wy_render = m_wy;
        m_wline = 0;
    }

    // new scanlines to render?
    if (to_render <= 0)
    {
        return;
    }
    int scanline = m_rendered_scanlines;
    m_rendered_scanlines += to_render;

    // render scanlines
    for ( ;scanline < m_rendered_scanlines; ++scanline)
    {
        render_scanline(scanline);
    }
}



void age::gb_lcd_render::render_scanline(int scanline)
{
    // We use the pixel alpha channel temporary for priority
    // information.
    // When the scanline is finshed, during copying to the
    // screen buffer the alpha channel is restored to 0xFF.
    //
    // Priority bits:
    //  0-2     BG color index
    //  3-6     -
    //  7       BG priority flag (OAM | CGB tile attribute)
    //
    //  => render sprite pixel only for priority <= 0x80

    // first visible pixel
    int px0 = 8 + (m_scx & 0b111);

    // BG & windows not visible
    if (!m_device.is_cgb() && !(m_lcdc & gb_lcdc_bg_enable))
    {
        pixel fill_color = m_palettes.get_palette(gb_palette_bgp)[0];
        fill_color.m_channels.m_a = 0x00; // sprites are prioritized
        std::fill(begin(m_scanline), end(m_scanline), fill_color);
    }

    // render BG & window
    else
    {
        int bg_y = m_scy + scanline;
        int tile_vram_ofs = m_bg_tile_map_offset + ((bg_y & 0b11111000) << 2);
        int tile_line = bg_y & 0b111;
        pixel *px = &m_scanline[8];

        bool window_visible = (m_lcdc & gb_lcdc_win_enable)
                && (m_wy_render <= scanline)
                && (m_wx < 167);

        int tiles = window_visible
                ? 1 + std::min<int>(20, std::max<int>(0, m_wx - 7) >> 3)
                : 21;

        for (int tx = m_scx >> 3, max = tx + tiles; tx < max; ++tx)
        {
            px = render_bg_tile(px, tile_line, tile_vram_ofs + (tx & 0b11111));
        }

        // render window
        if (window_visible)
        {
            tile_vram_ofs = m_win_tile_map_offset + ((m_wline & 0b11111000) << 2);
            tile_line = m_wline & 0b111;
            px = &m_scanline[px0 + m_wx - 7];

            for (int tx = 0, max = ((gb_screen_width + 7 - m_wx) >> 3) + 1; tx < max; ++tx)
            {
                px = render_bg_tile(px, tile_line, tile_vram_ofs + tx);
            }

            ++m_wline;
        }
    }

    // render sprites
    if (m_lcdc & gb_lcdc_obj_enable)
    {
        auto sprites = m_sprites.get_scanline_sprites(scanline);
        std::for_each(begin(sprites), end(sprites), [&](const auto &sprite)
        {
            //! \todo render sprite
        });
    }

    // copy scanline
    auto dst = &m_screen_buffer.get_back_buffer()[0] + scanline * gb_screen_width;
    auto src = &m_scanline[px0]; // % 8

    int32_t alpha = pixel{0, 0, 0, 255}.m_color;
    for (int i = 0; i < gb_screen_width; ++i)
    {
        // replace priority information with alpha value
        dst->m_color = src->m_color | alpha;
        ++src;
        ++dst;
    }
}



age::pixel* age::gb_lcd_render::render_bg_tile(pixel *dst,
                                               int tile_line,
                                               int tile_vram_ofs)
{
    AGE_ASSERT((tile_vram_ofs >= 0x1800) && (tile_vram_ofs < 0x2000));
    AGE_ASSERT((tile_line >= 0) && (tile_line < 8));

    int tile_nr = m_video_ram[tile_vram_ofs] ^ m_tile_xor; // bank 0
    int attributes = m_video_ram[tile_vram_ofs + 0x2000]; // bank 1

    // y-flip
    if (attributes & bg_tile_flip_y)
    {
        tile_line = 7 - tile_line;
    }

    // read tile data
    int tile_data_ofs = tile_nr << 4; // 16 bytes per tile
    tile_data_ofs += m_tile_data_offset;
    tile_data_ofs += (attributes & bg_tile_vram_bank) << 10;
    tile_data_ofs += tile_line << 1; // 2 bytes per line

    int tile_byte1 = m_video_ram[tile_data_ofs];
    int tile_byte2 = m_video_ram[tile_data_ofs + 1];

    // x-flip
    // (we invert the x-flip for easier rendering:
    // this way bit 0 is used for the leftmost pixel)
    if (!(attributes & bg_tile_flip_x))
    {
        tile_byte1 = m_xflip_cache[tile_byte1];
        tile_byte2 = m_xflip_cache[tile_byte2];
    }

    // bg priority
    const pixel *palette = m_palettes.get_palette(attributes & bg_tile_palette);
    uint8_t priority = attributes & bg_tile_priority;

    // render tile line
    // (least significant bit in byte1)
    tile_byte2 <<= 1;

    for (int i = 0; i < 8; ++i)
    {
        int color_idx = (tile_byte1 & 0b01) + (tile_byte2 & 0b10);
        pixel color = palette[color_idx];
        color.m_channels.m_a = color_idx + priority;
        *dst = color;

        ++dst;
        tile_byte1 >>= 1;
        tile_byte2 >>= 1;
    }
    return dst;
}



void age::gb_lcd_render::render_sprite_tile(pixel *dst,
                                            int tile_line,
                                            uint8_t tile_nr,
                                            uint8_t oam_attr)
{
    //! \todo render sprite
}
