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

#ifndef WEBWINDOWED_DETAIL_ERRORS_HPP
#define WEBWINDOWED_DETAIL_ERRORS_HPP

#include <expected>
#include <string>

namespace webwindowed
{
  /**
   * @brief Error codes returned to callers of the API.
   *
   * The following codes are commonly used in the library:
   * - @c WEBWINDOWED_ERROR_OK
   * - @c WEBWINDOWED_ERROR_UNSPECIFIED
   * - @c WEBWINDOWED_ERROR_INVALID_ARGUMENT
   * - @c WEBWINDOWED_ERROR_INVALID_STATE
   *
   * With the exception of @c WEBWINDOWED_ERROR_OK which is normally expected,
   * the other common codes do not normally need to be handled specifically.
   * Refer to specific functions regarding handling of other codes.
   */
  enum class webwindowed_error
  {
    /// Missing dependency.
    MISSING_DEPENDENCY = -5,
    /// Operation canceled.
    CANCELED = -4,
    /// Invalid state detected.
    INVALID_STATE = -3,
    /// One or more invalid arguments have been specified e.g. in a function call.
    INVALID_ARGUMENT = -2,
    /// An unspecified error occurred. A more specific error code may be needed.
    UNSPECIFIED = -1,
    /// OK/Success. Functions that return error codes will typically return this
    /// to signify successful operations.
    OK = 0,
    /// Signifies that something already exists.
    DUPLICATE = 1,
    /// Signifies that something does not exist.
    NOT_FOUND = 2
  };

  class error_info
  {
public:
    error_info();
    explicit error_info(webwindowed_error code, std::string message = {}) noexcept;

    webwindowed_error code() const;
    const std::string& message() const;

private:
    webwindowed_error m_code;
    std::string m_message;
  };

  template<typename T> using result = std::expected<T, error_info>;
  using noresult = std::expected<void, error_info>;
} // namespace webwindowed

#endif // WEBWINDOWED_ERRORS_HPP
