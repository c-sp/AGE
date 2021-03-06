//
// Copyright 2020 Christoph Sprenger
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

#ifndef AGE_PIXEL_HPP
#define AGE_PIXEL_HPP

//!
//! \file
//!

#include <vector>

#include <age_debug.hpp>
#include <age_types.hpp>



namespace age
{

    struct pixel
    {
        pixel()
            : pixel(0, 0, 0)
        {}

        explicit pixel(unsigned rgb)
            : pixel((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF)
        {}

        pixel(int r, int g, int b)
            : pixel(r, g, b, 0xFF)
        {}

        pixel(int r, int g, int b, int a)
        {
            AGE_ASSERT(r >= 0);
            AGE_ASSERT(g >= 0);
            AGE_ASSERT(b >= 0);
            AGE_ASSERT(a >= 0);

            AGE_ASSERT(r <= 255);
            AGE_ASSERT(g <= 255);
            AGE_ASSERT(b <= 255);
            AGE_ASSERT(a <= 255);

            m_rgba.m_r = r & 0xFF;
            m_rgba.m_g = g & 0xFF;
            m_rgba.m_b = b & 0xFF;
            m_rgba.m_a = a & 0xFF;
        }

        bool operator==(const pixel& other) const
        {
            return m_color == other.m_color;
        }

        bool operator!=(const pixel& other) const
        {
            return m_color != other.m_color;
        }

        // We use byte-level access to be independent of byte ordering
        // and 32 bit integer access to speed up operations like comparison.
        union
        {
            struct
            {
                uint8_t m_r;
                uint8_t m_g;
                uint8_t m_b;
                uint8_t m_a;
            } m_rgba;
            uint32_t m_color;
        };
    };

    static_assert(sizeof(pixel) == 4, "expected pixel size of 4 bytes (RGBA)");

    using pixel_vector = std::vector<pixel>;

} // namespace age



#endif // AGE_PIXEL_HPP
