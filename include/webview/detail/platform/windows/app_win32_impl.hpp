#pragma once

#ifndef WEBVIEW_DETAIL_PLATFORM_WINDOWS_APP_WIN32_IMPL_HPP
#define WEBVIEW_DETAIL_PLATFORM_WINDOWS_APP_WIN32_IMPL_HPP

#include "../../macros.hpp"

#if defined(WEBVIEW_PLATFORM_WINDOWS) && defined(WEBVIEW_EDGE)

#include "app_win32.hpp"

#include <windows.h>

namespace webview::detail
{
  WEBVIEW_IMPL void app_win32::terminate()
  {
    app_base::terminate();
    PostQuitMessage(0);
  }

  WEBVIEW_IMPL noresult app_win32::run_loop()
  {
    MSG msg;
    while (!m_stop_run_loop && GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }

    return {};
  }
} // namespace webview::detail

#endif
#endif
