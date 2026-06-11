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

#ifndef WEBVIEW_PLATFORM_WINDOWS_DPI_HPP
#define WEBVIEW_PLATFORM_WINDOWS_DPI_HPP

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

#include "../../native_library.hpp"
#include "shcore.hpp"
#include "user32.hpp"
#include "version.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#ifdef _MSC_VER
#pragma comment(lib, "user32.lib")
#endif

namespace webview::detail
{
  inline bool is_per_monitor_v2_awareness_available()
  {
    // Windows 10, version 1703
    return compare_os_version(10, 0, 15063) >= 0;
  }

  inline bool enable_dpi_awareness()
  {
    const auto user32 = native_library(L"user32.dll");
    if (const auto fn = user32.get(user32_symbols::SetProcessDpiAwarenessContext))
    {
      const auto dpi_awareness = reinterpret_cast<user32_symbols::DPI_AWARENESS_CONTEXT>(
          is_per_monitor_v2_awareness_available() ? user32_symbols::dpi_awareness::per_monitor_v2_aware : user32_symbols::dpi_awareness::per_monitor_aware);

      if (fn(dpi_awareness))
        return true;

      return GetLastError() == ERROR_ACCESS_DENIED;
    }

    if (const auto shcore = native_library(L"shcore.dll"))
    {
      if (const auto fn = shcore.get(shcore_symbols::SetProcessDpiAwareness))
      {
        const auto result = fn(shcore_symbols::PROCESS_PER_MONITOR_DPI_AWARE);
        return result == S_OK || result == E_ACCESSDENIED;
      }
    }

    if (const auto fn = user32.get(user32_symbols::SetProcessDPIAware))
      return !!fn();

    return true;
  }

  inline bool enable_non_client_dpi_scaling_if_needed(const HWND window)
  {
    const auto user32 = native_library(L"user32.dll");
    const auto get_ctx_fn = user32.get(user32_symbols::GetWindowDpiAwarenessContext);
    if (!get_ctx_fn)
      return true;

    const auto awareness = get_ctx_fn(window);
    if (!awareness)
      return false;

    const auto ctx_equal_fn = user32.get(user32_symbols::AreDpiAwarenessContextsEqual);
    if (!ctx_equal_fn)
      return true;

    // EnableNonClientDpiScaling is only needed with per monitor v1 awareness.
    const auto per_monitor = reinterpret_cast<user32_symbols::DPI_AWARENESS_CONTEXT>(user32_symbols::dpi_awareness::per_monitor_aware);
    if (!ctx_equal_fn(awareness, per_monitor))
      return true;

    const auto enable_fn = user32.get(user32_symbols::EnableNonClientDpiScaling);
    if (!enable_fn)
      return true;

    return !!enable_fn(window);
  }

  constexpr int get_default_window_dpi()
  {
    return 96; // USER_DEFAULT_SCREEN_DPI
  }

  inline int get_window_dpi(const HWND window)
  {
    const auto user32 = native_library(L"user32.dll");
    if (const auto fn = user32.get(user32_symbols::GetDpiForWindow))
    {
      const auto dpi = static_cast<int>(fn(window));
      return dpi;
    }

    return get_default_window_dpi();
  }

  constexpr int scale_value_for_dpi(const int value, const int from_dpi, const int to_dpi)
  {
    return (value * to_dpi) / from_dpi;
  }

  constexpr SIZE scale_size(const int width, const int height, const int from_dpi, const int to_dpi)
  {
    return {scale_value_for_dpi(width, from_dpi, to_dpi), scale_value_for_dpi(height, from_dpi, to_dpi)};
  }

  inline SIZE make_window_frame_size(const HWND window, const int width, const int height, const int dpi)
  {
    const auto style = GetWindowLong(window, GWL_STYLE);
    RECT r{0, 0, width, height};
    const auto user32 = native_library(L"user32.dll");

    if (const auto fn = user32.get(user32_symbols::AdjustWindowRectExForDpi))
      fn(&r, style, FALSE, 0, static_cast<UINT>(dpi));
    else
      AdjustWindowRect(&r, style, 0);

    const auto frame_width = r.right - r.left;
    const auto frame_height = r.bottom - r.top;

    return {frame_width, frame_height};
  }

} // namespace webview::detail

#endif // defined(WEBVIEW_PLATFORM_WINDOWS)
#endif // WEBVIEW_PLATFORM_WINDOWS_DPI_HPP
