#pragma once

#ifndef WEBWINDOWED_DETAIL_PLATFORM_WINDOWS_APP_WIN32_IMPL_HPP
#define WEBWINDOWED_DETAIL_PLATFORM_WINDOWS_APP_WIN32_IMPL_HPP

#include "../../macros.hpp"

#if defined(WEBWINDOWED_PLATFORM_WINDOWS) && defined(WEBWINDOWED_EDGE)

#include "app_win32.hpp"

#include <windows.h>

namespace webwindowed::detail
{
  WEBWINDOWED_IMPL void app_win32::terminate()
  {
    app_base::terminate();
    PostQuitMessage(0);
  }

  WEBWINDOWED_IMPL noresult app_win32::run_loop()
  {
    MSG msg;
    while (!m_stop_run_loop && GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }

    return {};
  }
} // namespace webwindowed::detail

#endif
#endif
