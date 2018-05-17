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

#ifndef AGE_TYPES_HPP
#define AGE_TYPES_HPP

//!
//! \file
//!

#include <array>
#include <cstdint> // std::uint8_t etc.
#include <limits> // std::numeric_limits
#include <string>
#include <type_traits> // std::underlying_type
#include <vector>



namespace age {

// typedefs

typedef std::uint8_t  uint8;
typedef std::uint16_t uint16;
typedef std::uint32_t uint32;
typedef std::uint64_t uint64;

typedef std::int8_t  int8;
typedef std::int16_t int16;
typedef std::int32_t int32;
typedef std::int64_t int64;

//!
//! Defines an unsigned integer type that matches the architecture's default CPU register
//! size (32 bits or 64 bits) but is guaranteed to be at least 32 bits wide.
//!
typedef size_t uint;

template<uint _size>
using uint8_array = std::array<uint8, _size>;

typedef std::vector<uint8> uint8_vector;
typedef std::vector<std::string> string_vector;



// constants and constant expressions

const uint8_vector empty_uint8_vector = uint8_vector();

constexpr const char *project_name = "AGE";
constexpr const char *project_version = "1.0";

constexpr uint uint_max = std::numeric_limits<uint>::max();

//!
//! Convert the specified enum value to the corresponding value of the underlying type.
//!
template<typename E>
constexpr auto to_integral(E e) -> typename std::underlying_type<E>::type
{
    return static_cast<typename std::underlying_type<E>::type>(e);
}

} // namespace age



#endif // AGE_TYPES_HPP