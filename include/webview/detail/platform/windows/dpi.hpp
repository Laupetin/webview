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

#ifndef WEBVIEW_DETAIL_PLATFORM_WINDOWS_DPI_HPP
#define WEBVIEW_DETAIL_PLATFORM_WINDOWS_DPI_HPP

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

namespace webview::detail
{
  bool enable_dpi_awareness();

  bool enable_non_client_dpi_scaling_if_needed(HWND window);

  constexpr int get_default_window_dpi()
  {
    return 96; // USER_DEFAULT_SCREEN_DPI
  }

  int get_window_dpi(HWND window);

  constexpr int scale_value_for_dpi(const int value, const int from_dpi, const int to_dpi)
  {
    return (value * to_dpi) / from_dpi;
  }

  constexpr SIZE scale_size(const int width, const int height, const int from_dpi, const int to_dpi)
  {
    return {scale_value_for_dpi(width, from_dpi, to_dpi), scale_value_for_dpi(height, from_dpi, to_dpi)};
  }

  SIZE make_window_frame_size(HWND window, int width, int height, int dpi);
} // namespace webview::detail

#endif // defined(WEBVIEW_PLATFORM_WINDOWS)
#endif // WEBVIEW_PLATFORM_WINDOWS_DPI_HPP
