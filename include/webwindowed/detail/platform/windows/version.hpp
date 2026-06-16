/*
 * MIT License
 *
 * Copyright (c) 2017 Serge Zaitsev
 * Copyright (c) 2022 Steffen André Langnes
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#ifndef WEBWINDOWED_DETAIL_PLATFORM_WINDOWS_VERSION_HPP
#define WEBWINDOWED_DETAIL_PLATFORM_WINDOWS_VERSION_HPP

#include "../../macros.hpp"

#if defined(WEBWINDOWED_PLATFORM_WINDOWS)

#include <array>
#include <string>

namespace webwindowed::detail
{
  // Parses a version string with 1-4 integral components, e.g. "1.2.3.4".
  // Missing or invalid components default to 0, and excess components are ignored.
  template<typename T> std::array<unsigned int, 4> parse_version(const std::basic_string<T>& version) noexcept
  {
    using iterator = typename std::basic_string<T>::const_iterator;
    auto parse_component = [](iterator sb, iterator se) -> unsigned int
    {
      try
      {
        auto n = std::stol(std::basic_string<T>(sb, se));
        return n < 0 ? 0 : n;
      }
      catch (std::exception&)
      {
        return 0;
      }
    };

    auto end = version.end();
    auto sb = version.begin(); // subrange begin
    auto se = sb;              // subrange end
    unsigned int ci = 0;       // component index
    std::array<unsigned int, 4> components{};

    while (sb != end && se != end && ci < components.size())
    {
      if (*se == static_cast<T>('.'))
      {
        components[ci++] = parse_component(sb, se);
        sb = ++se;

        continue;
      }

      ++se;
    }

    if (sb < se && ci < components.size())
      components[ci] = parse_component(sb, se);

    return components;
  }

  template<typename T, std::size_t Length> std::array<unsigned int, 4> parse_version(const T (&version)[Length]) noexcept
  {
    return parse_version(std::basic_string<T>(version, Length));
  }

  std::wstring get_file_version_string(const std::wstring& file_path) noexcept;

  // Compare the specified version against the OS version.
  // Returns less than 0 if the OS version is less.
  // Returns 0 if the versions are equal.
  // Returns greater than 0 if the specified version is greater.
  int compare_os_version(unsigned int major, unsigned int minor, unsigned int build);
} // namespace webwindowed::detail

#endif // defined(WEBWINDOWED_PLATFORM_WINDOWS)
#endif // WEBWINDOWED_PLATFORM_WINDOWS_VERSION_HPP
