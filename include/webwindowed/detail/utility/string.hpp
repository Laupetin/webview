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

#ifndef WEBWINDOWED_DETAIL_UTILITY_STRING_HPP
#define WEBWINDOWED_DETAIL_UTILITY_STRING_HPP

#include <string>

namespace webwindowed::detail
{
#if defined(_WIN32)
  // Converts a narrow (UTF-8-encoded) string into a wide (UTF-16-encoded) string.
  inline std::wstring widen_string(const std::string& input);

  // Converts a wide (UTF-16-encoded) string into a narrow (UTF-8-encoded) string.
  std::string narrow_string(const std::wstring& input);
#endif
} // namespace webwindowed::detail

#endif // WEBWINDOWED_DETAIL_UTILITY_STRING_HPP
