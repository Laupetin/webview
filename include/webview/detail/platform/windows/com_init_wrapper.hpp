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

#ifndef WEBVIEW_DETAIL_PLATFORM_WINDOWS_COM_INIT_WRAPPER_HPP
#define WEBVIEW_DETAIL_PLATFORM_WINDOWS_COM_INIT_WRAPPER_HPP

#include "../../macros.hpp"

#if defined(WEBVIEW_PLATFORM_WINDOWS)

//
// ====================================================================
//
// This implementation uses Win32 API to create a native window. It
// uses Edge/Chromium webview2 backend as a browser engine.
//
// ====================================================================
//

#include "../../errors.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

namespace webview::detail
{
  /**
   * A wrapper around COM library initialization. Calls CoInitializeEx in the
   * constructor and CoUninitialize in the destructor.
   *
   * @exception exception Thrown if CoInitializeEx has already been called with a
   * different concurrency model.
   */
  class com_init_wrapper
  {
public:
    com_init_wrapper();

    static result<com_init_wrapper> create(DWORD dwCoInit);

    ~com_init_wrapper();

    com_init_wrapper(const com_init_wrapper& other) = delete;
    com_init_wrapper& operator=(const com_init_wrapper& other) = delete;

    com_init_wrapper(com_init_wrapper&& other) noexcept;

    com_init_wrapper& operator=(com_init_wrapper&& other) noexcept;

private:
    explicit com_init_wrapper(bool initialized);

    bool m_initialized;
  };
} // namespace webview::detail

#endif // defined(WEBVIEW_PLATFORM_WINDOWS)
#endif // WEBVIEW_PLATFORM_WINDOWS_COM_INIT_WRAPPER_HPP
