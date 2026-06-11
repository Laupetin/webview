#pragma once

#ifndef WEBVIEW_DETAIL_PLATFORM_WINDOWS_THEME_IMPL_HPP
#define WEBVIEW_DETAIL_PLATFORM_WINDOWS_THEME_IMPL_HPP

#include "../../macros.hpp"

#if defined(WEBVIEW_PLATFORM_WINDOWS)

#include "../../native_library.hpp"
#include "dwmapi.hpp"
#include "reg_key.hpp"
#include "theme.hpp"

namespace webview::detail
{
  WEBVIEW_IMPL bool is_dark_theme_enabled()
  {
    constexpr auto* sub_key = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize";
    const reg_key key(HKEY_CURRENT_USER, sub_key, 0, KEY_READ);
    if (!key.is_open())
    {
      // Default is light theme
      return false;
    }

    return key.query_uint(L"AppsUseLightTheme", 1) == 0;
  }

  WEBVIEW_IMPL void apply_window_theme(const HWND window)
  {
    const auto dark_theme_enabled = is_dark_theme_enabled();

    // Use "immersive dark mode" on systems that support it.
    // Changes the color of the window's title bar (light or dark).
    const BOOL use_dark_mode{dark_theme_enabled ? TRUE : FALSE};
    static native_library dwmapi{L"dwmapi.dll"};

    if (const auto fn = dwmapi.get(dwmapi_symbols::DwmSetWindowAttribute))
    {
      // Try the modern, documented attribute before the older, undocumented one.
      if (fn(window, dwmapi_symbols::DWMWA_USE_IMMERSIVE_DARK_MODE, &use_dark_mode, sizeof(use_dark_mode)) != S_OK)
        fn(window, dwmapi_symbols::DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_V10_0_19041, &use_dark_mode, sizeof(use_dark_mode));
    }
  }
} // namespace webview::detail

#endif // defined(WEBVIEW_PLATFORM_WINDOWS)
#endif // WEBVIEW_PLATFORM_WINDOWS_THEME_HPP
