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

#ifndef WEBWINDOWED_DETAIL_PLATFORM_WINDOWS_REG_KEY_HPP
#define WEBWINDOWED_DETAIL_PLATFORM_WINDOWS_REG_KEY_HPP

#include "../../macros.hpp"

#if defined(WEBWINDOWED_PLATFORM_WINDOWS)

#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

namespace webwindowed::detail
{
  class reg_key
  {
public:
    explicit reg_key(HKEY root_key, const wchar_t* sub_key, DWORD options, REGSAM sam_desired);
    explicit reg_key(HKEY root_key, const std::wstring& sub_key, DWORD options, REGSAM sam_desired);
    virtual ~reg_key();

    reg_key(const reg_key& other) = delete;
    reg_key& operator=(const reg_key& other) = delete;
    reg_key(reg_key&& other) = delete;
    reg_key& operator=(reg_key&& other) = delete;

    bool is_open() const;
    bool get_handle() const;

    template<typename Container> void query_bytes(const wchar_t* name, Container& result) const
    {
      DWORD buf_length = 0;
      // Get the size of the data in bytes.
      auto status = RegQueryValueExW(m_handle, name, nullptr, nullptr, nullptr, &buf_length);
      if (status != ERROR_SUCCESS && status != ERROR_MORE_DATA)
      {
        result.resize(0);
        return;
      }

      // Read the data.
      result.resize(buf_length / sizeof(typename Container::value_type));
      auto* buf = reinterpret_cast<LPBYTE>(&result[0]);
      status = RegQueryValueExW(m_handle, name, nullptr, nullptr, buf, &buf_length);

      if (status != ERROR_SUCCESS)
        result.resize(0);
    }

    std::wstring query_string(const wchar_t* name) const;
    unsigned int query_uint(const wchar_t* name, const unsigned int default_value) const;

private:
    HKEY m_handle = nullptr;
  };
} // namespace webwindowed::detail

#endif // defined(WEBWINDOWED_PLATFORM_WINDOWS)
#endif // WEBWINDOWED_PLATFORM_WINDOWS_REG_KEY_HPP
